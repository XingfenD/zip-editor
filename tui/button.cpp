#include "button.hpp"
#include <ncurses.h>

Button::Button(const std::string& text, int row, int col, ButtonType type)
    : text_(text),
      row_(row),
      col_(col),
      type_(type),
      focused_(false) {
}

Button::~Button() {
}

void Button::setText(const std::string& text) {
    text_ = text;
}

std::string Button::getText() const {
    return text_;
}

void Button::setPosition(int row, int col) {
    row_ = row;
    col_ = col;
}

ButtonType Button::getType() const {
    return type_;
}

void Button::setFocused(bool focused) {
    focused_ = focused;
    draw();
}

bool Button::isFocused() const {
    return focused_;
}

bool Button::handleKey(int key) {
    if (!focused_) {
        return false;
    }

    /* check for activation keys */
    if (key == '\n' || key == '\r' || key == ' ' || key == KEY_ENTER) {
        /* button activated */
        return true;
    }

    return false;
}

void Button::draw() {
    int width = getWidth();

    /* draw button with borders */
    if (focused_) {
        /* highlight with reverse attribute when focused */
        attron(A_REVERSE);
    }

    /* draw left border */
    mvaddch(row_, col_, '[');

    /* draw spaces for button body */
    for (int i = 1; i < width - 1; ++i) {
        mvaddch(row_, col_ + i, ' ');
    }

    /* draw right border */
    mvaddch(row_, col_ + width - 1, ']');

    /* center the text */
    int text_col = col_ + 1 + (width - 2 - static_cast<int>(text_.length())) / 2;
    mvprintw(row_, text_col, "%s", text_.c_str());

    if (focused_) {
        attroff(A_REVERSE);
    }
}

int Button::getWidth() const {
    /* minimum width is 4 (for [ ]) plus text length */
    return 2 + text_.length() + 2; /* [ text ] */
}

bool Button::isInside(int row, int col) const {
    return (row == row_ && col >= col_ && col < col_ + getWidth());
}
