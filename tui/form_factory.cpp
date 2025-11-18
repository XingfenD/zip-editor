#include "form_factory.hpp"
#include "signal_manager.hpp"
#include <iostream>
#include "debug_helper.hpp"
#include "field_descriptor.hpp"
#include "utils.hpp"

/* get singleton instance */
FormFactory& FormFactory::getInstance() {
    static FormFactory instance;
    return instance;
}

void inputFieldAdder(UIManager& ui, const std::vector<InputDescriptor>& input_descriptors, int base_row, int base_col) {
    for (const auto& d: input_descriptors) {
        int capacity = d.getField().getBytes() * 2;
        if (capacity < 0) {
            capacity = 30;
        }
        ui.addInputField(d.getField().getName(), d.getField().getTitle() + ":", base_row, base_col, capacity, fieldTypeToInputType(d.getField().getType()), d.getDefaultValue());
        base_row += 2;
    }
}

bool checkRelatedFields(const FieldDescriptor& srcField, const FieldDescriptor& relatedField, std::map<std::string, std::string>& inputMap) {
    std::string src_result = inputMap[srcField.getName()];
    std::string related_result = inputMap[relatedField.getName()];
    /* get the length of related field from src field */
    if (srcField.getType() != FieldType::HEX) {
        throw std::invalid_argument("Source field must be HEX type");
    }
    int related_len = hexStrToInt(src_result);

    /* branch by related field type */
    switch (relatedField.getType()) {
        case FieldType::HEX:
            if (related_result.length() / 2 != static_cast<size_t>(related_len)) {
                return false;
            }
            break;
        case FieldType::STRING:
            if (related_result.length() != static_cast<size_t>(related_len)) {
                return false;
            }
            break;
    }

    return true;
}

/* initialize all predefined forms */
void FormFactory::initializeForms() {
    if (initialized_) {
        return;
    }

    /* register common forms */
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

    registerForm("edit_lfh",
        [](UIManager& ui) {
            /* add header */
            ui.addHeader("Edit Local File Header");

            inputFieldAdder(ui, LOCAL_FILE_HEADER_INPUT_DESCRIPTORS, 5, 10);

            /* add buttons */
            ui.addConfirmButton();
            ui.addCancelButton();
        },
    [](UIManager& ui, UIResult result) {
        DEBUG_LOG("Running extractor ...");
        FormResult form_result;
        form_result.result_type = result;
        if (result == UIResult::CANCEL || result == UIResult::NONE || result == UIResult::ESC) {
            return form_result;
        }

        /* collect input field values using name as key */
        for (const auto& field : ui.getInputFields()) {
            form_result.values[field->getName()] = field->getValue();
        }

        for (const auto& pair : LOCAL_FILE_HEADER_RELATED_FIELDS) {
            const FieldDescriptor& field = pair.key;
            const FieldDescriptor& related_field = pair.value;
            DEBUG_LOG_FMT("Current Pair: key.name = %s, value.name = %s", field.getName().c_str(), field.getName().c_str());
            const std::string src_result = form_result.values[field.getName()];
            const std::string related_result = form_result.values[related_field.getName()];
            if (!checkRelatedFields(field, related_field, form_result.values)) {
                form_result.values[LFH_LENGTH_UNMATCH_KEY] = "true";
            }
        }

        return form_result;
    });

    initialized_ = true;
}

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
            form_result.values[field->getName()] = field->getValue();
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
