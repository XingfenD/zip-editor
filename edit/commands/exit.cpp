#include "command.hpp"

/* exit command implementation */
class ExitCommand : public Command {
public:
    ExitCommand() : Command("exit") {}

    bool execute(ZipHandler&, const std::vector<std::string>&) override {
        return false; /* return false to indicate program should exit */
    }

    std::vector<std::string> getAliases() const override {
        return {"quit", "q"};
    }

    std::string getDescription() const override {
        return "Exit the interactive editor";
    }
};
