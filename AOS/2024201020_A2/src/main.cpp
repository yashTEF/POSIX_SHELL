#include "execution_utils.h"
#include "builtin.h"


void get_user_name(std::string& prompt_name) {
	char* user = getlogin();
	char hostname[10005];
	if (user == NULL) {
		std::cerr << "error: cannot fetch login name\n";
		exit(EXIT_FAILURE);
	}
	if (gethostname(hostname, sizeof(hostname)) != 0) {
		std::cerr << "error: cannot fetch host name\n";
		exit(EXIT_FAILURE);
	}
	prompt_name += std::string(user);
	prompt_name += "@";
	prompt_name += std::string(hostname);
	// std::cout << prompt_name << '\n';
}

int main() {
	char cwd[PATH_MAX];

	// shell_pg_id = getpid();
	// setpgid(shell_pg_id, shell_pg_id);
	current_fg_pid = -1;

	signal(SIGINT, registerSigInt);
	signal(SIGTSTP, registerSigStp);

	// tcsetpgrp(STDIN_FILENO, shell_pg_id);
	if (getcwd(cwd, sizeof(cwd)) == NULL) {
		std::cerr << "error: in finding current directory\n";
		exit(EXIT_FAILURE);
	}

	if (setenv("HOME", cwd, 1) != 0) {
		std::cerr << "error: in setting HOME\n";
		exit(EXIT_FAILURE);
	}

	std::string username;
	loadHistory();
	get_user_name(username);
	while (true) {
		std::string path = getCurrentDirectory();
		std::string prompt_name = "<" + username + ":" + path + ">";
		char* input = readline(prompt_name.c_str());
		if (input == nullptr) {
			std::cout << "Exiting from terminal\n";
			break;
		}
		std::string inp_str = std::string(input);
		std::vector<std::string> tokens = get_space_sep(inp_str);
		std::vector<Command> commands = pipeSeparateCommands(tokens);
		// for (auto x : commands) {
		// 	std::cout << x.command_name << ' ' << x.inp_f << ' ' << x.out_f << '\n';
		// 	for (auto y : x.args) {
		// 		std::cout << y << ' ';
		// 	}
		// 	std::cout << '\n';
		// }
		std::vector<Command> pipeCommands;
		for (auto command : commands) {
			if (command.is_separator) {
				executePipelineCommands(pipeCommands);
				pipeCommands.clear();
			} else {
				pipeCommands.push_back(command);
			}
		}
		if (!pipeCommands.empty()) {
			executePipelineCommands(commands);
		}
		free(input);
	}	
	saveHistory();
	return 0;
}
