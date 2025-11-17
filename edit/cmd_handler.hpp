#ifndef CMD_HANDLER_HPP
#define CMD_HANDLER_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "zip_handler.hpp"

/* command interface for the command pattern */
class Command {
public:
    Command(const std::string& name) : name(name) {}
    virtual ~Command() = default;
    virtual bool execute(ZipHandler& zip_handler, const std::vector<std::string>& params) = 0;
    virtual std::string getName() const { return name; }
    virtual std::vector<std::string> getAliases() const { return {}; }
    virtual std::string getDescription() const { return ""; }
    virtual std::string buildHelp() const {
        std::string ret;
        /* command name left-aligned, fixed width 15 characters */
        ret += getName();
        if (getName().length() < 15) {
            ret.append(15 - getName().length(), ' ');
        }
        ret += "- ";
        ret += getDescription();
        /* add alias information if any */
        const auto& aliases = getAliases();
        if (!aliases.empty()) {
            ret += " (aliases: ";
            for (size_t i = 0; i < aliases.size(); ++i) {
                ret += aliases[i];
                if (i < aliases.size() - 1) {
                    ret += ", ";
                }
            }
            ret += ")";
        }
        return ret;
    }

private:
    std::string name;
};

/* command factory class to create and manage commands */
class CommandFactory {
private:
    static std::map<std::string, std::shared_ptr<Command>> commands;
    static std::map<std::string, std::string> command_aliases;

public:
    /* initialize all available commands */
    static void initialize();

    /**
     * get command by name
     * @param name command name
     * @return command pointer if found, nullptr otherwise
     */
    static std::shared_ptr<Command> getCommand(const std::string& name);

    /**
     * get help for all commands
     * @return help string
     */
    static std::string sprintHelp();

    /**
     * register a command
     * @param command command to register
     */
    static void registerCommand(std::shared_ptr<Command> command);

    /**
     * register a command alias
     * @param alias alias name
     * @param command_name original command name
     */
    static void registerAlias(const std::string& alias, const std::string& command_name);

    /**
     * get all command names
     * @return vector of command names
     */
    static std::vector<std::string> getAllCommands();
};

#endif /* CMD_HANDLER_HPP */
