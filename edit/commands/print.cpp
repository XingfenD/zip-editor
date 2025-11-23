#include "command.hpp"
#include <iostream>

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
