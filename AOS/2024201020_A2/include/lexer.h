#include "basic_utils.h"

struct Command {
	std::string command_name;
	std::vector<std::string> args;
	std::string inp_f;
	std::string out_f;

	bool is_append = false;
	bool is_separator = false;
};

Command processCommand(std::vector<std::string> com_tok);

std::vector<std::string> get_space_sep(std::string input);

std::vector<Command> pipeSeparateCommands(std::vector<std::string> tokens);