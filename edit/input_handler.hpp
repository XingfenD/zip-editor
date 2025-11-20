#ifndef INPUT_HANDLER_HPP
#define INPUT_HANDLER_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>

/* input context structure to encapsulate all input-related parameters */
struct InputContext {
    char c;                             /* current input character */
    std::string& line;                  /* current input line */
    int& cursor_pos;                    /* current cursor position */
    std::vector<std::string>& history;  /* command history */
    int& history_index;                 /* current history index */
    std::string& current_input;         /* current input buffer */

    /* constructor to initialize all references */
    InputContext(
        char c_,
        std::string& line_,
        int& cursor_pos_,
        std::vector<std::string>& history_,
        int& history_index_,
        std::string& current_input_
    ) : c(c_),
        line(line_),
        cursor_pos(cursor_pos_),
        history(history_),
        history_index(history_index_),
        current_input(current_input_) {}
};

/* find matching commands based on prefix for tab completion */
std::vector<std::string> findMatchingCommands(const std::string& prefix);

/* input handler interface for the command pattern */
class InputHandler {
public:
    InputHandler(char trigger_char) : trigger_char(trigger_char) {}
    virtual ~InputHandler() = default;

    /**
     * handle the input character
     * @param c the input character
     * @param context the input context containing all necessary state
     * @return whether the loop should continue (false to break)
     */
    virtual bool handle(InputContext& context) = 0;

    /**
     * check if this handler can handle the given character
     * @param c the character to check
     * @return true if this handler can handle the character
     */
    virtual bool canHandle(char c) const {
        return c == trigger_char;
    }

    /**
     * get the trigger character for this handler
     * @return the trigger character
     */
    char getTriggerChar() const { return trigger_char; }

private:
    char trigger_char;
};

/* input handler factory class to create and manage input handlers */
class InputHandlerFactory {
private:
    static std::map<char, std::shared_ptr<InputHandler>> handlers;

public:
    /* initialize all available input handlers */
    static void initialize();

    /**
     * get handler for the given character
     * @param c the input character
     * @return handler pointer if found, nullptr otherwise
     */
    static std::shared_ptr<InputHandler> getHandler(char c);

    /**
     * register a handler
     * @param handler handler to register
     */
    static void registerHandler(std::shared_ptr<InputHandler> handler);
};

/* specific handler classes are implemented in input_handler.cpp */

#endif /* INPUT_HANDLER_HPP */
