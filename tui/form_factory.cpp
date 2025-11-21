#include "form_factory.hpp"
#include "signal_manager.hpp"
#include <iostream>
#include "debug_helper.hpp"
#include "field_descriptor.hpp"
#include "utils.hpp"
#include "confirmation_form.hpp"
#include "edit_lfh_form.hpp"

/* get singleton instance */
FormFactory& FormFactory::getInstance() {
    static FormFactory instance;
    return instance;
}

void inputFieldAdder(UIManager& ui, const std::vector<InputDescriptor>& input_descriptors, int base_row, int base_col) {
    for (const auto& d: input_descriptors) {
        int bytes = d.getField().getBytes();
        int capacity = (bytes > 0) ? bytes * 2 : -1;
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
    int related_expected_len = hexStrToInt(src_result);


    /* branch by related field type */
    switch (relatedField.getType()) {
        case FieldType::HEX:
            if (related_result.length() / 2 != static_cast<size_t>(related_expected_len)) {
                return false;
            }
            break;
        case FieldType::STRING:
            if (related_result.length() != static_cast<size_t>(related_expected_len)) {
                return false;
            }
            break;
    }

    return true;
}

/* initialize form factory - creates and registers all form instances */
void FormFactory::initialize() {
    if (initialized_) {
        return;
    }

    /* create form instances and register with FormInterface pointers */
    registerForm("confirmation", std::make_unique<ConfirmationForm>());

    registerForm("edit_lfh", std::make_unique<EditLfhForm>());

    initialized_ = true;
}

FormResult FormFactory::showForm(const std::string& form_name) {
    /* call the context-based showForm with empty context */
    return showForm(form_name, FormContext{});
}

FormResult FormFactory::showForm(const std::string& form_name, const std::map<std::string, std::string>& custom_defaults) {
    /* create context with custom defaults */
    FormContext context;
    context.custom_defaults = custom_defaults;

    /* call the context-based showForm */
    return showForm(form_name, context);
}

/* show a form with custom context and return its result */
FormResult FormFactory::showForm(const std::string& form_name, const FormContext& context) {
    /* ensure forms are initialized */
    if (!initialized_) {
        initialize();
    }

    /* check if form exists */
    auto it = form_mapping_.find(form_name);
    if (it == form_mapping_.end()) {
        DEBUG_LOG_FMT("Error: Form template '%s' not found!", form_name.c_str());
        FormResult error_result;
        error_result.result_type = UIResult::NONE;
        return error_result;
    }

    /* get form template */
    const auto& form2show = it->second;

    /* set form context using polymorphic interface if available */
    form2show->setContext(context);

    FormResult result;

    /* create UI manager */
    UIManager ui;
    if (!ui.initialize()) {
        std::cerr << "Failed to initialize UI" << std::endl;
        result.result_type = UIResult::NONE;
        return result;
    }

    /* initialize signal manager if not already initialized */
    SignalManager::initialize();

    /* register SIGINT handler */
    SignalManager::SignalGuard sigintGuard(SIGINT, [&ui](int) {
        if (ui.isInitialized()) {
            ui.shutdown();
        }
    });

    /* build the form */
    form2show->build(ui);

    /* run the UI loop and get result */
    UIResult ui_result = ui.run();

    /* extract form result */
    result = form2show->extractResult(ui, ui_result);

    /* clear form context using polymorphic interface if available */
    form2show->clearContext();

    /* return result */
    return result;
}

/* register a new form template */
void FormFactory::registerForm(const std::string& name, std::unique_ptr<FormInterface> form_interface) {
    form_mapping_[name] = std::move(form_interface);
}
