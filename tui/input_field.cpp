#include "input_field.hpp"
#include <ncurses.h>

InputField::InputField(const std::string& name, const std::string& label, int row, int col, int capacity,
                       InputType type, const std::string& default_value)
    : name_(name),
      label_(label),
      value_(default_value),
      default_value_(default_value),
      row_(row),
      col_(col),
      capacity_(capacity),
      display_width_(capacity + 1),
      cursor_pos_(static_cast<int>(default_value.length())),
      type_(type),
      focused_(false) {
    /* validate default value against capacity */
    if (value_.length() > static_cast<size_t>(capacity_)) {
        value_ = value_.substr(0, capacity_);
        cursor_pos_ = capacity_;
    }
}

InputField::InputField(const std::string& name, const std::string& label, int row, int col, int capacity, int display_width,
                       InputType type, const std::string& default_value)
    : name_(name),
      label_(label),
      value_(default_value),
      default_value_(default_value),
      row_(row),
      col_(col),
      capacity_(capacity),
      display_width_(display_width),
      cursor_pos_(static_cast<int>(default_value.length())),
      type_(type),
      focused_(false) {
}

InputField::~InputField() {}

std::string InputField::getName() const {
    return name_;
}

void InputField::setLabel(const std::string& label) {
    label_ = label;
}

std::string InputField::getLabel() const {
    return label_;
}

void InputField::setPosition(int row, int col) {
    row_ = row;
    col_ = col;
}

void InputField::setCapacity(int capacity) {
    capacity_ = capacity;
    /* truncate value if it exceeds new capacity */
    if (value_.length() > static_cast<size_t>(capacity_)) {
        value_ = value_.substr(0, capacity_);
        if (cursor_pos_ > capacity_) {
            cursor_pos_ = capacity_;
        }
    }
}

int InputField::getCapacity() const {
    return capacity_;
}

void InputField::setDisplayWidth(int display_width) {
    display_width_ = display_width;
}

int InputField::getDisplayWidth() const {
    return display_width_;
}

void InputField::setInputType(InputType type) {
    type_ = type;
    /* validate current value against new type */
    std::string new_value;
    for (char c : value_) {
        if (isValidChar(c)) {
            new_value += c;
        }
    }
    value_ = new_value;
    /* adjust cursor position */
    if (cursor_pos_ > static_cast<int>(value_.length())) {
        cursor_pos_ = static_cast<int>(value_.length());
    }
}

void InputField::setDefaultValue(const std::string& default_value) {
    default_value_ = default_value;
    if (value_.empty()) {
        value_ = default_value;
        cursor_pos_ = static_cast<int>(default_value.length());
    }
}

std::string InputField::getValue() const {
    return value_.empty() ? default_value_ : value_;
}

void InputField::setValue(const std::string& value) {
    /* validate input based on type */
    std::string validated_value;
    for (char c : value) {
        if (isValidChar(c)) {
            validated_value += c;
        }
    }
    value_ = validated_value;
    cursor_pos_ = static_cast<int>(value_.length());
}

void InputField::setFocused(bool focused) {
    if (focused && !focused_) {
        /* gain focus */
        /* position cursor - cursor visibility will be managed by UIManager */
        int display_start = 0;
        if (cursor_pos_ >= display_width_) {
            display_start = cursor_pos_ - display_width_ + 1;
        }
        int cursor_col = col_ + label_.length() + 2 + (cursor_pos_ - display_start);
        move(row_, cursor_col);
    }
    focused_ = focused;
    draw();
}

void InputField::getCursorPosition(int& row, int& col) const {
    row = row_;
    
    /* calculate display parameters to determine cursor column */
    int field_start_col = col_ + label_.length() + 2;
    int display_start = 0;
    if (cursor_pos_ >= display_width_) {
        display_start = cursor_pos_ - display_width_ + 1;
    }
    
    col = field_start_col + (cursor_pos_ - display_start);
}

bool InputField::isFocused() const {
    return focused_;
}

bool InputField::handleKey(int key) {
    if (!focused_) {
        return false;
    }

    bool handled = true;

    switch (key) {
        case KEY_LEFT:
            moveCursorLeft();
            break;
        case KEY_RIGHT:
            moveCursorRight();
            break;
        case KEY_BACKSPACE:
        case 127:  /* ASCII delete */
            backspace();
            break;
        case KEY_DC:  /* Delete key */
            deleteChar();
            break;
        case '\n':
        case '\r':
            /* enter key - typically handled by UI manager */
            handled = false;
            break;
        default:
            /* check if it's a printable character */
            if (key >= 32 && key <= 126) {
                if (isValidChar(static_cast<char>(key)) && value_.length() < static_cast<size_t>(capacity_)) {
                    insertChar(static_cast<char>(key));
                }
            } else {
                handled = false;
            }
            break;
    }

    if (handled) {
        draw();
        /* update cursor position */
        int display_start = 0;
        if (cursor_pos_ >= display_width_) {
            display_start = cursor_pos_ - display_width_ + 1;
        }
        int cursor_col = col_ + label_.length() + 2 + (cursor_pos_ - display_start);
        move(row_, cursor_col);
    }

    return handled;
}

void InputField::draw() {
    /* draw label */
    mvprintw(row_, col_, "%s: ", label_.c_str());

    /* calculate display parameters */
    int field_start_col = col_ + label_.length() + 2;
    int display_start = 0;
    if (cursor_pos_ >= display_width_) {
        display_start = cursor_pos_ - display_width_ + 1;
    }
    std::string display_value = value_.substr(display_start, display_width_);

    /* draw input field with border */
    if (focused_) {
        /* highlight with reverse attribute when focused */
        attron(A_REVERSE);
    }

    /* draw background */
    for (int i = 0; i < display_width_; ++i) {
        mvaddch(row_, field_start_col + i, ' ');
    }

    /* draw value */
    mvprintw(row_, field_start_col, "%s", display_value.c_str());

    if (focused_) {
        attroff(A_REVERSE);
    }

    /* if focused, update cursor position */
    if (focused_) {
        int cursor_col = field_start_col + (cursor_pos_ - display_start);
        move(row_, cursor_col);
    }
}

bool InputField::isValidChar(char c) const {
    switch (type_) {
        case InputType::STRING:
            /* allow any printable ASCII character for string input */
            return (c >= 32 && c <= 126);
        case InputType::HEX:
            /* allow hexadecimal characters (0-9, a-f, A-F) */
            return ((c >= '0' && c <= '9') ||
                    (c >= 'a' && c <= 'f') ||
                    (c >= 'A' && c <= 'F'));
        default:
            return false;
    }
}

void InputField::moveCursorLeft() {
    if (cursor_pos_ > 0) {
        --cursor_pos_;
    }
}

void InputField::moveCursorRight() {
    if (cursor_pos_ < static_cast<int>(value_.length())) {
        ++cursor_pos_;
    }
}

void InputField::insertChar(char c) {
    if (value_.length() < static_cast<size_t>(capacity_)) {
        value_.insert(cursor_pos_, 1, c);
        ++cursor_pos_;
    }
}

void InputField::deleteChar() {
    if (cursor_pos_ < static_cast<int>(value_.length())) {
        value_.erase(cursor_pos_, 1);
    }
}

void InputField::backspace() {
    if (cursor_pos_ > 0) {
        value_.erase(cursor_pos_ - 1, 1);
        --cursor_pos_;
    }
}
