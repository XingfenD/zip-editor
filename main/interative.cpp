#include "main_callee.hpp"
#include <iostream>
#include <csignal>

// 全局标志，表示是否处于编辑模式
volatile sig_atomic_t in_edit_mode = 0;

// 信号处理函数
void signalHandler(int signal) {
    if (signal == SIGINT && in_edit_mode) {
        // 在编辑模式下捕获Ctrl+C，只打印提示而不终止程序
        std::cout << "\n^C - 已取消当前命令，请输入新命令" << std::endl;
        std::cout << "> " << std::flush;
    } else {
        // 在非编辑模式下，使用默认处理方式（终止程序）
        std::signal(signal, SIG_DFL);
        std::raise(signal);
    }
}

/* display help information for interactive mode */
void displayHelp() {
    std::cout << "Available Commands:" << std::endl;
    std::cout << "  help, h          - Display this help message" << std::endl;
    std::cout << "  print, p         - Print all information about the ZIP file" << std::endl;
    std::cout << "  print local, pl  - Print local file headers information" << std::endl;
    std::cout << "  print central, pc- Print central directory headers information" << std::endl;
    std::cout << "  print end, pe    - Print end of central directory record information" << std::endl;
    std::cout << "  exit, quit, q    - Exit the interactive editor" << std::endl;
}

/* edit the zip file in interactive mode */
void edit(ZipHandler& zip_handler) {
    std::string command;
    bool running = true;

    // 设置信号处理函数和编辑模式标志
    std::signal(SIGINT, signalHandler);
    in_edit_mode = 1;

    std::cout << "Welcome to ZIP File Interactive Editor" << std::endl;
    std::cout << "Type 'help' for available commands, 'exit' to quit" << std::endl;
    std::cout << "注意：按Ctrl+C只会取消当前命令，不会退出编辑模式" << std::endl;
    std::cout << "--------------------------------------------" << std::endl;

    // First ensure the ZIP file is parsed
    if (!zip_handler.parse()) {
        std::cerr << "Error: Failed to parse ZIP file" << std::endl;
        return;
    }

    while (running) {
        std::cout << "\n> " << std::flush;
        std::getline(std::cin, command);

        // Convert command to lowercase for case-insensitive comparison
        for (auto& c : command) {
            c = std::tolower(c);
        }

        if (command == "exit" || command == "quit" || command == "q") {
            running = false;
            std::cout << "Exiting editor..." << std::endl;
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
        } else {
            std::cout << "Unknown command: " << command << std::endl;
            std::cout << "Type 'help' for available commands" << std::endl;
        }
    }
}
