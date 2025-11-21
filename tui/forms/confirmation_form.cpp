#include "confirmation_form.hpp"
#include "form_factory.hpp"

/* build the form UI */
void ConfirmationForm::build(UIManager& ui) {
    ui.addHeader("Confirmation");
    /* use custom message from context if provided, otherwise use default */
    ui.drawText(5, 10, current_context_.message.empty() ? DEFAULT_MESSAGE : current_context_.message);
    ui.addConfirmButton("Yes");
    ui.addCancelButton("No");
}

/* extract form result */
FormResult ConfirmationForm::extractResult(UIManager&, UIResult result) {
    FormResult form_result;
    form_result.result_type = result;
    return form_result;
}
