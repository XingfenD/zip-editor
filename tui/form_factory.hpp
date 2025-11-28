#ifndef FORM_FACTORY_HPP
#define FORM_FACTORY_HPP

#include "ui_manager.hpp"
#include <map>
#include <string>
#include <functional>
#include <memory>

/* forward declarations */
class FormInterface;
class ConfirmationForm;
class EditLfhForm;

/* form result structure */
struct FormResult {
    UIResult result_type;
    std::map<std::string, std::string> values;
};

/* form context for passing custom parameters */
struct FormContext {
    std::string message;
    std::map<std::string, std::string> custom_defaults;

    FormContext() = default;
    FormContext(const std::string& message) : message(message) {}
};

/* form factory class for predefined forms */
class FormFactory {
public:
    /* singleton instance */
    static FormFactory& getInstance();

    /* initialize form factory - registers all form classes */
    void initialize();

    /* show a predefined form and return its result */
    FormResult showForm(const std::string& form_name);

    /* show a predefined form with custom default values and return its result */
    FormResult showForm(const std::string& form_name, const std::map<std::string, std::string>& custom_defaults);

    /* show a form with custom context and return its result */
    FormResult showForm(const std::string& form_name, const FormContext& context);

    /* register a new form template */
    void registerForm(const std::string& name, std::unique_ptr<FormInterface> form_interface = nullptr);

private:
    FormFactory() = default;
    ~FormFactory() = default;

    /* registered form templates */
    std::map<std::string, std::unique_ptr<FormInterface>> form_mapping_;

    /* whether factory has been initialized */
    bool initialized_ = false;
};

#endif /* FORM_FACTORY_HPP */
