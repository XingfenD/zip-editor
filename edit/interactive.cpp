#include "main_callee.hpp"
#include <iostream>
#include <csignal>
#include <vector>
#include <string>
#include <termios.h>
#include <unistd.h>
#include "debug_helper.hpp"
#include "cmd_handler.hpp"
#include "utils.hpp"
#include "input_handler.hpp"

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

/* function to read input with arrow key support and tab completion using factory pattern */
std::string readInputWithHistory(std::vector<std::string> &history, int &history_index, std::string &current_input) {
    /* initialize input handler factory if not already initialized */
    static bool initialized = false;
    if (!initialized) {
        InputHandlerFactory::initialize();
        initialized = true;
    }

    termios old_tio;
    tcgetattr(STDIN_FILENO, &old_tio);
    setRawMode(old_tio);    /* set terminal to raw mode */
    std::string line;
    int cursor_pos = 0;

    try {
        while (true) {
            char c = getchar();

            /* get appropriate handler for this character */
            std::shared_ptr<InputHandler> handler = InputHandlerFactory::getHandler(c);

            if (handler) {
                /* create input context */
                InputContext context(c, line, cursor_pos, history, history_index, current_input);
                /* use the handler to process the input */
                bool continue_loop = handler->handle(context);
                if (!continue_loop) {
                    break;  /* exit loop if handler returns false */
                }
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

    /* signal handling is now managed by SignalManager class */
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