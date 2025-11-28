#include "ui_manager.hpp"
#include <algorithm>
#include <iostream>
#include "debug_helper.hpp"
#include <termios.h>
#include <unistd.h>

UIManager::UIManager()
    : initialized_(false),
      current_focus_index_(-1),
      confirm_button_(nullptr),
      cancel_button_(nullptr),
      reject_button_(nullptr),
      auto_arrange_buttons_(true) {
}

UIManager::~UIManager() {
    shutdown();
}

bool UIManager::initialize() {
    if (initialized_) {
        return true;
    }

    /* save current terminal state before initializing ncurses */
    def_shell_mode();

    /* initialize ncurses */
    initscr();
    if (!stdscr) {
        return false;
    }
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    /* initialize colors if supported */
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_BLACK, COLOR_WHITE); // 选中状态
        init_pair(2, COLOR_RED, COLOR_BLACK);   // 错误状态
        init_pair(3, COLOR_GREEN, COLOR_BLACK); // 成功状态
        init_pair(4, COLOR_WHITE, COLOR_BLACK); /* default color pair */
    }
    curs_set(0);             /* hide cursor */
    mousemask(ALL_MOUSE_EVENTS, NULL); /* enable mouse events */

    initialized_ = true;
    return true;
}

void UIManager::clearComponents() {
    headers_.clear();
    input_fields_.clear();
    buttons_.clear();
    focus_order_.clear();
    current_focus_index_ = -1;
    confirm_button_ = nullptr;
    cancel_button_ = nullptr;
    reject_button_ = nullptr;
}

void UIManager::shutdown() {
    clearComponents();

    if (initialized_) {
        endwin();               /* end ncurses mode - this is crucial */

        reset_shell_mode();     /* restore previous terminal state */

        initialized_ = false;
    }
}

Header* UIManager::addHeader(const std::string& text, int col, int row) {
    auto header = std::make_shared<Header>(text);
    header->setPosition(row);

    headers_.push_back(header);

    return header.get();
}

InputField* UIManager::addInputField(const std::string& name, const std::string& label, int row, int col, int capacity,
                                     InputType type, const std::string& default_value) {
    auto input_field = std::make_shared<InputField>(name, label, row, col, capacity, type, default_value);
    input_fields_.push_back(input_field);

    /* add to focus order */
    focus_order_.push_back(input_field.get());

    /* if this is the first focusable component, set focus to it */
    if (current_focus_index_ == -1) {
        setFocusIndex(0);
    }

    return input_field.get();
}

InputField* UIManager::addInputField(const std::string& label, const std::string& value, int row, int col) {
    // 使用label作为name，设置默认capacity为-1（可变长度）
    auto field = std::make_shared<InputField>(label, label, row, col, -1, InputType::STRING, value);

    input_fields_.push_back(field);
    focus_order_.push_back(field.get());

    /* if this is the first focusable component, set focus to it */
    if (current_focus_index_ == -1) {
        setFocusIndex(0);
    }

    return field.get();
}

// 已保留上面的7参数版本addInputField方法

Button* UIManager::addButton(const std::string& text, int row, int col, ButtonType type) {
    /* when auto arrangement is enabled, we'll set row to -1 to indicate that
       position will be determined later */
    int actual_row = auto_arrange_buttons_ ? -1 : row;
    int actual_col = auto_arrange_buttons_ ? -1 : col;

    auto button = std::make_shared<Button>(text, actual_row, actual_col, type);
    buttons_.push_back(button);

    /* add to focus order */
    focus_order_.push_back(button.get());

    /* update confirm/cancel/reject button pointers */
    if (type == ButtonType::CONFIRM) {
        confirm_button_ = button.get();
    } else if (type == ButtonType::CANCEL) {
        cancel_button_ = button.get();
    } else if (type == ButtonType::REJECT) {
        reject_button_ = button.get();
    }

    /* if this is the first focusable component, set focus to it */
    if (current_focus_index_ == -1) {
        setFocusIndex(0);
    }

    return button.get();
}

Button* UIManager::addConfirmButton(const std::string& text, int row, int col) {
    /* when auto arrangement is enabled, we ignore the position parameters */
    if (auto_arrange_buttons_) {
        return addButton(text, -1, -1, ButtonType::CONFIRM);
    }

    /* if auto arrangement is disabled, use original position calculation */
    if (row == -1 || col == -1) {
        int screen_rows, screen_cols;
        getScreenSize(screen_rows, screen_cols);

        if (row == -1) {
            /* position buttons at the bottom of the screen */
            row = screen_rows - 3;
        }

        if (col == -1) {
            /* position in the middle, but shifted left */
            col = (screen_cols - text.length() - 4) / 2 - 10;
        }
    }

    return addButton(text, row, col, ButtonType::CONFIRM);
}

Button* UIManager::addCancelButton(const std::string& text, int row, int col) {
    /* when auto arrangement is enabled, we ignore the position parameters */
    if (auto_arrange_buttons_) {
        return addButton(text, -1, -1, ButtonType::CANCEL);
    }

    /* if auto arrangement is disabled, use original position calculation */
    if (row == -1 || col == -1) {
        int screen_rows, screen_cols;
        getScreenSize(screen_rows, screen_cols);

        if (row == -1) {
            /* position buttons at the bottom of the screen */
            row = screen_rows - 3;
        }

        if (col == -1) {
            /* position in the middle, but shifted right */
            col = (screen_cols - text.length() - 4) / 2 + 10;
        }
    }

    return addButton(text, row, col, ButtonType::CANCEL);
}

Button* UIManager::addRejectButton(const std::string& text, int row, int col) {
    /* when auto arrangement is enabled, we ignore the position parameters */
    if (auto_arrange_buttons_) {
        return addButton(text, -1, -1, ButtonType::REJECT);
    }

    /* if auto arrangement is disabled, use original position calculation */
    if (row == -1 || col == -1) {
        int screen_rows, screen_cols;
        getScreenSize(screen_rows, screen_cols);

        if (row == -1) {
            /* position buttons at the bottom of the screen */
            row = screen_rows - 3;
        }

        if (col == -1) {
            /* position in the middle, but shifted left more than confirm */
            col = (screen_cols - text.length() - 4) / 2 - 20;
        }
    }

    return addButton(text, row, col, ButtonType::REJECT);
}

void UIManager::focusNext() {
    if (focus_order_.empty()) {
        return;
    }

    /* reset current focus */
    if (current_focus_index_ >= 0 && current_focus_index_ < static_cast<int>(focus_order_.size())) {
        void* current = focus_order_[current_focus_index_];
        InputField* input = static_cast<InputField*>(current);
        Button* button = static_cast<Button*>(current);

        /* determine which type it is and set focus off */
        if (input_fields_.size() > 0) {
            /* check if it's an input field */
            auto it = std::find_if(input_fields_.begin(), input_fields_.end(),
                [input](const std::shared_ptr<InputField>& ptr) { return ptr.get() == input; });
            if (it != input_fields_.end()) {
                input->setFocused(false);
                /* hide cursor when unfocusing input field */
                setCursorVisible(false);
            } else {
                button->setFocused(false);
            }
        } else {
            button->setFocused(false);
        }
    }

    /* move to next focus */
    current_focus_index_ = (current_focus_index_ + 1) % focus_order_.size();
    setFocusIndex(current_focus_index_);
}

void UIManager::focusPrevious() {
    if (focus_order_.empty()) {
        return;
    }

    /* reset current focus */
    if (current_focus_index_ >= 0 && current_focus_index_ < static_cast<int>(focus_order_.size())) {
        void* current = focus_order_[current_focus_index_];
        InputField* input = static_cast<InputField*>(current);
        Button* button = static_cast<Button*>(current);

        /* determine which type it is and set focus off */
        if (input_fields_.size() > 0) {
            /* check if it's an input field */
            auto it = std::find_if(input_fields_.begin(), input_fields_.end(),
                [input](const std::shared_ptr<InputField>& ptr) { return ptr.get() == input; });
            if (it != input_fields_.end()) {
                input->setFocused(false);
                /* hide cursor when unfocusing input field */
                setCursorVisible(false);
            } else {
                button->setFocused(false);
            }
        } else {
            button->setFocused(false);
        }
    }

    /* move to previous focus */
    current_focus_index_ = (current_focus_index_ - 1 + focus_order_.size()) % focus_order_.size();
    setFocusIndex(current_focus_index_);
}

void UIManager::setFocusIndex(int index) {
    if (index < 0 || index >= static_cast<int>(focus_order_.size())) {
        return;
    }

    current_focus_index_ = index;
    void* component = focus_order_[index];

    /* determine component type and set focus */
    /* first check if it's an input field - only input fields should show cursor */
    for (const auto& input : input_fields_) {
        if (input.get() == component) {
            /* for input fields, show cursor */
            setCursorVisible(true);
            input->setFocused(true);
            return;
        }
    }

    /* for all other component types (buttons), ensure cursor is hidden */
    for (const auto& button : buttons_) {
        if (button.get() == component) {
            /* hide cursor before setting button focus */
            setCursorVisible(false);
            button->setFocused(true);
            return;
        }
    }

    /* if it's neither input field nor button, hide cursor */
    setCursorVisible(false);
}

void UIManager::drawAll() {
    clearScreen();

    /* if auto arrangement is enabled, arrange buttons before drawing */
    if (auto_arrange_buttons_ && !buttons_.empty()) {
        arrangeButtons();
    }

    /* draw headers */
    for (const auto& header : headers_) {
        header->draw();
    }

    /* draw input fields */
    for (const auto& input : input_fields_) {
        input->draw();
    }

    /* draw buttons */
    for (const auto& button : buttons_) {
        button->draw();
    }

    refreshScreen();

    /* strict cursor visibility control - only show for focused input fields */
    if (current_focus_index_ >= 0 && current_focus_index_ < static_cast<int>(focus_order_.size())) {
        void* component = focus_order_[current_focus_index_];

        /* check if focused component is an input field */
        bool isInputField = false;
        InputField* focusedInput = nullptr;

        for (const auto& input : input_fields_) {
            if (input.get() == component) {
                isInputField = true;
                focusedInput = input.get();
                break;
            }
        }

        /* only show cursor for input fields */
        if (isInputField && focusedInput && focusedInput->isFocused()) {
            setCursorVisible(true);
            /* ensure cursor is at the correct position */
            int row, col;
            focusedInput->getCursorPosition(row, col);
            move(row, col);
        } else {
            /* hide cursor for all other components */
            setCursorVisible(false);
        }
    } else {
        /* no focused component, hide cursor */
        setCursorVisible(false);
    }
}

UIResult UIManager::run() {
    if (!isInitialized()) {
        return UIResult::NONE;
    }

    /* initial draw */
    drawAll();

    UIResult result = UIResult::NONE;

    /* main loop */
    while (result == UIResult::NONE) {
        int key = getch();
        // DEBUG_LOG_FMT("key pressed: %d", key);
        result = handleKey(key);

        /* redraw after each key press */
        drawAll();
    }

    return result;
}

void UIManager::clearScreen() {
    if (initialized_) {
        clear();
    }
}

void UIManager::refreshScreen() {
    if (initialized_) {
        refresh();
    }
}

void UIManager::getScreenSize(int& rows, int& cols) {
    if (initialized_) {
        getmaxyx(stdscr, rows, cols);
    } else {
        rows = 0;
        cols = 0;
    }
}

void UIManager::setCursorVisible(bool visible) {
    if (initialized_) {
        curs_set(visible ? 1 : 0);
    }
}

void UIManager::drawText(int row, int col, const std::string& text) {
    if (initialized_) {
        mvprintw(row, col, "%s", text.c_str());
    }
}

void UIManager::drawTextWithAttr(int row, int col, const std::string& text, chtype attr) {
    if (initialized_) {
        attron(attr);
        mvprintw(row, col, "%s", text.c_str());
        attroff(attr);
    }
}

bool UIManager::isInitialized() const {
    return initialized_;
}

const std::vector<std::shared_ptr<InputField>>& UIManager::getInputFields() const {
    return input_fields_;
}

UIResult UIManager::handleKey(int key) {
    /* handle navigation keys */
    switch (key) {
        case '\t':  /* Tab key */
            focusNext();
            return UIResult::NONE;
        case KEY_BTAB:  /* Shift+Tab */
            focusPrevious();
            return UIResult::NONE;
        case 27:  /* Escape key */
            return UIResult::ESC;
    }

    /* handle focused component */
    if (current_focus_index_ >= 0 && current_focus_index_ < static_cast<int>(focus_order_.size())) {
        void* component = focus_order_[current_focus_index_];

        /* check if it's an input field */
        for (const auto& input : input_fields_) {
            if (input.get() == component) {
                input->handleKey(key);
                return UIResult::NONE;
            }
        }

        /* check if it's a button */
        for (const auto& button : buttons_) {
            if (button.get() == component) {
                if (button->handleKey(key)) {
                    /* button activated */
                    if (button.get() == confirm_button_ || button->getType() == ButtonType::CONFIRM) {
                        return UIResult::CONFIRM;
                    } else if (button.get() == cancel_button_ || button->getType() == ButtonType::CANCEL) {
                        return UIResult::CANCEL;
                    } else if (button.get() == reject_button_ || button->getType() == ButtonType::REJECT) {
                        return UIResult::REJECT;
                    }
                }
                return UIResult::NONE;
            }
        }
    }

    return UIResult::NONE;
}

void UIManager::setAutoArrangeButtons(bool enabled) {
    auto_arrange_buttons_ = enabled;
}

void UIManager::arrangeButtons() {
    if (buttons_.empty()) {
        return;
    }

    int screen_rows, screen_cols;
    getScreenSize(screen_rows, screen_cols);

    /* calculate total width needed for all buttons with spacing */
    int total_width = 0;
    const int button_spacing = 4; /* space between buttons */

    for (const auto& button : buttons_) {
        total_width += button->getWidth() + button_spacing;
    }

    /* adjust for extra spacing at the end */
    total_width -= button_spacing;

    /* calculate starting column to center all buttons */
    int start_col = (screen_cols - total_width) / 2;
    if (start_col < 2) {
        start_col = 2; /* minimum margin */
    }

    /* set button positions in a row at the bottom of the screen */
    int current_col = start_col;
    int button_row = screen_rows - 4; /* position above the bottom border */

    for (const auto& button : buttons_) {
        button->setPosition(button_row, current_col);
        current_col += button->getWidth() + button_spacing;
    }
}
