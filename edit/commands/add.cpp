#include "command.hpp"
#include <iostream>
#include "form_factory.hpp"
#include "defs.hpp"

class AddCommand : public Command {
public:
    AddCommand() : Command("add") {}

    bool execute(ZipHandler&, const std::vector<std::string>& params) override {
        if (params.size() == 0 || params[0] == "") {
            std::cout << "Error: Invalid parameter for add command" << std::endl;
            std::cout << "Usage: add <lfh|cdh>" << std::endl;
        } else if (params.size() == 1){
            if (params[0] == "lfh") {
                /* show form to get local file header information */
                FormResult form_result = FormFactory::getInstance().showForm("edit_lfh");
                if (form_result.result_type == UIResult::CONFIRM) {
                    if (form_result.values[LFH_LENGTH_UNMATCH_KEY] == "true") {
                        FormContext context = FormContext("Local File Header length does not match the expected value. Do you want to continue?\nYes will override the field with expected value. No will keep the original value.");
                        FormResult confirm_result = FormFactory::getInstance().showForm("confirmation");
                    }
                }
            } else if (params[0] == "cdh") {
                // zip_handler.addCentralDirectoryHeader();
                FormResult confirm_result = FormFactory::getInstance().showForm("confirmation");
            } else {
                std::cout << "Error: Invalid parameter for add command" << std::endl;
                std::cout << "Usage: add <lfh|cdh>" << std::endl;
            }
        }
        return true;
    }

    std::string getDescription() const override {
        return "Add a segment to the ZIP file";
    }
};
