#include "command.hpp"
#include <iostream>

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
