#include "command.hpp"
#include <iostream>
#include "cmd_handler.hpp"

/* help command implementation */
class HelpCommand : public Command {
public:
    HelpCommand() : Command("help") {}

    bool execute(ZipHandler&, const std::vector<std::string>&) override {
        std::cout << "Available Commands:" << std::endl;
        std::cout << CommandFactory::sprintHelp() << std::endl;
        return true;
    }

    std::vector<std::string> getAliases() const override {
        return {"h"};
    }

    std::string getDescription() const override {
        return "Display this help message";
    }
};
