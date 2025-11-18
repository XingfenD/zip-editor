#include "form_factory.hpp"
#include "../utils/signal_manager.hpp"
#include <iostream>
#include "debug_helper.hpp"

/* get singleton instance */
FormFactory& FormFactory::getInstance() {
    static FormFactory instance;
    return instance;
}

/* initialize all predefined forms */
void FormFactory::initializeForms() {
    if (initialized_) {
        return;
    }

    /* register common forms */

    #ifdef REMOTE_DEBUG_ON
    /* file operation form - similar to TestTUICommand */
    registerForm("file_operation",
        [](UIManager& ui) {
            /* add header */
            ui.addHeader("File Operation");

            /* add input fields */
            ui.addInputField("filename", "Filename:", 5, 10, 30, InputType::STRING, "document.txt");
            ui.addInputField("password", "Password:", 7, 10, 30, InputType::STRING);
            ui.addInputField("hex_flag", "Hex Flag:", 9, 10, 10, InputType::HEX, "FF00");

            /* add buttons */
            ui.addConfirmButton();
            ui.addCancelButton();
        }
    );

    #endif /* REMOTE_DEBUG_ON */

    /* simple confirmation form */
    registerForm("confirmation",
        [](UIManager& ui) {
            ui.addHeader("Confirmation");
            ui.drawText(5, 10, "Are you sure you want to proceed?");
            ui.addConfirmButton("Yes");
            ui.addCancelButton("No");
        },
        [](UIManager& ui, UIResult result) {
            FormResult form_result;
            form_result.result_type = result;
            return form_result;
        }
    );
// private:
//     uint32_t signature;
//     uint16_t version_needed;
//     uint16_t general_bit_flag;
//     uint16_t compression_method;
//     uint16_t last_mod_time;
//     uint16_t last_mod_date;
//     uint32_t crc32;
//     uint32_t compressed_size;
//     uint32_t uncompressed_size;
//     uint16_t filename_length;
//     uint16_t extra_field_length;
//     std::string filename;
//     std::unique_ptr<uint8_t[]> extra_field;

//     /* the file data is not belong to local file header, but defined in LocalFileHeader for convenience */
//     std::unique_ptr<uint8_t[]> file_data;
// };
    registerForm("edit_lfh",
        [](UIManager& ui) {
            /* add header */
            ui.addHeader("Edit Local File Header");

            /* add input fields */
            ui.addInputField("signature", "Signature:", 5, 10, 8, InputType::HEX, "04034B50");
            ui.addInputField("version_needed", "Version Needed:", 7, 10, 4, InputType::HEX, "000A");
            ui.addInputField("general_bit_flag", "General Bit Flag:", 9, 10, 4, InputType::HEX, "0000");
            ui.addInputField("compression_method", "Compression Method:", 11, 10, 4, InputType::HEX, "0000");
            ui.addInputField("last_mod_time", "Last Mod Time:", 13, 10, 4, InputType::HEX, "5AF2");
            ui.addInputField("last_mod_date", "Last Mod Date:", 15, 10, 4, InputType::HEX, "0000");
            ui.addInputField("crc32", "CRC32:", 17, 10, 8, InputType::HEX, "00000000");
            ui.addInputField("compressed_size", "Compressed Size:", 19, 10, 8, InputType::HEX, "00000004");
            ui.addInputField("uncompressed_size", "Uncompressed Size:", 21, 10, 8, InputType::HEX, "00000004");
            ui.addInputField("filename_length", "Filename Length:", 23, 10, 4, InputType::HEX, "0007");
            ui.addInputField("extra_field_length", "Extra Field Length:", 25, 10, 4, InputType::HEX, "0000");
            ui.addInputField("filename", "Filename:", 27, 10, 30, InputType::STRING, "newfile");
            ui.addInputField("extra_field", "Extra Field:", 29, 10, 30, InputType::HEX);
            ui.addInputField("file_data", "File Data:", 31, 10, 30, InputType::HEX, "00000000");

            /* add buttons */
            ui.addConfirmButton();
            ui.addCancelButton();
        });




    initialized_ = true;
}

/* show a predefined form and return its result */
/* helper structure to store input field creation information */
struct InputFieldInfo {
    std::string name;
    std::string label;
    int row;
    int col;
    int width;
    InputType type;
    std::string default_value;
};

FormResult FormFactory::showForm(const std::string& form_name) {
    /* call the overloaded method with empty custom defaults */
    return showForm(form_name, {});
}

FormResult FormFactory::showForm(const std::string& form_name, const std::map<std::string, std::string>& custom_defaults) {
    /* ensure forms are initialized */
    if (!initialized_) {
        initializeForms();
    }

    /* check if form exists */
    auto it = form_templates_.find(form_name);
    if (it == form_templates_.end()) {
        DEBUG_LOG_FMT("Error: Form template '%s' not found!", form_name.c_str());
        FormResult error_result;
        error_result.result_type = UIResult::NONE;
        return error_result;
    }

    /* get form template */
    const auto& form_template = it->second;

    FormResult result;

    try {
        /* create and initialize UI manager */
        UIManager ui;
        if (!ui.initialize()) {
            std::cerr << "Failed to initialize UI" << std::endl;
            result.result_type = UIResult::NONE;
            return result;
        }

        /* initialize signal manager if not already initialized */
        SignalManager::initialize();

        /* register SIGINT handler */
        SignalManager::SignalGuard sigintGuard(SIGINT, [&ui](int signal) {
            if (ui.isInitialized()) {
                ui.shutdown();
            }
        });

        /* build the form */
        form_template.builder(ui);

        /* apply custom default values if provided */
        if (!custom_defaults.empty()) {
            /* get all input fields and apply custom defaults */
            for (const auto& field : ui.getInputFields()) {
                /* check if there's a custom default for this field's label */
                auto it = custom_defaults.find(field->getLabel());
                if (it != custom_defaults.end()) {
                    /* set the custom default value */
                    field->setValue(it->second);
                }
            }
        }

        /* run UI and get result */
        UIResult ui_result = ui.run();

        /* extract form data */
        result = form_template.extractor(ui, ui_result);

        /* ensure UI is shut down */
        ui.shutdown();

    } catch (const std::exception& e) {
        std::cerr << "Exception in form: " << e.what() << std::endl;
        result.result_type = UIResult::NONE;
    }

    return result;
}

static FormResult defaultExtractor(UIManager& ui, UIResult result) {
    FormResult form_result;
    form_result.result_type = result;

    if (result == UIResult::CONFIRM) {
        /* extract all input field values using label as key */
        for (const auto& field : ui.getInputFields()) {
            form_result.values[field->getLabel()] = field->getValue();
        }
    }

    return form_result;
}

/* register a new form template */
void FormFactory::registerForm(const std::string& name,
                              std::function<void(UIManager&)> form_builder,
                              std::function<FormResult(UIManager&, UIResult)> result_extractor) {
    form_templates_[name] = {form_builder, result_extractor};
}

void FormFactory::registerForm(const std::string& name, std::function<void(UIManager&)> form_builder) {
    registerForm(name, form_builder, defaultExtractor);
}