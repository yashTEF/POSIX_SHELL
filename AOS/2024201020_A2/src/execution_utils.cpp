#include "execution_utils.h"
#include "builtin.h"
#include <signal.h>


int shell_pg_id = -1;
int current_fg_pid = -1;

static std::string oldpwd = "";

std::vector<pid_t> background_processes;

void registerSigInt(int sign) {
	if (current_fg_pid != -1) {
		kill(current_fg_pid, SIGINT);
	}
}

void registerSigStp(int sign) {
	if (current_fg_pid != -1) {
		kill(current_fg_pid, SIGTSTP);
		background_processes.push_back(current_fg_pid);
		std::cout << "background_processes id: " << current_fg_pid << " stopped" << '\n';
		current_fg_pid = -1;
	}
}

void executeEcho(Command cmd) {
	for (auto str : cmd.args) {
		std::cout << str << " ";
	}
	std::cout << '\n';
}

void executePwd(Command cmd) {
	if (!cmd.args.empty()) {
		std::cerr << "too many arguments\n";
		return;
	}
	std::string path = getCurrentDirectory();
	if (path.find("~") == 0) {
		std::string homeDir = getHomeDirectory();
		path.replace(0, 1, homeDir);
	}
	std::cout << path << '\n';
}

void executeCd(Command command) {
	std::vector<std::string> args = command.args;
	std::string path;
	if (args.size() == 0) {
		path = getHomeDirectory();
	} else if (args.size() == 1) {
		std::string arg = args[0];
		if (arg == ".") {
			path = getCurrentDirectory();
		} else if (arg == "..") {
			path = getCurrentDirectory() + "/..";
		} else if (arg == "-") {
			if (oldpwd.empty()) {
				std::cerr << "cd: OLDPWD not set" << std::endl;
				return;
			}
			path = oldpwd;
		} else if (arg == "~") {
			path = getHomeDirectory();
		} else {
			path = arg;
		}
	} else {
		std::cerr << "cd: too many arguments" << std::endl;
		return;
	}
	if (path.find("~") == 0) {
		std::string homeDir = getHomeDirectory();
		path.replace(0, 1, homeDir);
	}
	oldpwd = getCurrentDirectory();
	if (chdir(path.c_str()) != 0) {
		std::cerr << "cd: " << path << ": No such file or directory" << std::endl;
	}
}

void executeListDir(Command command) {
	bool all = false;
	bool is_long = false;
	std::vector<std::string> paths;
	std::vector<std::string> args = command.args;
	for (auto arg : args) {
		if (arg == "-a") {
			all = true;
		} else if (arg == "-l") {
			is_long = true;
		} else if (arg == "-la" || arg == "-al") {
			is_long = all = true;
		} else {
			paths.push_back(arg);
		}
	}
	if (paths.empty()) {
		std::string path = getCurrentDirectory();
		if (path.find("~") == 0) {
			std::string homeDir = getHomeDirectory();
			path.replace(0, 1, homeDir);
		}
		lsCommand(path, all, is_long);
		return;
	}
	for (auto path : paths) {
		if (path == ".") {
			path = getCurrentDirectory();
		} else if (path == "..") {
			path = path + "/..";
		} else if (path == "~") {
			path = getHomeDirectory();
		}
		if (path.find("~") == 0) {
			std::string homeDir = getHomeDirectory();
			path.replace(0, 1, homeDir);
		}
		lsCommand(path, all, is_long);
	}
}

void executeFg(Command command) {
	pid_t pid;
	if (!command.args.empty()) {
		if (command.args.size() > 1) {
			std::cerr << "error in arguments\n";
			return;
		}
		pid = stoi(command.args[0]);
		if (find(background_processes.begin(), background_processes.end(), pid) == background_processes.end()) {
			std::cerr << "No process with " << pid << " in background\n";
			return;
		}
		background_processes.erase(std::remove(background_processes.begin(), background_processes.end(), pid), background_processes.end());
	} else {
		if (background_processes.empty()) {
			return;
		}
		pid = background_processes.back();
		background_processes.pop_back();
	}
	// int prgpr = getpgid(getpid());
	// logMsg("shell grp: " + std::to_string(prgpr));
	if (kill(pid, SIGCONT) == -1) {
		perror("cannot invoke signal\n");
		return;
	}
	current_fg_pid = pid;
	int status;
	tcsetpgrp(STDIN_FILENO, getpgid(pid));
	if (waitpid(pid, &status, WUNTRACED) == -1) {
		perror("error in wait");
		return;
	}
	current_fg_pid = -1;
	if (WIFSTOPPED(status)) {
		background_processes.push_back(pid);
	}
}

void executeCommand(Command cmd, int in_fd, int out_fd) {
	bool is_background = false;
	if (!cmd.args.empty() && cmd.args.back() == "&") {
		cmd.args.pop_back();
		is_background = true;
	}
	std::string command_str = cmd.command_name;
	for (std::string arg : cmd.args) {
		command_str += " " + arg;
	}
	if (cmd.command_name == "cd") {
		executeCd(cmd);
		return;
	} 
	if (cmd.command_name == "pwd") {
		executePwd(cmd);
		return;
	}
	if (cmd.command_name == "fg") {
		executeFg(cmd);
		return;
	}
	if (cmd.command_name == "pinfo") {
		if (cmd.args.empty()) {
			std::cerr << "error: invalid arguments\n";
			return;
		}
		std::cout << getProcInfo(stoi(cmd.args[0])) << '\n';
		return;
	}
	if (cmd.command_name == "search") {
		std::string target = cmd.args[0];
		std::string f_path = getCurrentDirectory();
		if (f_path.find("~") == 0) {
			std::string homeDir = getHomeDirectory();
			f_path.replace(0, 1, homeDir);
		}
		std::cout << (searchFileDFS(f_path, target) ? "YES" : "NO") << '\n';
		return;
	}
	if (cmd.command_name == "history") {
		int num = (cmd.args.empty()) ? 10 : std::stoi(cmd.args[0]);
		displayHistory(num);
		return;
	}

	int pid = fork();
	if (pid < 0) {
		perror("error in forking parent process");
		return;
	}

	if (pid == 0) {
		if (in_fd != -1) {
			dup2(in_fd, STDIN_FILENO);
			close(in_fd);
		}
		if (out_fd != -1) {
			dup2(out_fd, STDOUT_FILENO);
			close(out_fd);
		}
		if (!cmd.inp_f.empty()) {
			int inr_fd = open(cmd.inp_f.c_str(), O_RDONLY);
			if (inr_fd < 0) {
				perror("Failed to open input file");
				exit(EXIT_FAILURE);
			}
			dup2(inr_fd, STDIN_FILENO);
			close(inr_fd);
		}
		if (!cmd.out_f.empty()) {
			int flags = O_WRONLY | O_CREAT | (cmd.is_append ? O_APPEND : O_TRUNC);
			int outr_fd = open(cmd.out_f.c_str(), flags, 0644);
			if (outr_fd < 0) {
				perror("Failed to open output file");
				exit(EXIT_FAILURE);
			}
			dup2(outr_fd, STDOUT_FILENO);
			close(outr_fd);
		}
		if (cmd.command_name == "ls") {
			executeListDir(cmd);
			exit(EXIT_SUCCESS);
		}
		if (is_background) {
			setpgid(0, 0);
		}
		std::vector<char*> argv;
		argv.push_back(const_cast<char*>(cmd.command_name.c_str()));
		for (std::string& arg : cmd.args) {
			argv.push_back(const_cast<char*>(arg.c_str()));
		}
		argv.push_back(nullptr);
		execvp(cmd.command_name.c_str(), argv.data());
		perror("Exec failed");
		exit(EXIT_FAILURE);
	} else {
		if (is_background) {
			background_processes.push_back(pid);
			std::cout << "Started background job with PID " << pid << std::endl;
		} else {
			current_fg_pid = pid;
			// setpgid(pid, pid);
			// tcsetpgrp(STDIN_FILENO, pid);
			int status;
			waitpid(pid, &status, WUNTRACED);
			// tcsetpgrp(STDIN_FILENO, getpgid(getpid()));
			if (WIFSTOPPED(status)) {
				std::cerr << "Child stopped by signal " << WSTOPSIG(status) << std::endl;
				background_processes.push_back(pid);
			}
			current_fg_pid = -1;
		}
	}
	addHistory(command_str);
}


void executePipelineCommands(std::vector<Command> commands) {
	int clen = commands.size();
	if (clen < 2) {
		executeCommand(commands[0], -1, -1);
		return;
	}
	int pipes[2];
	int in_fd = -1;
	for (int i = 0; i < clen; ++i) {
		if (i < clen - 1) {
			if (pipe(pipes) == -1) {
				perror("error in pipe");
				exit(EXIT_FAILURE);
			}
		}
		executeCommand(commands[i], in_fd, i + 1 < clen ? pipes[1] : -1);
		if (in_fd != -1) {
			close(in_fd);
		}
		if (i < clen - 1) {
			close(pipes[1]);
		}
		in_fd = i + 1 < clen ? pipes[0] : -1;
	}	
}



