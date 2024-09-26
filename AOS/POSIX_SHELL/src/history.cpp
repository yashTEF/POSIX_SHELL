#include "builtin.h"

std::string HISTORY_FILE = "history.txt";
std::vector<std::string> command_history;

void loadHistory() {
    std::ifstream infile(HISTORY_FILE);
    if (!infile.is_open()) {
        std::cerr << "Error: Could not open history file for reading\n";
        return;
    }
    std::string line;
    while (getline(infile, line)) {
        command_history.push_back(line);
    }
    infile.close();
}

void saveHistory() {
    std::ofstream outfile(HISTORY_FILE);
    if (!outfile.is_open()) {
        std::cerr << "Error: Could not open history file for writing\n";
        return;
    }
    for (std::string command : command_history) {
        outfile << command << std::endl;
        // cout << command << '\n';
    }
    outfile.flush();
    outfile.close();
}

void addHistory(std::string command) {
    if (command_history.size() >= MAX_HISTORY) {
        command_history.erase(command_history.begin());
    }
    command_history.push_back(command);
}

void displayHistory(int num) {
    int start = command_history.size() > num ? command_history.size() - num : 0;
    for (int i = start; i < command_history.size(); ++i) {
        std::cout << i + 1 << ": " << command_history[i] << std::endl;
    }
}