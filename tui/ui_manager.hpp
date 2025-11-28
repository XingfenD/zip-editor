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

/* focus type for UI components */
enum class FocusType {
    INPUT_FIELD,
    BUTTON,
    HEADER
};

/**
 * UI manager class
 * handles all UI components and input
 */
class UIManager {
public:
    /**
     * constructor
     */
    UIManager();

    /**
     * destructor
     */
    ~UIManager();

    /**
     * initialize UI
     */
    bool initialize();

    /**
     * shutdown UI
     */
    void shutdown();

    /**
     * clear screen
     */
    void clearScreen();

    /**
     * refresh screen
     */
    void refreshScreen();

    /**
     * get screen size
     * @param rows screen rows
     * @param cols screen columns
     */
    void getScreenSize(int& rows, int& cols);

    /**
     * draw text at position
     * @param row row position
     * @param col column position
     * @param text text to draw
     */
    void drawText(int row, int col, const std::string& text);

    /**
     * set cursor visibility
     * @param visible whether cursor is visible
     */
    void setCursorVisible(bool visible);

    /**
     * set cursor position
     * @param row row position
     * @param col column position
     */
    void setCursorPosition(int row, int col);

    /**
     * add header to UI
     * @param text header text
     * @param col column position
     * @param row row position
     * @return header component
     */
    Header* addHeader(const std::string& text, int col = -1, int row = -1);

    /**
     * add input field to UI
     * @param label field label
     * @param value initial value
     * @param row row position
     * @param col column position
     * @return input field component
     */
    InputField* addInputField(const std::string& label, const std::string& value = "", int row = -1, int col = -1);

    /**
     * add input field to UI with more options
     * @param name field name
     * @param label field label
     * @param row row position
     * @param col column position
     * @param capacity maximum capacity
     * @param type input type
     * @param defaultValue default value
     * @return input field component
     */
    InputField* addInputField(const std::string& name, const std::string& label, int row, int col, int capacity, InputType type, const std::string& defaultValue);

    /**
     * add button to UI
     * @param text button text
     * @param row row position
     * @param col column position
     * @param type button type
     * @return button component
     */
    Button* addButton(const std::string& text, int row = -1, int col = -1, ButtonType type = ButtonType::CUSTOM);

    /**
     * add confirm button to UI
     * @param text button text
     * @param row row position
     * @param col column position
     * @return button component
     */
    Button* addConfirmButton(const std::string& text = "Confirm", int row = -1, int col = -1);

    /**
     * add cancel button to UI
     * @param text button text
     * @param row row position
     * @param col column position
     * @return button component
     */
    Button* addCancelButton(const std::string& text = "Cancel", int row = -1, int col = -1);

    /**
     * add reject button to UI
     * @param text button text
     * @param row row position
     * @param col column position
     * @return button component
     */
    Button* addRejectButton(const std::string& text = "Reject", int row = -1, int col = -1);

    /**
     * focus next component
     */
    void focusNext();

    /**
     * focus previous component
     */
    void focusPrevious();

    /**
     * focus a specific component
     * @param index component index
     */
    void setFocusIndex(int index);

    /**
     * run the UI main loop
     * @return UI result (confirm, cancel, etc.)
     */
    UIResult run();

    /**
     * enable or disable automatic button arrangement
     * @param enabled whether to enable auto arrangement
     */
    void setAutoArrangeButtons(bool enabled);

    /**
     * arrange buttons in a row with proper spacing
     * this method should be called after all buttons are added
     */
    void arrangeButtons();

    /**
     * get all headers
     * @return headers
     */
    const std::vector<std::shared_ptr<Header>>& getHeaders() const;

    /**
     * get all input fields
     * @return input fields
     */
    const std::vector<std::shared_ptr<InputField>>& getInputFields() const;

    /**
     * get all buttons
     * @return buttons
     */
    const std::vector<std::shared_ptr<Button>>& getButtons() const;

    /**
     * check if UI is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const;

private:
    /**
     * handle key input
     * @param key key code
     * @return UI result
     */
    UIResult handleKey(int key);

    /**
     * draw all UI components
     */
    void drawAll();

    /**
     * clear all UI components
     */
    void clearComponents();

    /**
     * draw text with attributes at position
     * @param row row position
     * @param col column position
     * @param text text to draw
     * @param attr text attributes
     */
    void drawTextWithAttr(int row, int col, const std::string& text, chtype attr);

    bool initialized_;                                            /**< flag indicating if tui is initialized */
    std::vector<std::shared_ptr<Header>> headers_;                /**< header components */
    std::vector<std::shared_ptr<InputField>> input_fields_;       /**< input field components */
    std::vector<std::shared_ptr<Button>> buttons_;                /**< button components */
    std::vector<void*> focus_order_;                              /**< focus order of components */
    int current_focus_index_;                                     /**< current focused component index */
    Button* confirm_button_;                                      /**< pointer to confirm button */
    Button* cancel_button_;                                       /**< pointer to cancel button */
    Button* reject_button_;                                       /**< pointer to reject button */
    bool auto_arrange_buttons_ = true;                            /**< whether to automatically arrange buttons */
};

#endif /* UI_MANAGER_HPP */
