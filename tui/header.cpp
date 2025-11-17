#include "header.hpp"
#include <ncurses.h>

Header::Header(const std::string& title)
    : title_(title),
      row_(0),
      centered_(true),
      attribute_(A_BOLD) {
}

Header::~Header() {
}

void Header::setTitle(const std::string& title) {
    title_ = title;
}

std::string Header::getTitle() const {
    return title_;
}

void Header::setPosition(int row) {
    row_ = row;
}

void Header::setCentered(bool centered) {
    centered_ = centered;
}

void Header::setAttribute(chtype attr) {
    attribute_ = attr;
}

void Header::draw() {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    /* calculate column position */
    int col = 0;
    if (centered_) {
        col = (cols - static_cast<int>(title_.length())) / 2;
        /* ensure column is at least 0 */
        col = (col < 0) ? 0 : col;
    }

    /* clear the entire row */
    for (int c = 0; c < cols; ++c) {
        mvaddch(row_, c, ' ');
    }

    /* set attribute and draw title */
    attron(attribute_);
    mvprintw(row_, col, "%s", title_.c_str());
    attroff(attribute_);
}
