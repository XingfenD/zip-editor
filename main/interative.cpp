#include "main_callee.hpp"
#include <iostream>
#include <csignal>
#include <vector>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <cctype>
#include "debug_helper.hpp"

/* global flag to indicate whether the program is in edit mode */
volatile sig_atomic_t in_edit_mode = 0;

/* Function to set terminal to raw mode for reading arrow keys */
termios setRawMode(termios &old_tio) {
    termios new_tio = old_tio;
    new_tio.c_lflag &= (~ICANON & ~ECHO);  /* disable canonical mode and echo */
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
    return new_tio;
}

/* Function to restore terminal to original mode */
void restoreTerminal(termios &old_tio) {
    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
}

/* Function to read input with arrow key support */
std::string readInputWithHistory(std::vector<std::string> &history, int &history_index, std::string &current_input) {
    termios old_tio;
    tcgetattr(STDIN_FILENO, &old_tio);
    setRawMode(old_tio);    /* set terminal to raw mode */

    std::string line;
    int cursor_pos = 0;

    try {
        while (true) {
            char c = getchar();
            DEBUG_LOG_FMT("readInputWithHistory: c = %d, cursor_pos = %d, line = %s", c, cursor_pos, line.c_str());

            if (c == '\n') {  /* enter key */
                std::cout << std::endl;
                /* add non-empty command to history */
                if (!line.empty() && (history.empty() || line != history.back())) {
                    history.push_back(line);
                }
                history_index = -1;    /* reset history index */
                current_input.clear();
                break;
            }
            else if (c == 127 || c == '\b') {  /* backspace */
                if (cursor_pos > 0) {
                    /* move cursor back, print space to clear character, then move back again */
                    std::cout << "\b \b" << std::flush;
                    line.erase(cursor_pos - 1, 1);
                    cursor_pos--;
                    /* if there are characters after the cursor position, redraw them */
                    if (cursor_pos < static_cast<int>(line.length())) {
                        std::cout << line.substr(cursor_pos) << " " << std::string(line.length() - cursor_pos, '\b') << std::flush;
                    }
                }
            }
            else if (c == 0x15) {  /* command+backspace on macos */
                if (!line.empty()) {
                    /* clear the entire line by moving back and overwriting with spaces */
                    std::cout << std::string(cursor_pos, '\b');  /* move to start of input */
                    std::cout << std::string(line.length(), ' ');  /* overwrite with spaces */
                    std::cout << std::string(line.length(), '\b');  /* move back to start */

                    /* clear the actual input string and reset cursor */
                    line.clear();
                    cursor_pos = 0;
                }
            }
            else if (c == 27) {  /* esc sequence */
                char next1 = getchar();
                if (next1 == '[') {
                    char next2 = getchar();
                    if (next2 == 'A') {  /* up arrow */
                        if (!history.empty()) {
                            /* save current input if not already navigating */
                            if (history_index == -1) {
                                current_input = line;
                                history_index = history.size();
                            }
                            if (history_index > 0) {
                                history_index--;
                                /* clear current line */
                                std::cout << std::string(cursor_pos, '\b') << std::string(line.length(), ' ') << std::string(line.length(), '\b');
                                /* show historical command */
                                line = history[history_index];
                                cursor_pos = line.length();
                                std::cout << line << std::flush;
                            }
                        }
                    }
                    else if (next2 == 'B') {  /* down arrow */
                        if (!history.empty() && history_index < static_cast<int>(history.size()) - 1) {
                            history_index++;
                            /* clear current line */
                            std::cout << std::string(cursor_pos, '\b') << std::string(line.length(), ' ') << std::string(line.length(), '\b');
                            /* show historical command */
                            line = history[history_index];
                            cursor_pos = line.length();
                            std::cout << line << std::flush;
                        } else if (history_index == static_cast<int>(history.size()) - 1) {
                            /* go back to current input */
                            history_index = -1;
                            /* clear current line */
                            std::cout << std::string(cursor_pos, '\b') << std::string(line.length(), ' ') << std::string(line.length(), '\b');
                            line = current_input;
                            cursor_pos = line.length();
                            std::cout << line << std::flush;
                        }
                    }
                    else if (next2 == 'C') {  /* right arrow */
                        if (cursor_pos < static_cast<int>(line.length())) {
                            std::cout << "\033[C" << std::flush;
                            cursor_pos++;
                        }
                    }
                    else if (next2 == 'D') {  /* left arrow */
                        if (cursor_pos > 0) {
                            std::cout << "\033[D" << std::flush;
                            cursor_pos--;
                        }
                    }
                }
            }
            else if (c == 3) {  /* ctrl+c */
                line.clear();
                cursor_pos = 0;
                history_index = -1;
                current_input.clear();
                break;
            }
            else if (isprint(static_cast<unsigned char>(c))) {  /* regular printable character */
                line.insert(cursor_pos, 1, c);
                cursor_pos++;
                /* display the added character and the rest of the line */
                std::cout << c << line.substr(cursor_pos) << std::string(line.length() - cursor_pos, '\b') << std::flush;
            }
            /* ignore non-printable characters except those we handle */
        }
    } catch (...) {
        restoreTerminal(old_tio);
        throw;
    }

    restoreTerminal(old_tio);
    return line;
}

/* signal handler function to handle Ctrl+C */
void signalHandler(int signal) {
    if (signal == SIGINT && in_edit_mode) {
        /* In edit mode, catch Ctrl+C and print a prompt instead of terminating */
        std::cout << "\n> " << std::flush;
    } else {
        /* In non-edit mode, use default behavior (terminate the program) */
        std::signal(signal, SIG_DFL);
        std::raise(signal);
    }
}

/* display help information for interactive mode */
void displayHelp() {
    std::cout << "Available Commands:" << std::endl;
    std::cout << "  help, h          - Display this help message" << std::endl;
    std::cout << "  clear, c         - Clear the terminal screen" << std::endl;
    std::cout << "  print, p         - Print all information about the ZIP file" << std::endl;
    std::cout << "  print local, pl  - Print local file headers information" << std::endl;
    std::cout << "  print central, pc- Print central directory headers information" << std::endl;
    std::cout << "  print end, pe    - Print end of central directory record information" << std::endl;
    std::cout << "  save <path>      - Save the ZIP file to the specified path" << std::endl;
    std::cout << "  exit, quit, q    - Exit the interactive editor" << std::endl;
}

/* edit the zip file in interactive mode */
void edit(ZipHandler& zip_handler) {
    std::string command;
    bool running = true;
    /* Command history storage */
    std::vector<std::string> history;
    int history_index = -1;  /* -1 means not navigating history */
    std::string current_input;  /* Store current input when navigating history */

    /* Set up signal handler for Ctrl+C */
    std::signal(SIGINT, signalHandler);
    in_edit_mode = 1;

    std::cout << "Welcome to ZIP File Interactive Editor" << std::endl;
    std::cout << "Type 'help' for available commands, 'exit' to quit" << std::endl;
    std::cout << "--------------------------------------------" << std::endl;

    /* First ensure the ZIP file is parsed */
    if (!zip_handler.parse()) {
        std::cerr << "Error: Failed to parse ZIP file" << std::endl;
        return;
    }

    while (running) {
        std::cout << "\n> " << std::flush;
        command = readInputWithHistory(history, history_index, current_input);

        /* Skip empty commands */
        if (command.empty()) {
            continue;
        }

        /* Convert command to lowercase for case-insensitive comparison */
        for (auto& c : command) {
            c = std::tolower(static_cast<unsigned char>(c));
        }

        if (command == "exit" || command == "quit" || command == "q") {
            running = false;
        } else if (command == "help" || command == "h") {
            displayHelp();
        } else if (command == "print" || command == "p") {
            zip_handler.print();
        } else if (command == "print local" || command == "pl") {
            zip_handler.printLocalFileHeaders();
        } else if (command == "print central" || command == "pc") {
            zip_handler.printCentralDirectoryHeaders();
        } else if (command == "print end" || command == "pe") {
            zip_handler.printEndOfCentralDirectoryRecord();
        } else if (command == "clear" || command == "c") {
            /* Use ANSI escape sequence to clear screen */
            std::cout << "\033[2J\033[1;1H" << std::flush;
            /* Display welcome message again after clearing */
            std::cout << "Welcome to ZIP File Interactive Editor" << std::endl;
            std::cout << "Type 'help' for available commands, 'exit' to quit" << std::endl;
            std::cout << "--------------------------------------------" << std::endl;
            /* Reset history index when clearing screen */
            history_index = -1;
            current_input.clear();
        } else if (command.substr(0, 5) == "save ") {
            /* Extract output path from command */
            std::string output_path = command.substr(5);

            /* Check if output path is provided */
            if (output_path.empty()) {
                std::cout << "Error: Output path is required for save command" << std::endl;
                std::cout << "Usage: save <path>" << std::endl;
            } else {
                /* Call save method with the provided output path */
                zip_handler.save(output_path);
            }
        } else {
            std::cout << "Unknown command: " << command << std::endl;
            std::cout << "Type 'help' for available commands" << std::endl;
        }
    }
}
