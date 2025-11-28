#ifndef UI_MANAGER_HPP
#define UI_MANAGER_HPP

#include <ncurses.h>
#include <string>
#include "header.hpp"
#include "input_field.hpp"
#include "button.hpp"
#include <vector>
#include <memory>

/* UI result type */
enum class UIResult {
    NONE,
    CONFIRM,
    CANCEL,
    REJECT,
    ESC
};

class UIManager {
public:
    UIManager();
    virtual ~UIManager();

    /**
     * initialize the UI manager
     * @return true if initialization succeeds
     */
    bool initialize();

    /**
     * shutdown the UI manager
     */
    void shutdown();

    /**
     * add header component
     * @param title title text
     * @param row row position
     * @param centered whether to center the title
     * @return pointer to the created header
     */
    Header* addHeader(const std::string& title, int row = 0, bool centered = true);

    /**
     * add input field component with default display width (capacity + 1)
     * @param name field name
     * @param label field label
     * @param row row position
     * @param col column position
     * @param capacity maximum characters allowed
     * @param type input type
     * @param default_value default value
     * @return pointer to the created input field
     */
    InputField* addInputField(const std::string& name, const std::string& label,
                             int row, int col, int capacity,
                             InputType type = InputType::STRING,
                             const std::string& default_value = "");

    /**
     * add input field component with explicit display width
     * @param name field name
     * @param label field label
     * @param row row position
     * @param col column position
     * @param capacity maximum characters allowed
     * @param display_width visible display width
     * @param type input type
     * @param default_value default value
     * @return pointer to the created input field
     */
    InputField* addInputField(const std::string& name, const std::string& label,
                             int row, int col, int capacity, int display_width,
                             InputType type = InputType::STRING,
                             const std::string& default_value = "");

    /**
     * add button component
     * @param text button text
     * @param row row position
     * @param col column position
     * @param type button type
     * @return pointer to the created button
     */
    Button* addButton(const std::string& text, int row, int col, ButtonType type = ButtonType::CUSTOM);

    /**
     * add confirm button (convenience method)
     * @param text button text (default: "OK")
     * @param row row position
     * @param col column position
     * @return pointer to the created button
     */
    Button* addConfirmButton(const std::string& text = "OK", int row = -1, int col = -1);

    /**
     * add cancel button (convenience method)
     * @param text button text (default: "Cancel")
     * @param row row position
     * @param col column position
     * @return pointer to the created button
     */
    Button* addCancelButton(const std::string& text = "Cancel", int row = -1, int col = -1);

    /**
     * add reject button (convenience method)
     * @param text button text (default: "Reject")
     * @param row row position
     * @param col column position
     * @return pointer to the created button
     */
    Button* addRejectButton(const std::string& text = "Reject", int row = -1, int col = -1);

    /**
     * set focus to the next focusable component
     */
    void focusNext();

    /**
     * set focus to the previous focusable component
     */
    void focusPrevious();

    /**
     * draw all components
     */
    void drawAll();

    /**
     * run the UI main loop
     * @return UI result (confirm, cancel, etc.)
     */
    UIResult run();

    /**
     * clear all components
     */
    void clearComponents();

    /**
     * handle key input and distribute to focused component
     * @param key key code
     * @return UI result based on key input
     */
    UIResult handleKey(int key);

    /**
     * clear the screen
     */
    void clearScreen();

    /**
     * refresh the screen
     */
    void refreshScreen();

    /**
     * get screen dimensions
     * @param rows reference to store number of rows
     * @param cols reference to store number of columns
     */
    void getScreenSize(int& rows, int& cols);

    /**
     * set cursor visibility
     * @param visible true to show cursor, false to hide
     */
    void setCursorVisible(bool visible);

    /**
     * draw text at specified position
     * @param row row position
     * @param col column position
     * @param text text to draw
     */
    void drawText(int row, int col, const std::string& text);

    /**
     * draw text with attributes at specified position
     * @param row row position
     * @param col column position
     * @param text text to draw
     * @param attr ncurses attributes (e.g., A_BOLD, A_REVERSE)
     */
    void drawTextWithAttr(int row, int col, const std::string& text, chtype attr);

    /**
     * check if tui is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const;

    /**
     * get all input fields
     * @return reference to input fields vector
     */
    const std::vector<std::shared_ptr<InputField>>& getInputFields() const;

private:
    /**
     * focus a specific component
     * @param index component index
     */
    void setFocusIndex(int index);

    bool initialized_;                                            /**< flag indicating if tui is initialized */
    std::vector<std::shared_ptr<Header>> headers_;                /**< header components */
    std::vector<std::shared_ptr<InputField>> input_fields_;       /**< input field components */
    std::vector<std::shared_ptr<Button>> buttons_;                /**< button components */
    std::vector<void*> focus_order_;                              /**< focusable components in order */
    int current_focus_index_;                                     /**< current focused component index */
    Button* confirm_button_;                                      /**< pointer to confirm button */
    Button* cancel_button_;                                       /**< pointer to cancel button */
    Button* reject_button_;                                       /**< pointer to reject button */
};

#endif /* UI_MANAGER_HPP */
