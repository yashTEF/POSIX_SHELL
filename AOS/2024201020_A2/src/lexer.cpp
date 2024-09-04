#include <lexer.h>

std::vector<std::string> get_space_sep(std::string input) {
    std::vector<std::string> tokens;
    std::string currentToken;
    bool inQuotes = false;
    char quoteChar = '\0';

    for (char ch : input) {
        if (ch == '"' || ch == '\'') {
            if (inQuotes && ch == quoteChar) {
                inQuotes = false;
            } else if (!inQuotes) {
                inQuotes = true;
                quoteChar = ch;
            }
        }
        if (ch == ';' && !inQuotes) {
            if (!currentToken.empty()) {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
            tokens.push_back(";");
        } else if (ch == ' ' && !inQuotes) {
            if (!currentToken.empty()) {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
        } else {
            currentToken += ch;
        }
    }

    if (!currentToken.empty()) {
        tokens.push_back(currentToken);
    }

    return tokens;
}

Command processCommand(std::vector<std::string> com_tok) {
    Command cur_com;
    int c_len = com_tok.size();
    for (int j = 0; j < c_len;) {
        if (com_tok[j] == "<") {
            ++j;
            cur_com.inp_f = com_tok[j];
        } else if (com_tok[j] == ">") {
            ++j;
            cur_com.out_f = com_tok[j];
        } else if (com_tok[j] == ">>") {
            ++j;
            cur_com.out_f = com_tok[j];
            cur_com.is_append = true;
        } else {
            if (cur_com.command_name.empty()) {
                cur_com.command_name = com_tok[j];
            } else {
                cur_com.args.push_back(com_tok[j]);
            }
        }
        ++j;
    }
    return cur_com;
}

std::vector<Command> pipeSeparateCommands(std::vector<std::string> tokens) {
    std::vector<Command> command_sep;
    std::vector<std::string> com_tok;
    int t_len = tokens.size();
    
    for (int i = 0; i < t_len; ++i) {
        if (tokens[i] == "|" || tokens[i] == ";") {
            if (!com_tok.empty()) {
                command_sep.push_back(processCommand(com_tok));
                com_tok.clear();
            }
            
            if (tokens[i] == ";") {
                Command separator;
                separator.command_name = tokens[i];
                separator.is_separator = true;
                command_sep.push_back(separator);
            } 
        } else {
            com_tok.push_back(tokens[i]);
        }
    }
    
    if (!com_tok.empty()) {
        command_sep.push_back(processCommand(com_tok));
        com_tok.clear();
    }
    
    return command_sep;
}