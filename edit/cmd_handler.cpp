#include "cmd_handler.hpp"
#include <iostream>
#include "command_list.hpp"

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

/* command factory implementation */
void CommandFactory::initialize() {
    /* register all commands */
    registerCommand(std::make_shared<ExitCommand>());
    registerCommand(std::make_shared<HelpCommand>());
    registerCommand(std::make_shared<PrintCommand>());
    registerCommand(std::make_shared<ClearCommand>());
    registerCommand(std::make_shared<SaveCommand>());
    registerCommand(std::make_shared<ListCommand>());
    registerCommand(std::make_shared<AddCommand>());

    /* register aliases */
    for (const auto& command : commands) {
        for (const auto& alias : command.second->getAliases()) {
            registerAlias(alias, command.first);
        }
    }
}
