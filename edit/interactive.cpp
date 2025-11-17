#include "main_callee.hpp"
#include <iostream>
#include <csignal>
#include <vector>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <cctype>
#include <limits>
#include "debug_helper.hpp"
#include "cmd_handler.hpp"
#include "utils.hpp"

/* global flag to indicate whether the program is in edit mode */
volatile sig_atomic_t in_edit_mode = 0;

/* function to set terminal to raw mode for reading arrow keys */
termios setRawMode(termios &old_tio) {
    termios new_tio = old_tio;
    new_tio.c_lflag &= (~ICANON & ~ECHO);  /* disable canonical mode and echo */
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
    return new_tio;
}

/* function to restore terminal to original mode */
void restoreTerminal(termios &old_tio) {
    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
}

/* helper function to find commands matching the prefix */
std::vector<std::string> findMatchingCommands(const std::string& prefix) {
    std::vector<std::string> matches;
    std::vector<std::string> all_commands = CommandFactory::getAllCommands();

    for (const auto& cmd : all_commands) {
        if (cmd.compare(0, prefix.length(), prefix) == 0) {
            matches.push_back(cmd);
        }
    }

    /* sort matches alphabetically */
    std::sort(matches.begin(), matches.end());
    return matches;
}

/* function to read input with arrow key support and tab completion */
std::string readInputWithHistory(std::vector<std::string> &history, int &history_index, std::string &current_input) {
    termios old_tio;
    tcgetattr(STDIN_FILENO, &old_tio);
    setRawMode(old_tio);    /* set terminal to raw mode */
    std::string line;
    int cursor_pos = 0;

    try {
        while (true) {
            char c = getchar();

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
                    /* move cursor back one position */
                    std::cout << "\b" << std::flush;

                    /* save cursor after character */
                    std::string remaining = line.substr(cursor_pos);

                    /* remove character at cursor position */
                    line.erase(cursor_pos - 1, 1);
                    cursor_pos--;

                    /* redraw cursor after character (including space to clear last position) */
                    std::cout << remaining << " " << std::flush;

                    /* calculate number of characters to backtrack: remaining length + 1 space */
                    int back_count = remaining.length() + 1;

                    /* move cursor back to correct position */
                    std::cout << std::string(back_count, '\b') << std::flush;
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
            else if (c == 9) {  /* tab key for command completion */
                /* find the start of the current command (before cursor) */
                size_t cmd_start = line.rfind(' ', cursor_pos - 1);
                if (cmd_start == std::string::npos) {
                    cmd_start = 0;
                } else {
                    cmd_start += 1;
                }

                /* extract the command prefix */
                std::string prefix = line.substr(cmd_start, cursor_pos - cmd_start);

                /* find matching commands */
                std::vector<std::string> matches = findMatchingCommands(prefix);

                if (matches.size() == 1) {  /* single match, complete it */
                    std::string completion = matches[0].substr(prefix.length());
                    line.insert(cursor_pos, completion + " ");
                    cursor_pos += completion.length() + 1;

                    /* redraw the line from cursor position */
                    std::cout << line.substr(cursor_pos - completion.length() - 1)
                              << std::string(line.length() - cursor_pos, '\b') << std::flush;
                } else if (matches.size() > 1) {  /* multiple matches */
                    /* find the longest common prefix */
                    size_t max_len = 0;
                    size_t min_len = std::numeric_limits<size_t>::max();
                    for (const auto& match : matches) {
                        min_len = std::min(min_len, match.length());
                    }

                    while (max_len < min_len) {
                        bool same = true;
                        char first = matches[0][max_len];
                        for (const auto& match : matches) {
                            if (match[max_len] != first) {
                                same = false;
                                break;
                            }
                        }
                        if (!same) break;
                        max_len++;
                    }

                    /* if there's a common prefix beyond what we already have */
                    if (max_len > prefix.length()) {
                        std::string common = matches[0].substr(prefix.length(), max_len - prefix.length());
                        line.insert(cursor_pos, common);
                        cursor_pos += common.length();

                        /* redraw the common part */
                        std::cout << common << line.substr(cursor_pos)
                                  << std::string(line.length() - cursor_pos, '\b') << std::flush;
                    } else {
                        /* no common prefix, display all matches */
                        std::cout << "\n";
                        for (const auto& match : matches) {
                            std::cout << "  " << match << std::endl;
                        }
                        std::cout << "> " << line << std::flush;
                    }
                }
            } else if (isprint(static_cast<unsigned char>(c))) {  /* regular printable character */
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

/* This function is now handled by the HelpCommand class */
void displayHelp() {
    std::cout << CommandFactory::sprintHelp() << std::endl;
}

/* edit the zip file in interactive mode */
void edit(ZipHandler& zip_handler) {
    std::string command;
    bool running = true;
    /* command history storage */
    std::vector<std::string> history;
    int history_index = -1;  /* -1 means not navigating history */
    std::string current_input;  /* store current input when navigating history */

    /* set up signal handler for Ctrl+C */
    std::signal(SIGINT, signalHandler);
    in_edit_mode = 1;

    /* initialize command factory */
    CommandFactory::initialize();

    std::cout << "Welcome to ZIP File Interactive Editor" << std::endl;
    std::cout << "Type 'help' for available commands, 'exit' to quit" << std::endl;
    std::cout << "--------------------------------------------" << std::endl;

    while (running) {
        std::cout << "\n> " << std::flush;
        command = readInputWithHistory(history, history_index, current_input);

        /* skip empty commands */
        if (command.empty()) {
            continue;
        }

        /* convert command to lowercase for case-insensitive comparison */
        for (auto& c : command) {
            c = std::tolower(static_cast<unsigned char>(c));
        }

        DEBUG_LOG_FMT("Command: %s, len: %d\n", command.c_str(), command.length());

        /* process command with factory pattern */
        size_t first_space = command.find(' ');
        std::string cmd_name = command;
        std::string cmd_param;

        /* extract command name and parameter if space exists */
        if (first_space != std::string::npos) {
            cmd_name = command.substr(0, first_space);
            /* extract parameter and trim leading whitespace */
            cmd_param = command.substr(first_space + 1);
            size_t start_pos = cmd_param.find_first_not_of(" ");
            if (start_pos != std::string::npos) {
                cmd_param = cmd_param.substr(start_pos);
            }
        }

        /* try to get command by exact match first (including multi-word commands) */
        auto cmd = CommandFactory::getCommand(command);
        if (cmd == nullptr) {
            /* if not found, try with just the command name */
            cmd = CommandFactory::getCommand(cmd_name);
        }

        if (cmd != nullptr) {
            /* execute the command */
            /* for clear command, we need to reset history index */
            if (cmd->getName() == "clear" || cmd_name == "c") {
                history_index = -1;
                current_input.clear();
            }
            running = cmd->execute(zip_handler, splitString(cmd_param, " "));
        } else {
            std::cout << "Unknown command: " << command << std::endl;
            std::cout << "Type 'help' for available commands" << std::endl;
        }
    }
}