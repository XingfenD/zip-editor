#ifndef BUTTON_HPP
#define BUTTON_HPP

#include <string>

/* button type */
enum class ButtonType {
    CONFIRM,  /**< confirm button */
    CANCEL,   /**< cancel button */
    REJECT,   /**< reject button */
    CUSTOM    /**< custom button */
};

class Button {
public:
    /**
     * constructor
     * @param text button text
     * @param row row position
     * @param col column position
     * @param type button type
     */
    Button(const std::string& text, int row, int col, ButtonType type = ButtonType::CUSTOM);

    /**
     * destructor
     */
    virtual ~Button();

    /**
     * set button text
     * @param text new button text
     */
    void setText(const std::string& text);

    /**
     * get button text
     * @return current button text
     */
    std::string getText() const;

    /**
     * set button position
     * @param row row position
     * @param col column position
     */
    void setPosition(int row, int col);

    /**
     * get button type
     * @return button type
     */
    ButtonType getType() const;

    /**
     * set focus state
     * @param focused true if button has focus
     */
    void setFocused(bool focused);

    /**
     * check if button has focus
     * @return true if button has focus
     */
    bool isFocused() const;

    /**
     * handle key input
     * @param key key code
     * @return true if key was handled and button was activated
     */
    bool handleKey(int key);

    /**
     * draw the button
     */
    void draw();

    /**
     * get button width
     * @return button width
     */
    int getWidth() const;

    /**
     * check if position is inside button
     * @param row row position
     * @param col column position
     * @return true if position is inside button
     */
    bool isInside(int row, int col) const;

private:
    std::string text_;      /**< button text */
    int row_;               /**< row position */
    int col_;               /**< column position */
    ButtonType type_;       /**< button type */
    bool focused_;          /**< focus state */
};

#endif /* BUTTON_HPP */
