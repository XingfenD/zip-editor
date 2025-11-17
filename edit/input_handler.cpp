#include "input_handler.hpp"
#include <iostream>
#include <cctype>
#include <limits>
#include <algorithm>
#include "cmd_handler.hpp"

/* static member initialization */
std::map<char, std::shared_ptr<InputHandler>> InputHandlerFactory::handlers;

std::shared_ptr<InputHandler> InputHandlerFactory::getHandler(char c) {
    /* first check for exact match in the handler map */
    auto it = handlers.find(c);
    if (it != handlers.end()) {
        return it->second;
    }

    /* if no exact match, check all handlers for canHandle */
    for (const auto& [key, handler] : handlers) {
        if (handler->canHandle(c)) {
            return handler;
        }
    }

    /* no handler found */
    return nullptr;
}

void InputHandlerFactory::registerHandler(std::shared_ptr<InputHandler> handler) {
    handlers[handler->getTriggerChar()] = handler;
}

/* helper function to find commands matching the prefix */
std::vector<std::string> findMatchingCommands(const std::string& prefix) {
    std::vector<std::string> matches;
    std::vector<std::string> all_commands = CommandFactory::getAllCommands();

    for (const auto& cmd : all_commands) {
        if (cmd.compare(0, prefix.length(), prefix) == 0) {
            matches.push_back(cmd);
        }
    }

    /* sort matches alphabetically */
    std::sort(matches.begin(), matches.end());
    return matches;
}

/* enter key handler implementation */
class EnterKeyHandler : public InputHandler {
public:
    EnterKeyHandler() : InputHandler('\n') {}

    bool handle(InputContext& context) override {
        std::cout << std::endl;
        /* add non-empty command to history */
        if (!context.line.empty() && (context.history.empty() || context.line != context.history.back())) {
            context.history.push_back(context.line);
        }
        context.history_index = -1;    /* reset history index */
        context.current_input.clear();
        return false; /* return false to break the loop */
    }
};

/* backspace key handler implementation */
class BackspaceKeyHandler : public InputHandler {
public:
    BackspaceKeyHandler() : InputHandler(127) {}

    bool canHandle(char c) const override {
        return c == 127 || c == '\b'; /* handle both 127 and \b */
    }

    bool handle(InputContext& context) override {
        if (context.cursor_pos > 0) {
            /* move cursor back one position */
            std::cout << "\b" << std::flush;

            /* save cursor after character */
            std::string remaining = context.line.substr(context.cursor_pos);

            /* remove character at cursor position */
            context.line.erase(context.cursor_pos - 1, 1);
            context.cursor_pos--;

            /* redraw cursor after character (including space to clear last position) */
            std::cout << remaining << " " << std::flush;

            /* calculate number of characters to backtrack: remaining length + 1 space */
            int back_count = remaining.length() + 1;

            /* move cursor back to correct position */
            std::cout << std::string(back_count, '\b') << std::flush;
        }
        return true;
    }
};

/* command+backspace key handler for macOS */
class MacOSDeleteKeyHandler : public InputHandler {
public:
    MacOSDeleteKeyHandler() : InputHandler(0x15) {}

    bool handle(InputContext& context) override {
        if (!context.line.empty()) {
            /* clear the entire line by moving back and overwriting with spaces */
            std::cout << std::string(context.cursor_pos, '\b');  /* move to start of input */
            std::cout << std::string(context.line.length(), ' ');  /* overwrite with spaces */
            std::cout << std::string(context.line.length(), '\b');  /* move back to start */
            /* clear the actual input string and reset cursor */
            context.line.clear();
            context.cursor_pos = 0;
        }
        return true;
    }
};

/* escape sequence handler implementation */
class EscapeSequenceHandler : public InputHandler {
public:
    EscapeSequenceHandler() : InputHandler(27) {}

    bool handle(InputContext& context) override {
        char next1 = getchar();
        if (next1 == '[') {
            char next2 = getchar();
            if (next2 == 'A') {  /* up arrow */
                if (!context.history.empty()) {
                    /* save current input if not already navigating */
                    if (context.history_index == -1) {
                        context.current_input = context.line;
                        context.history_index = context.history.size();
                    }
                    if (context.history_index > 0) {
                        context.history_index--;
                        /* clear current line */
                        std::cout << std::string(context.cursor_pos, '\b') << std::string(context.line.length(), ' ') << std::string(context.line.length(), '\b');
                        /* show historical command */
                        context.line = context.history[context.history_index];
                        context.cursor_pos = context.line.length();
                        std::cout << context.line << std::flush;
                    }
                }
            }
            else if (next2 == 'B') {  /* down arrow */
                if (!context.history.empty() && context.history_index < static_cast<int>(context.history.size()) - 1) {
                    context.history_index++;
                    /* clear current line */
                    std::cout << std::string(context.cursor_pos, '\b') << std::string(context.line.length(), ' ') << std::string(context.line.length(), '\b');
                    /* show historical command */
                    context.line = context.history[context.history_index];
                    context.cursor_pos = context.line.length();
                    std::cout << context.line << std::flush;
                } else if (context.history_index == static_cast<int>(context.history.size()) - 1) {
                    /* go back to current input */
                    context.history_index = -1;
                    /* clear current line */
                    std::cout << std::string(context.cursor_pos, '\b') << std::string(context.line.length(), ' ') << std::string(context.line.length(), '\b');
                    context.line = context.current_input;
                    context.cursor_pos = context.line.length();
                    std::cout << context.line << std::flush;
                }
            }
            else if (next2 == 'C') {  /* right arrow */
                if (context.cursor_pos < static_cast<int>(context.line.length())) {
                    std::cout << "\033[C" << std::flush;
                    context.cursor_pos++;
                }
            }
            else if (next2 == 'D') {  /* left arrow */
                if (context.cursor_pos > 0) {
                    std::cout << "\033[D" << std::flush;
                    context.cursor_pos--;
                }
            }
        }
        return true;
    }
};

/* ctrl+c handler implementation */
class CtrlCHandler : public InputHandler {
public:
    CtrlCHandler() : InputHandler(3) {}

    bool handle(InputContext& context) override {
        context.line.clear();
        context.cursor_pos = 0;
        context.history_index = -1;
        context.current_input.clear();
        return false;
    }
};

/* tab key handler implementation */
class TabKeyHandler : public InputHandler {
public:
    TabKeyHandler() : InputHandler(9) {}

    bool handle(InputContext& context) override {
        /* find the start of the current command (before cursor) */
        size_t cmd_start = context.line.rfind(' ', context.cursor_pos - 1);
        if (cmd_start == std::string::npos) {
            cmd_start = 0;
        } else {
            cmd_start += 1;
        }

        /* extract the command prefix */
        std::string prefix = context.line.substr(cmd_start, context.cursor_pos - cmd_start);

        /* find matching commands */
        std::vector<std::string> matches = findMatchingCommands(prefix);

        if (matches.size() == 1) {  /* single match, complete it */
            std::string completion = matches[0].substr(prefix.length());
            context.line.insert(context.cursor_pos, completion + " ");
            context.cursor_pos += completion.length() + 1;

            /* redraw the line from cursor position */
            std::cout << context.line.substr(context.cursor_pos - completion.length() - 1) << std::string(context.line.length() - context.cursor_pos, '\b') << std::flush;
        } else if (matches.size() > 1) {  /* multiple matches */
            /* find the longest common prefix */
            size_t max_len = 0;
            size_t min_len = std::numeric_limits<size_t>::max();
            for (const auto& match : matches) {
                min_len = std::min(min_len, match.length());
            }

            while (max_len < min_len) {
                bool same = true;
                char first = matches[0][max_len];
                for (const auto& match : matches) {
                    if (match[max_len] != first) {
                        same = false;
                        break;
                    }
                }
                if (!same) break;
                max_len++;
            }

            /* if there's a common prefix beyond what we already have */
            if (max_len > prefix.length()) {
                std::string common = matches[0].substr(prefix.length(), max_len - prefix.length());
                context.line.insert(context.cursor_pos, common);
                context.cursor_pos += common.length();

                /* redraw the common part */
                std::cout << common << context.line.substr(context.cursor_pos) << std::string(context.line.length() - context.cursor_pos, '\b') << std::flush;
            } else {
                /* no common prefix, display all matches */
                std::cout << "\n";
                for (const auto& match : matches) {
                    std::cout << "  " << match << std::endl;
                }
                std::cout << "> " << context.line << std::flush;
            }
        }
        return true;
    }
};

/* printable character handler implementation */
class PrintableCharHandler : public InputHandler {
public:
    PrintableCharHandler() : InputHandler(0) {}

    bool canHandle(char c) const override {
        return isprint(static_cast<unsigned char>(c));
    }

    bool handle(InputContext& context) override {
        context.line.insert(context.cursor_pos, 1, context.c);
        context.cursor_pos++;
        /* display the added character and the rest of the line */
        std::cout << context.c << context.line.substr(context.cursor_pos) << std::string(context.line.length() - context.cursor_pos, '\b') << std::flush;
        return true;
    }
};

/* input handler factory implementation */
void InputHandlerFactory::initialize() {
    /* register all handlers */
    registerHandler(std::make_shared<EnterKeyHandler>());
    registerHandler(std::make_shared<BackspaceKeyHandler>());
    registerHandler(std::make_shared<MacOSDeleteKeyHandler>());
    registerHandler(std::make_shared<EscapeSequenceHandler>());
    registerHandler(std::make_shared<CtrlCHandler>());
    registerHandler(std::make_shared<TabKeyHandler>());
    registerHandler(std::make_shared<PrintableCharHandler>());
}

/* all handler implementations are now inline in the class definitions above */
