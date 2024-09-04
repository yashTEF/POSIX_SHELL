#include "builtin.h"

std::string getHomeDirectory() {
    char* home = getenv("HOME");
    if (home != NULL) {
        return std::string(home);
    } else {
        return "/";
    }
}

std::string getCurrentDirectory() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        std::cerr << "error: in finding current directory\n";
        exit(EXIT_FAILURE);
    }
    std::string currentDir(cwd);
    std::string homeDir = getHomeDirectory();
    if (currentDir.find(homeDir) == 0) {
        currentDir.replace(0, homeDir.size(), "~");
    }
    return currentDir;
}