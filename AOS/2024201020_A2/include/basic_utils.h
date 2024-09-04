#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <iostream>
#include <readline/readline.h>
#include <limits.h>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <grp.h>
#include <ctime>
#include <pwd.h>
#include <libproc.h>

void logMsg(std::string msg);