#ifndef CONFIRMATION_FORM_HPP
#define CONFIRMATION_FORM_HPP

#include <string>
#include <map>
#include "form_interface.hpp"

/* confirmation form class */
class ConfirmationForm : public FormInterface {
public:
    /* constructor and destructor */
    ConfirmationForm() = default;
    virtual ~ConfirmationForm() = default;

    /* build the form UI */
    void build(UIManager& ui) override;
private:
    /* default confirmation message */
    static constexpr const char* DEFAULT_MESSAGE = "Are you sure you want to proceed?";
};

#endif /* CONFIRMATION_FORM_HPP */
