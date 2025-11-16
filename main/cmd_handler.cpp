#include "cmd_handler.hpp"
#include <iostream>
#include "debug_helper.hpp"

/* static member initialization*/
std::map<std::string, std::shared_ptr<Command>> CommandFactory::commands;
std::map<std::string, std::string> CommandFactory::command_aliases;

/* exit command implementation */
class ExitCommand : public Command {
public:
    ExitCommand() : Command("exit") {}

    bool execute(ZipHandler& zip_handler, const std::string& params) override {
        DEBUG_LOG("Exit command executed\n");
        return false; /* return false to indicate program should exit */
    }

    std::vector<std::string> getAliases() const override {
        return {"quit", "q"};
    }

    std::string getDescription() const override {
        return "Exit the interactive editor";
    }
};

/* help command implementation */
class HelpCommand : public Command {
public:
    HelpCommand() : Command("help") {}

    bool execute(ZipHandler& zip_handler, const std::string& params) override {
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

/* print command implementation */
class PrintCommand : public Command {
public:
    PrintCommand() : Command("print") {}

    bool execute(ZipHandler& zip_handler, const std::string& params) override {
        zip_handler.print();
        return true;
    }

    std::vector<std::string> getAliases() const override {
        return {"p"};
    }

    std::string getDescription() const override {
        return "Print all information about the ZIP file";
    }
};

/* print local file headers command implementation */
class PrintLFHCommand : public Command {
public:
    PrintLFHCommand() : Command("print lfh") {}

    bool execute(ZipHandler& zip_handler, const std::string& params) override {
        zip_handler.printLocalFileHeaders();
        return true;
    }

    std::string getDescription() const override {
        return "Print local file headers information";
    }
};

/* print central directory headers command implementation */
class PrintCDHCommand : public Command {
public:
    PrintCDHCommand() : Command("print cdh") {}

    bool execute(ZipHandler& zip_handler, const std::string& params) override {
        zip_handler.printCentralDirectoryHeaders();
        return true;
    }

    std::string getDescription() const override {
        return "Print central directory headers information";
    }
};

/* print end of central directory record command implementation */
class PrintEOCDRCommand : public Command {
public:
    PrintEOCDRCommand() : Command("print eocdr") {}

    bool execute(ZipHandler& zip_handler, const std::string& params) override {
        zip_handler.printEndOfCentralDirectoryRecord();
        return true;
    }

    std::string getDescription() const override {
        return "Print end of central directory record information";
    }
};

/* clear command implementation */
class ClearCommand : public Command {
public:
    ClearCommand() : Command("clear") {}

    bool execute(ZipHandler& zip_handler, const std::string& params) override {
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

/* save command implementation */
class SaveCommand : public Command {
public:
    SaveCommand() : Command("save") {}

    bool execute(ZipHandler& zip_handler, const std::string& params) override {
        if (params.empty()) {
            std::cout << "Error: Output path is required for save command" << std::endl;
            std::cout << "Usage: save <path>" << std::endl;
        } else {
            zip_handler.save(params);
        }
        return true;
    }

    std::string getDescription() const override {
        return "Save the ZIP file to the specified path";
    }

    std::string buildHelp() const override {
        std::string ret = "save <path>";
        if (ret.length() < 15) {
            ret.append(15 - ret.length(), ' ');
        }
        ret += "- " + getDescription();
        return ret;
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
    return std::make_shared<PrintEOCDRCommand>();
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
    for (const auto& command : commands) {
        for (const auto& alias : command.second->getAliases()) {
            registerAlias(alias, command.first);
        }
    }
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

std::string CommandFactory::sprintHelp() {
    std::string help;
    for (const auto& [name, command] : commands) {
        /* if command has custom help, use it */
        if (!command->buildHelp().empty()) {
            help += "  " + command->buildHelp() + "\n";
        } else {
            /* command name left-aligned, fixed width 15 characters */
            std::string cmd_help = "  ";
            cmd_help += name;
            if (name.length() < 15) {
                cmd_help.append(15 - name.length(), ' ');
            }
            cmd_help += "- ";
            cmd_help += command->getDescription();

            /* add alias information if any */
            const auto& aliases = command->getAliases();
            if (!aliases.empty()) {
                cmd_help += " (aliases: ";
                for (size_t i = 0; i < aliases.size(); ++i) {
                    cmd_help += aliases[i];
                    if (i < aliases.size() - 1) {
                        cmd_help += ", ";
                    }
                }
                cmd_help += ")";
            }

            help += cmd_help + "\n";
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
