#include "cmd_handler.hpp"
#include <iostream>
#include <ctime>
#include <unistd.h>
#include "debug_helper.hpp"
#include "ui_manager.hpp"
#include "input_field.hpp"
#include "button.hpp"
#include "signal_manager.hpp"

/* static member initialization*/
std::map<std::string, std::shared_ptr<Command>> CommandFactory::commands;
std::map<std::string, std::string> CommandFactory::command_aliases;

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

std::vector<std::string> CommandFactory::getAllCommands() {
    std::vector<std::string> command_list;
    /* add all command names */
    for (const auto& command : commands) {
        command_list.push_back(command.first);
    }
    /* add all aliases */
    for (const auto& alias : command_aliases) {
        command_list.push_back(alias.first);
    }
    return command_list;
}


/* exit command implementation */
class ExitCommand : public Command {
public:
    ExitCommand() : Command("exit") {}

    bool execute(ZipHandler& zip_handler, const std::vector<std::string>& params) override {
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

    bool execute(ZipHandler& zip_handler, const std::vector<std::string>& params) override {
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

    bool execute(ZipHandler& zip_handler, const std::vector<std::string>& params) override {
        if (params.empty() || params[0] == "") {
            zip_handler.print();
        } else if (params.size() >= 1) {
            if (params[0] == "lfh") {
                printLocalFileHeaders(zip_handler, params);
            } else if (params[0] == "cdh") {
                printCentralDirectoryHeaders(zip_handler, params);
            } else if (params[0] == "eocdr") {
                zip_handler.printEndOfCentralDirectoryRecord();
            } else {
                std::cout << "Error: Invalid parameter for print command" << std::endl;
                std::cout << "Usage: print [lfh|cdh|eocdr] [index]" << std::endl;
            }
        }
        return true;
    }

    void printLocalFileHeaders(ZipHandler& zip_handler, const std::vector<std::string>& params) const {
        if (params.size() >= 2) {
            try {
                uint16_t index = std::stoi(params[1]);
                zip_handler.printLocalFileHeaders(index);
            } catch (const std::invalid_argument& e) {
                std::cerr << "Error: Invalid index for local file header" << std::endl;
            }
        } else {
            zip_handler.printLocalFileHeaders();
        }
    }

    void printCentralDirectoryHeaders(ZipHandler& zip_handler, const std::vector<std::string>& params) const {
        if (params.size() >= 2) {
            try {
                uint16_t index = std::stoi(params[1]);
                zip_handler.printCentralDirectoryHeaders(index);
            } catch (const std::invalid_argument& e) {
                std::cerr << "Error: Invalid index for central directory header" << std::endl;
            }
        } else {
            zip_handler.printCentralDirectoryHeaders();
        }
    }

    std::vector<std::string> getAliases() const override {
        return {"p"};
    }

    std::string getDescription() const override {
        return "Print information about the ZIP file";
    }

    std::string buildHelp() const override {
        std::string ret = "print [lfh|cdh|eocdr] [index]";
        if (ret.length() < 15) {
            ret.append(15 - ret.length(), ' ');
        }
        ret += "- " + getDescription();
        return ret;
    }
};

/* clear command implementation */
class ClearCommand : public Command {
public:
    ClearCommand() : Command("clear") {}

    bool execute(ZipHandler& zip_handler, const std::vector<std::string>& params) override {
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

    bool execute(ZipHandler& zip_handler, const std::vector<std::string>& params) override {
        if (params.empty()) {
            std::cout << "Error: Output path is required for save command" << std::endl;
            std::cout << "Usage: save <path>" << std::endl;
        } else {
            zip_handler.save(params[0]);
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

class ListCommand : public Command {
public:
    ListCommand() : Command("list") {}

    bool execute(ZipHandler& zip_handler, const std::vector<std::string>& params) override {
        if (params.size() == 0 || params[0] == "") {
            zip_handler.listLocalFileHeaders();
            zip_handler.listCentralDirectoryHeaders();
        } else if (params.size() == 1){
            if (params[0] == "lfh") {
                zip_handler.listLocalFileHeaders();
            } else if (params[0] == "cdh") {
                zip_handler.listCentralDirectoryHeaders();
            } else {
                std::cout << "Error: Invalid parameter for list command" << std::endl;
                std::cout << "Usage: list [lfh|cdh]" << std::endl;
            }
        }
        return true;
    }

    std::vector<std::string> getAliases() const override {
        return {"l"};
    }

    std::string getDescription() const override {
        return "List local file headers information";
    }
};

/* test_tui command implementation */
class TestTUICommand : public Command {
public:
    TestTUICommand() : Command("test_tui") {}

    bool execute(ZipHandler& zip_handler, const std::vector<std::string>& params) override {
        DEBUG_LOG("Test TUI command executed\n");

        try {
            /* create and initialize UI manager */
            UIManager ui;
            if (!ui.initialize()) {
                std::cerr << "Failed to initialize UI\n";
                return false;
            }

            /* initialize signal manager if not already initialized */
            SignalManager::initialize();

            /* register SIGINT (Ctrl+C) handler with cleanup callback using SignalGuard for RAII */
            SignalManager::SignalGuard sigintGuard(SIGINT, [&ui](int signal) {
                DEBUG_LOG_FMT("Signal %d received, cleaning up TUI...\n", signal);
                /* ensure TUI is properly shut down */
                if (ui.isInitialized()) {
                    ui.shutdown();
                }
                DEBUG_LOG("TUI cleanup completed\n");
            });

            /* add header */
            ui.addHeader("ZIP Editor Configuration");

            /* add input fields */
            InputField* nameField = ui.addInputField("Name:", 5, 10, 30, InputType::STRING, "default_name");
            InputField* passwordField = ui.addInputField("Password:", 7, 10, 30, InputType::STRING);
            InputField* hexField = ui.addInputField("Hex Value:", 9, 10, 30, InputType::HEX, "1A2B3C");

            /* add buttons */
            ui.addButton("OK", 11, 10, ButtonType::CONFIRM);
            ui.addButton("Cancel", 11, 25, ButtonType::CANCEL);

            /* run the UI main loop and get result */
            UIResult result = ui.run();

            /* get results after UI exits */
            if (result == UIResult::CONFIRM) {
                std::cout << "Configuration saved:\n";
                std::cout << "  Name: = " << nameField->getValue() << std::endl;
                std::cout << "  Password: = " << passwordField->getValue() << std::endl;
                std::cout << "  Hex Value: = " << hexField->getValue() << std::endl;
            } else {
                std::cout << "Configuration cancelled\n";
            }

            /* ensure shutdown is called even if exception occurs */
            ui.shutdown();
        } catch (const std::exception& e) {
            std::cerr << "Error running TUI: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Unknown error running TUI\n";
        }

        return true;
    }

    std::vector<std::string> getAliases() const override {
        return {"ttui"};
    }

    std::string getDescription() const override {
        return "Test the Text User Interface components";
    }
};

#ifdef REMOTE_DEBUG_ON
class TestDebugCommand : public Command {
public:
    TestDebugCommand() : Command("test_debug") {}

    bool execute(ZipHandler& zip_handler, const std::vector<std::string>& params) override {
        for (int i = 0; i < 500; i++) {
            DEBUG_LOG_FMT("test%d", i);
        }
        return true;
    }

    std::string getDescription() const override {
        return "Test debug command";
    }
};

class ReconnectDebugCommand : public Command {
public:
    ReconnectDebugCommand() : Command("reconnect_debug") {}

    bool execute(ZipHandler& zip_handler, const std::vector<std::string>& params) override {
        RemoteDebugClient::getInstance().initialize();
        return true;
    }

    std::string getDescription() const override {
        return "Reconnect debug server";
    }
};

class TestCtrlC : public Command {
public:
    TestCtrlC() : Command("test_ctrl_c") {}

    bool execute(ZipHandler& zip_handler, const std::vector<std::string>& params) override {
        std::cout << "Test counter execution, will exit after 15 seconds" << std::endl;

        /* initialize counter */
        int counter = 0;
        const int max_loops = 5; /* 5 loops * 3 seconds = 15 seconds */
        const int sleep_time = 3; /* sleep for 3 seconds */

        /* loop with counter incrementing every 3 seconds */
        while (counter < max_loops) {
            sleep(sleep_time);
            counter++;
            std::cout << "Counter incremented to: " << counter << std::endl;
            DEBUG_LOG_FMT("TestCtrlC counter value: %d", counter);
        }

        std::cout << "15 seconds elapsed, exiting loop" << std::endl;
        return true;
    }

    std::string getDescription() const override {
        return "Test Ctrl+C signal handler";
    }
};
#endif /* REMOTE_DEBUG_ON */

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

std::shared_ptr<Command> createClearCommand() {
    return std::make_shared<ClearCommand>();
}

std::shared_ptr<Command> createSaveCommand() {
    return std::make_shared<SaveCommand>();
}

std::shared_ptr<Command> createListCommand() {
    return std::make_shared<ListCommand>();
}

std::shared_ptr<Command> createTestTUICommand() {
    return std::make_shared<TestTUICommand>();
}

#ifdef REMOTE_DEBUG_ON
std::shared_ptr<Command> createTestDebugCommand() {
    return std::make_shared<TestDebugCommand>();
}

std::shared_ptr<Command> createReconnectDebugCommand() {
    return std::make_shared<ReconnectDebugCommand>();
}

std::shared_ptr<Command> createTestCtrlCCommand() {
    return std::make_shared<TestCtrlC>();
}
#endif /* REMOTE_DEBUG_ON */

/* command factory implementation */
void CommandFactory::initialize() {
    /* register all commands */
    registerCommand(createExitCommand());
    registerCommand(createHelpCommand());
    registerCommand(createPrintCommand());
    registerCommand(createClearCommand());
    registerCommand(createSaveCommand());
    registerCommand(createListCommand());
    registerCommand(createTestTUICommand());
#ifdef REMOTE_DEBUG_ON
    registerCommand(createTestDebugCommand());
    registerCommand(createReconnectDebugCommand());
    registerCommand(createTestCtrlCCommand());
#endif /* REMOTE_DEBUG_ON */

    /* register aliases */
    for (const auto& command : commands) {
        for (const auto& alias : command.second->getAliases()) {
            registerAlias(alias, command.first);
        }
    }
}
