#include "confirmation_form.hpp"
#include "form_factory.hpp"
#include <string>
#include <vector>

/* split a string into lines of a specified maximum length */
std::vector<std::string> splitTextIntoLines(const std::string& text, int max_line_length) {
    std::vector<std::string> lines;
    std::string current_line;

    for (size_t i = 0; i < text.length(); ++i) {
        current_line += text[i];

        /* check if we need to split */
        if (current_line.length() >= static_cast<size_t>(max_line_length)) {
            /* try to find a space to split */
            size_t last_space = current_line.rfind(' ');
            if (last_space != std::string::npos) {
                lines.push_back(current_line.substr(0, last_space));
                current_line = current_line.substr(last_space + 1);
                /* put back the current character if it wasn't part of the last space */
                if (i > 0 && text[i] != ' ') {
                    --i;
                }
            } else {
                /* no space found, split at max_line_length */
                lines.push_back(current_line);
                current_line.clear();
            }
        } else if (text[i] == '\n') {
            /* handle explicit newlines */
            current_line.pop_back(); /* remove the newline */
            lines.push_back(current_line);
            current_line.clear();
        }
    }

    /* add the remaining text */
    if (!current_line.empty()) {
        lines.push_back(current_line);
    }

    return lines;
}

/* build the form UI */
void ConfirmationForm::build(UIManager& ui) {
    /* enable auto arrangement for buttons */
    ui.setAutoArrangeButtons(true);

    ui.addHeader("Confirmation");

    /* get screen size for proper positioning */
    int screen_rows, screen_cols;
    ui.getScreenSize(screen_rows, screen_cols);

    /* use custom message from context if provided, otherwise use default */
    std::string message = current_context_.message.empty() ? DEFAULT_MESSAGE : current_context_.message;

    /* split message into lines with reasonable maximum width */
    const int max_line_width = screen_cols - 10; /* leave margin */
    std::vector<std::string> message_lines = splitTextIntoLines(message, max_line_width);

    /* calculate starting row for centered text */
    int start_row = (screen_rows / 2) - (message_lines.size() / 2);
    if (start_row < 3) start_row = 3; /* ensure space below header */

    /* draw each line of the message, centered */
    for (size_t i = 0; i < message_lines.size(); ++i) {
        const std::string& line = message_lines[i];
        int col = (screen_cols - line.length()) / 2;
        if (col < 2) col = 2; /* minimum margin */
        ui.drawText(start_row + i, col, line);
    }

    /* add buttons - positions will be automatically arranged by UIManager */
    ui.addConfirmButton("Yes", -1, -1);
    ui.addCancelButton("Cancel", -1, -1);
    ui.addRejectButton("No", -1, -1);
}
