#include "command.hpp"
#include <iostream>

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
