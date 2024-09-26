#include "execution_utils.h"
#include "builtin.h"


int history_index = -1;
const std::vector<std::string> common_commands = {
    "ls", "cd", "pwd", "cp", "mv", "rm", "mkdir", "rmdir",
    "touch", "chmod", "chown", "cat", "less", "more", "grep",
    "find", "locate", "man", "echo", "clear", "history", "ps",
    "top", "kill", "jobs", "fg", "bg", "exit", "su", "sudo"
};

std::vector<std::string> file_list;

void getCommandList() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        std::string current_dir = cwd;
        DIR* dir = opendir(current_dir.c_str());
        
        if (dir) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                if (entry->d_type == DT_REG || entry->d_type == DT_DIR) {
                    std::string name(entry->d_name);
                    if (entry->d_type == DT_REG) {
                        if (access((current_dir + "/" + name).c_str(), X_OK) == 0) {
                            file_list.push_back(name);
                        }
                    } else if (entry->d_type == DT_DIR) {
                        if (name != "." && name != "..") {
                            file_list.push_back(name + "/");
                        }
                    }
                }
            }
            closedir(dir);
        }
    } else {
        std::cerr << "Error getting current directory" << std::endl;
    }
}

std::string getUserInput(const std::string& prompt_text) {
	struct termios original_termios, modified_termios;
	tcgetattr(STDIN_FILENO, &original_termios);
	modified_termios = original_termios;
	modified_termios.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &modified_termios);

	std::string user_input;
	std::string last_input;
	std::vector<std::string> matches;
	char input_char;
	std::cout << prompt_text;
	fflush(stdout);
	while (read(STDIN_FILENO, &input_char, 1) > 0) {
        if (input_char == '\n') {
        	std::cout << std::endl;
        	if (!user_input.empty() && (command_history.empty() || command_history.back() != user_input)) {
        		command_history.push_back(user_input);
        	}
        	history_index = -1;
        	break;
        } else if (input_char == 127) {
        	if (!user_input.empty()) {
        		user_input.pop_back();
        		std::cout << "\b \b";
        		fflush(stdout);
        		matches.clear();
        	}
        } else if (input_char == 27) { 
        	char escape_seq[2];
        	if (read(STDIN_FILENO, escape_seq, 2) == 2) {
        		if (escape_seq[0] == '[') {
                    if (escape_seq[1] == 'A') {
                    	if (history_index == -1) {
                    		history_index = command_history.size() - 1;
                    	} else if (history_index > 0) {
                    		history_index--;
                    	}
                    	if (history_index >= 0) {
                    		user_input = command_history[history_index];
                    	}
                    	std::cout << "\33[2K\r" << prompt_text << user_input;
                    	fflush(stdout);
                    } else if (escape_seq[1] == 'B') {
                    	if (history_index >= 0 && history_index < command_history.size() - 1) {
                    		history_index++;
                    		user_input = command_history[history_index];
                    	} else if (history_index == command_history.size() - 1) {
                    		history_index = -1;
                    		user_input.clear();
                    	}
                    	std::cout << "\33[2K\r" << prompt_text << user_input;
                    	fflush(stdout);
                    }
                }
            }
        } else if (input_char == '\t') {  
        	if (matches.empty() || user_input != last_input) {
        		matches.clear();
        		for (auto cmd : common_commands) {
        			if (cmd.find(user_input) == 0) { 
        				if (matches.size() < 5) {  
        					matches.push_back(cmd);
        				}
        			}
        		}
        		for (auto file : file_list) {
                    if (file.find(user_input) == 0) {
                        if (matches.size() < 5) {
                            matches.push_back(file);
                        }
                    }
                }
                last_input = user_input;
        	}
        	if (matches.size() == 1) {
        		user_input = matches[0];
        		std::cout << "\33[2K\r" << prompt_text << user_input;
        		fflush(stdout);
        	} else if (matches.size() > 1) {
        		std::cout << "\n";
        		for (auto match : matches) {
        			std::cout << "  " << match << "\n";
        		}
        		std::cout << prompt_text << user_input;
        		fflush(stdout);
        	}
        }  else if (input_char == 4) {
        	std::cout << std::endl;
            break;
        } 
        else {
        	user_input += input_char;
        	std::cout << input_char;
        	fflush(stdout);
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);

    return user_input;
}

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
		std::string input = getUserInput(prompt_name);

		if (input.empty()) {
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
	}	
	saveHistory();
	return 0;
}
