#include "lexer.h"

extern int current_fg_pid;
extern int shell_pg_id;

extern std::vector<pid_t> background_processes;

void registerSigInt(int);

void registerSigStp(int);

void executeFg(Command command);

void executeCd(Command command);

void executeCommand(std::string& input);

void executePipelineCommands(std::vector<Command> commands);