#ifndef FORM_INTERFACE_HPP
#define FORM_INTERFACE_HPP

#include <string>
#include <map>
#include "form_factory.hpp"

/* pure virtual interface for all forms */
class FormInterface {
public:
    /* virtual destructor to ensure proper cleanup */
    virtual ~FormInterface() = default;

    /* set form context - used to pass parameters to the form */
    virtual void setContext(const FormContext& context) {
        current_context_ = context;
    };

    /* clear form context - used to reset any stored context */
    virtual void clearContext() {
        current_context_ = FormContext();
    };

    /* get current form context */
    virtual FormContext getContext() const { return current_context_; };

    /* build the form UI */
    virtual void build(UIManager& ui) = 0;

    /* extract form result */
    virtual FormResult extractResult(UIManager& ui, UIResult result) {
        FormResult form_result;
        form_result.result_type = result;

        if (result == UIResult::CONFIRM) {
            /* extract all input field values using label as key */
            for (const auto& field : ui.getInputFields()) {
                form_result.values[field->getName()] = field->getValue();
            }
        }

    return form_result;
    };

protected:
    /* current context for the form */
    FormContext current_context_;
};

#endif /* FORM_INTERFACE_HPP */
