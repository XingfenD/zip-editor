#ifndef EDIT_LFH_FORM_HPP
#define EDIT_LFH_FORM_HPP

#include <string>
#include <map>
#include "form_interface.hpp"

/* edit local file header form class */
class EditLfhForm : public FormInterface {
public:
    /* constructor and destructor */
    EditLfhForm() = default;
    virtual ~EditLfhForm() = default;

    /* build the form UI */
    void build(UIManager& ui) override;

    /* extract form result */
    FormResult extractResult(UIManager& ui, UIResult result) override;

private:
    /* helper function to add input fields */
    static void addInputFields(UIManager& ui);

    /* check related fields for consistency */
    bool checkRelatedFields(const std::map<std::string, std::string>& inputMap);

    /* current context for the form */
    FormContext current_context_;
};

#endif /* EDIT_LFH_FORM_HPP */
