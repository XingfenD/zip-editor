#include "command.hpp"
#include <iostream>

class ClearCommand : public Command {
public:
    ClearCommand() : Command("clear") {}

    bool execute(ZipHandler&, const std::vector<std::string>&) override {
        /* use ANSI escape sequence to clear screen */
        std::cout << "\033[2J\033[1;1H" << std::flush;
        std::cout << "Welcome to ZIP File Interactive Editor" << std::endl;
        std::cout << "Type 'help' for available commands, 'exit' to quit" << std::endl;
        std::cout << "--------------------------------------------" << std::endl;
        return true;
    }

    std::vector<std::string> getAliases() const override {
        return {"c"};
    }

    std::string getDescription() const override {
        return "Clear the terminal screen";
    }
};
