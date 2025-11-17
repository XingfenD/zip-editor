#ifndef INPUT_FIELD_HPP
#define INPUT_FIELD_HPP

#include <string>

/* input field type */
enum class InputType {
    STRING,  /**< string input */
    HEX      /**< hexadecimal input */
};

class InputField {
public:
    /**
     * constructor
     * @param label field label text
     * @param row row position
     * @param col column position
     * @param width input field width
     * @param type input type (string or hex)
     * @param default_value default value
     */
    InputField(const std::string& label, int row, int col, int width,
               InputType type = InputType::STRING, const std::string& default_value = "");

    virtual ~InputField();

    /**
     * set field label
     * @param label new label text
     */
    void setLabel(const std::string& label);

    /**
     * get field label
     * @return current label text
     */
    std::string getLabel() const;

    /**
     * set field position
     * @param row row position
     * @param col column position
     */
    void setPosition(int row, int col);

    /**
     * set field width
     * @param width new width
     */
    void setWidth(int width);

    /**
     * set input type
     * @param type input type
     */
    void setInputType(InputType type);

    /**
     * set default value
     * @param default_value default value
     */
    void setDefaultValue(const std::string& default_value);

    /**
     * get current value
     * @return current input value
     */
    std::string getValue() const;

    /**
     * set current value
     * @param value new value
     */
    void setValue(const std::string& value);

    /**
     * set focus state
     * @param focused true if field has focus
     */
    void setFocused(bool focused);

    /**
     * check if field has focus
     * @return true if field has focus
     */
    bool isFocused() const;

    /**
     * get cursor position
     * @param row output parameter for row position
     * @param col output parameter for column position
     */
    void getCursorPosition(int& row, int& col) const;

    /**
     * handle key input
     * @param key key code
     * @return true if key was handled
     */
    bool handleKey(int key);

    /**
     * draw the input field
     */
    void draw();

private:
    /**
     * validate input character based on input type
     * @param c character to validate
     * @return true if character is valid
     */
    bool isValidChar(char c) const;

    /**
     * move cursor left
     */
    void moveCursorLeft();

    /**
     * move cursor right
     */
    void moveCursorRight();

    /**
     * insert character at cursor position
     * @param c character to insert
     */
    void insertChar(char c);

    /**
     * delete character at cursor position
     */
    void deleteChar();

    /**
     * delete character before cursor position
     */
    void backspace();

    std::string label_;          /**< field label */
    std::string value_;          /**< input value */
    std::string default_value_;  /**< default value */
    int row_;                    /**< row position */
    int col_;                    /**< column position */
    int width_;                  /**< field width */
    int cursor_pos_;             /**< cursor position within value */
    InputType type_;             /**< input type */
    bool focused_;               /**< focus state */
};

#endif /* INPUT_FIELD_HPP */
