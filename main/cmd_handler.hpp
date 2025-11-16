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
    virtual ~Command() = default;
    virtual bool execute(ZipHandler& zip_handler, const std::string& params) = 0;
    virtual std::string getName() const { return name; }
    virtual std::string getHelp() const = 0;
    // virtual void registerAliases() = 0;
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
    static std::string getHelp();

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
};

/* specific command implementations */
extern std::shared_ptr<Command> createExitCommand();
extern std::shared_ptr<Command> createHelpCommand();
extern std::shared_ptr<Command> createPrintCommand();
extern std::shared_ptr<Command> createPrintLFHCommand();
extern std::shared_ptr<Command> createPrintCDHCommand();
extern std::shared_ptr<Command> createPrintEOCDRCommand();
extern std::shared_ptr<Command> createClearCommand();
extern std::shared_ptr<Command> createSaveCommand();
extern std::shared_ptr<Command> createListCommand();

#endif /* CMD_HANDLER_HPP */
