#include <lexer.h>

std::vector<std::string> get_space_sep(std::string input) {
   std::vector<std::string> tokens;
    std::string current_token;
    bool in_double_quotes = false;
    bool in_single_quotes = false;
    bool escape_next = false;

    for (size_t i = 0; i < input.length(); ++i) {
        char ch = input[i];

        if (escape_next) {
            current_token += ch;
            escape_next = false;
        } else if (ch == '\\') {
            escape_next = true;
        } else if (ch == '"' && !in_single_quotes) {
            in_double_quotes = !in_double_quotes;
        } else if (ch == '\'' && !in_double_quotes) {
            in_single_quotes = !in_single_quotes;
        } else if (ch == ' ' && !in_double_quotes && !in_single_quotes) {
            if (!current_token.empty()) {
                tokens.push_back(current_token);
                current_token.clear();
            }
        } else if (ch == ';' && !in_double_quotes && !in_single_quotes) {
            if (!current_token.empty()) {
                tokens.push_back(current_token);
                current_token.clear();
            }
            tokens.push_back(";");
        } else {
            current_token += ch;
        }
    }

    if (!current_token.empty()) {
        tokens.push_back(current_token);
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