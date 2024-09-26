# ASSIGNMENT-2

## POSIX TERMINAL IMPLEMENTATION USING SYSTEM CALLS

### Project Structure

- **`src/`**: Contains implementation files for commands and parser for tokens.
- **`include/`**: Header files for `.cpp` files.
- **`Makefile`**: Build script to compile and run the project.
- **`history.txt`**: Global history storage file.

### How to Run

1. Compile and run the project using:
   ```bash
   make run


This will open an instance of the shell where commands can be executed.

Assumptions
The pinfo command works for macOS only using the procbsdinfo struct.
Tokenization only works for normal tokens `<>|"";