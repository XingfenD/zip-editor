#ifndef HEADER_HPP
#define HEADER_HPP

#include <string>
#include <ncurses.h>

class Header {
public:
    Header(const std::string& title);
    virtual ~Header();

    /**
     * set title text
     * @param title new title text
     */
    void setTitle(const std::string& title);

    /**
     * get title text
     * @return current title text
     */
    std::string getTitle() const;

    /**
     * set title position
     * @param row row position
     */
    void setPosition(int row);

    /**
     * set title alignment
     * @param centered true to center the title, false to left-align
     */
    void setCentered(bool centered);

    /**
     * set title attribute
     * @param attr ncurses attribute (e.g., A_BOLD, A_REVERSE)
     */
    void setAttribute(chtype attr);

    /**
     * draw the title
     */
    void draw();

private:
    std::string title_;    /**< title text */
    int row_;              /**< row position */
    bool centered_;        /**< whether title is centered */
    chtype attribute_;     /**< title attribute */
};

#endif /* HEADER_HPP */
