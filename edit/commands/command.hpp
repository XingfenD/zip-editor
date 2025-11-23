#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <string>
#include <vector>
#include "zip_handler.hpp"

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

#endif /* COMMAND_HPP */
