#ifndef INPUT_HANDLER_HPP
#define INPUT_HANDLER_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>

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
     * @param line the current input line
     * @param cursor_pos the current cursor position
     * @param history the command history
     * @param history_index the current history index
     * @param current_input the current input buffer
     * @return whether the loop should continue (false to break)
     */
    virtual bool handle(char c, std::string& line, int& cursor_pos, std::vector<std::string>& history, int& history_index, std::string& current_input) = 0;

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