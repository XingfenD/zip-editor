#include "cmd_handler.hpp"
#include <iostream>
#include "debug_helper.hpp"

/* static member initialization*/
std::map<std::string, std::shared_ptr<Command>> CommandFactory::commands;
std::map<std::string, std::string> CommandFactory::command_aliases;

/* exit command implementation */
class ExitCommand : public Command {
public:
    bool execute(ZipHandler& zip_handler, const std::string& params) override {
        DEBUG_LOG("Exit command executed\n");
        return false; /* return false to indicate program should exit */
    }

    std::string getName() const override {
        return "exit";
    }

    std::string getHelp() const override {
        return "exit, quit, q          - Exit the interactive editor";
    }
};

/* help command implementation */
class HelpCommand : public Command {
public:
    bool execute(ZipHandler& zip_handler, const std::string& params) override {
        std::cout << "Available Commands:" << std::endl;
        std::cout << CommandFactory::getHelp() << std::endl;
        return true;
    }

    std::string getName() const override {
        return "help";
    }

    std::string getHelp() const override {
        return "help, h          - Display this help message";
    }
};

/* print command implementation */
class PrintCommand : public Command {
public:
    bool execute(ZipHandler& zip_handler, const std::string& params) override {
        zip_handler.print();
        return true;
    }

    std::string getName() const override {
        return "print";
    }

    std::string getHelp() const override {
        return "print, p         - Print all information about the ZIP file";
    }
};

/* print local file headers command implementation */
class PrintLFHCommand : public Command {
public:
    bool execute(ZipHandler& zip_handler, const std::string& params) override {
        zip_handler.printLocalFileHeaders();
        return true;
    }

    std::string getName() const override {
        return "print lfh";
    }

    std::string getHelp() const override {
        return "print lfh, pl  - Print local file headers information";
    }
};

/* print central directory headers command implementation */
class PrintCDHCommand : public Command {
public:
    bool execute(ZipHandler& zip_handler, const std::string& params) override {
        zip_handler.printCentralDirectoryHeaders();
        return true;
    }

    std::string getName() const override {
        return "print central";
    }

    std::string getHelp() const override {
        return "print central, pc- Print central directory headers information";
    }
};

/* print end command implementation */
class PrintEndCommand : public Command {
public:
    bool execute(ZipHandler& zip_handler, const std::string& params) override {
        zip_handler.printEndOfCentralDirectoryRecord();
        return true;
    }

    std::string getName() const override {
        return "print end";
    }

    std::string getHelp() const override {
        return "print end, pe    - Print end of central directory record information";
    }
};

/* clear command implementation */
class ClearCommand : public Command {
public:
    bool execute(ZipHandler& zip_handler, const std::string& params) override {
        /* use ANSI escape sequence to clear screen */
        std::cout << "\033[2J\033[1;1H" << std::flush;
        std::cout << "Welcome to ZIP File Interactive Editor" << std::endl;
        std::cout << "Type 'help' for available commands, 'exit' to quit" << std::endl;
        std::cout << "--------------------------------------------" << std::endl;
        return true;
    }

    std::string getName() const override {
        return "clear";
    }

    std::string getHelp() const override {
        return "clear, c         - Clear the terminal screen";
    }
};

/* save command implementation */
class SaveCommand : public Command {
public:
    bool execute(ZipHandler& zip_handler, const std::string& params) override {
        if (params.empty()) {
            std::cout << "Error: Output path is required for save command" << std::endl;
            std::cout << "Usage: save <path>" << std::endl;
        } else {
            zip_handler.save(params);
        }
        return true;
    }

    std::string getName() const override {
        return "save";
    }

    std::string getHelp() const override {
        return "save <path>      - Save the ZIP file to the specified path";
    }
};

/* specific command factory methods */
std::shared_ptr<Command> createExitCommand() {
    return std::make_shared<ExitCommand>();
}

std::shared_ptr<Command> createHelpCommand() {
    return std::make_shared<HelpCommand>();
}

std::shared_ptr<Command> createPrintCommand() {
    return std::make_shared<PrintCommand>();
}

std::shared_ptr<Command> createPrintLFHCommand() {
    return std::make_shared<PrintLFHCommand>();
}

std::shared_ptr<Command> createPrintCDHCommand() {
    return std::make_shared<PrintCDHCommand>();
}

std::shared_ptr<Command> createPrintEOCDRCommand() {
    return std::make_shared<PrintEndCommand>();
}

std::shared_ptr<Command> createClearCommand() {
    return std::make_shared<ClearCommand>();
}

std::shared_ptr<Command> createSaveCommand() {
    return std::make_shared<SaveCommand>();
}

/* command factory implementation */
void CommandFactory::initialize() {
    /* register all commands */
    registerCommand(createExitCommand());
    registerCommand(createHelpCommand());
    registerCommand(createPrintCommand());
    registerCommand(createPrintLFHCommand());
    registerCommand(createPrintCDHCommand());
    registerCommand(createPrintEOCDRCommand());
    registerCommand(createClearCommand());
    registerCommand(createSaveCommand());
    // registerCommand(createListCommand());

    /* register aliases */
    registerAlias("quit", "exit");
    registerAlias("q", "exit");
    registerAlias("h", "help");
    registerAlias("p", "print");
    registerAlias("pl", "print local");
    registerAlias("pc", "print central");
    registerAlias("pe", "print end");
    registerAlias("c", "clear");
}

std::shared_ptr<Command> CommandFactory::getCommand(const std::string& name) {
    /* check if the name is an alias */
    auto alias_it = command_aliases.find(name);
    std::string actual_name = name;
    if (alias_it != command_aliases.end()) {
        actual_name = alias_it->second;
    }

    /* find the command */
    auto it = commands.find(actual_name);
    if (it != commands.end()) {
        return it->second;
    }

    /* check for multi-word commands (like "print local") */
    /* first, try exact match for multi-word commands */
    it = commands.find(name);
    if (it != commands.end()) {
        return it->second;
    }

    return nullptr;
}

std::string CommandFactory::getHelp() {
    std::string help;
    for (const auto& [name, command] : commands) {
        if (!command->getHelp().empty()) {
            help += "  " + command->getHelp() + "\n";
        }
    }
    return help;
}

void CommandFactory::registerCommand(std::shared_ptr<Command> command) {
    commands[command->getName()] = command;
}

void CommandFactory::registerAlias(const std::string& alias, const std::string& command_name) {
    command_aliases[alias] = command_name;
}
