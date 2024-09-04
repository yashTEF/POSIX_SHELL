#include "basic_utils.h"

#define MAX_HISTORY 20

extern std::string HISTORY_FILE;

extern std::vector<std::string> command_history;

void loadHistory();
void saveHistory();
void addHistory(std::string command);
void displayHistory(int num);

std::string getHomeDirectory();
std::string getCurrentDirectory();

bool searchFileDFS(std::string path, std::string target);
void listFileInfo(std::string path, std::string fName);
void lsCommand(std::string path, bool is_all, bool longf);

std::string getProcInfo(pid_t pid);
std::string getMemoryUsage(pid_t pid);
std::string getExecPath(pid_t pid);
std::string getProcessStatus(pid_t pid);

