#include "edit_lfh_form.hpp"
#include "form_factory.hpp"
#include "signal_manager.hpp"
#include "debug_helper.hpp"
#include "field_descriptor.hpp"
#include "utils.hpp"

void EditLfhForm::addInputFields(UIManager& ui) {
    int base_row = 5;
    int base_col = 10;

    for (const auto& d : LOCAL_FILE_HEADER_INPUT_DESCRIPTORS) {
        int bytes = d.getField().getBytes();
        int capacity = (bytes > 0) ? bytes * 2 : -1;
        ui.addInputField(d.getField().getName(), d.getField().getTitle() + ":",
                        base_row, base_col, capacity,
                        fieldTypeToInputType(d.getField().getType()),
                        d.getDefaultValue());
        base_row += 2;
    }
}

bool EditLfhForm::checkRelatedFields(const std::map<std::string, std::string>& inputMap) {
    for (const auto& pair : LOCAL_FILE_HEADER_RELATED_FIELDS) {
        const FieldDescriptor& field = pair.key;
        const FieldDescriptor& related_field = pair.value;

        auto src_it = inputMap.find(field.getName());
        auto related_it = inputMap.find(related_field.getName());

        if (src_it == inputMap.end() || related_it == inputMap.end()) {
            continue;
        }

        std::string src_result = src_it->second;
        std::string related_result = related_it->second;

        /* get the length of related field from src field */
        if (field.getType() != FieldType::HEX) {
            throw std::invalid_argument("Source field must be HEX type");
        }
        int related_expected_len = hexStrToInt(src_result);

        /* branch by related field type */
        switch (related_field.getType()) {
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
    }
    return true;
}

void EditLfhForm::build(UIManager& ui) {
    /* add header using context message or default */
    if (!current_context_.message.empty()) {
        ui.addHeader(current_context_.message);
    } else {
        ui.addHeader("Edit Local File Header");
    }

    addInputFields(ui);

    /* add buttons */
    ui.addConfirmButton();
    ui.addCancelButton();
}

FormResult EditLfhForm::extractResult(UIManager& ui, UIResult result) {
    FormResult form_result;
    form_result.result_type = result;

    if (result == UIResult::CANCEL || result == UIResult::NONE || result == UIResult::ESC) {
        return form_result;
    }

    /* collect input field values using name as key */
    for (const auto& field : ui.getInputFields()) {
        form_result.values[field->getName()] = field->getValue();
    }

    /* check related fields for consistency */
    if (!checkRelatedFields(form_result.values)) {
        form_result.values[LFH_LENGTH_UNMATCH_KEY] = "true";
    }

    return form_result;
}
