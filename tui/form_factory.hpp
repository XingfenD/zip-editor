#ifndef FORM_FACTORY_HPP
#define FORM_FACTORY_HPP

#include "ui_manager.hpp"
#include <map>
#include <string>
#include <functional>
#include <memory>

/* form result structure to store form values */
struct FormResult {
    UIResult result_type;  /* confirm, cancel, etc. */
    std::map<std::string, std::string> values;  /* field name -> value mapping */
};

/* form factory class for predefined forms */
class FormFactory {
public:
    /* singleton instance */
    static FormFactory& getInstance();

    /* initialize all predefined forms */
    void initializeForms();

    /* show a predefined form and return its result */
    FormResult showForm(const std::string& form_name);

    /* show a predefined form with custom default values and return its result */
    FormResult showForm(const std::string& form_name, const std::map<std::string, std::string>& custom_defaults);

    /* register a new form template */
    void registerForm(const std::string& name, std::function<void(UIManager&)> form_builder);
    void registerForm(const std::string& name,
                      std::function<void(UIManager&)> form_builder,
                      std::function<FormResult(UIManager&, UIResult)> result_extractor);

private:
    FormFactory() = default;
    ~FormFactory() = default;

    /* form template structure */
    struct FormTemplate {
        std::function<void(UIManager&)> builder;  /* function to build the form */
        std::function<FormResult(UIManager&, UIResult)> extractor;  /* function to extract results */
    };

    /* registered form templates */
    std::map<std::string, FormTemplate> form_templates_;

    /* whether forms have been initialized */
    bool initialized_ = false;
};

#endif /* FORM_FACTORY_HPP */