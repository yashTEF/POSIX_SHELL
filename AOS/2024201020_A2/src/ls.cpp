#include "builtin.h"

void listFileInfo(std::string path, std::string fName) {
    struct stat fileStat;
    std::string fullPath = path + "/" + fName;

    if (stat(fullPath.c_str(), &fileStat) == -1) {
        perror("stat");
        return;
    }
    std::cout << ((S_ISDIR(fileStat.st_mode)) ? "d" : "-");
    std::cout << ((fileStat.st_mode & S_IRUSR) ? "r" : "-");
    std::cout << ((fileStat.st_mode & S_IWUSR) ? "w" : "-");
    std::cout << ((fileStat.st_mode & S_IXUSR) ? "x" : "-");
    std::cout << ((fileStat.st_mode & S_IRGRP) ? "r" : "-");
    std::cout << ((fileStat.st_mode & S_IWGRP) ? "w" : "-");
    std::cout << ((fileStat.st_mode & S_IXGRP) ? "x" : "-");
    std::cout << ((fileStat.st_mode & S_IROTH) ? "r" : "-");
    std::cout << ((fileStat.st_mode & S_IWOTH) ? "w" : "-");
    std::cout << ((fileStat.st_mode & S_IXOTH) ? "x" : "-") << " ";
    std::cout << fileStat.st_nlink << " ";
    struct passwd *pw = getpwuid(fileStat.st_uid);
    struct group *gr = getgrgid(fileStat.st_gid);
    std::cout << (pw ? pw->pw_name : "UNKNOWN") << " ";
    std::cout << (gr ? gr->gr_name : "UNKNOWN") << " ";
    std::cout << fileStat.st_size << " ";
    char timeBuf[80];
    struct tm *tm_info = localtime(&fileStat.st_mtime);
    strftime(timeBuf, sizeof(timeBuf), "%b %d %Y %H:%M", tm_info);
    std::cout << timeBuf << " ";
    std::cout << fName << std::endl;
}

bool searchFileDFS(std::string path, std::string target) {
    DIR* dir = opendir(path.c_str());
    if (dir == nullptr) {
        perror("error in opening directory");
        return false;
    }
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (std::string(entry->d_name) == "." || std::string(entry->d_name) == "..") {
            continue;
        }
        std::string fullPath = path + "/" + entry->d_name;
        if (entry->d_name == target) {
            closedir(dir);
            return true;
        }
        struct stat fileStat;
        if (stat(fullPath.c_str(), &fileStat) == 0 && S_ISDIR(fileStat.st_mode)) {
            if (searchFileDFS(fullPath, target)) {
                closedir(dir);
                return true;
            }
        }
    }
    closedir(dir);
    return false;
}


void lsCommand(std::string path, bool is_all, bool longf) {
    DIR* dir = opendir(path.c_str());
    if (dir == nullptr) {
        perror("error in opening directory");
        return;
    }

    struct dirent* entry;
    std::vector<std::string> entries;
    // logMsg("Here\n");
    long total_blocks = 0;
    while (true) {
        entry = readdir(dir);
        if (entry == nullptr) {
            break;
        }
        if (is_all || (entry->d_name[0] != '.')) {
            entries.push_back(entry->d_name);
        }
        struct stat st;
        if (entry->d_name[0] != '.') {
            std::string fpath = path + "/" + entry->d_name;
            if (stat(fpath.c_str(), &st) == 0) {
                total_blocks += st.st_blocks;
            }
        }
    }
    closedir(dir);
    if (longf) {
        std::cout << "total: " << total_blocks << std::endl;
        for (std::string name : entries) {
            listFileInfo(path, name);
        }
    } else {
        for (std::string name : entries) {
            std::cout << name << " ";
        }
        std::cout << std::endl;
    }
}