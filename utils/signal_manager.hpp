#ifndef SIGNAL_MANAGER_HPP
#define SIGNAL_MANAGER_HPP

#include <functional>
#include <csignal>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <memory>

/**
 * signalManager class
 * provides improved RAII-based signal handling with nested callback support
 * supports calling all registered handlers in proper order when signal occurs
 * thread-safe implementation with singleton pattern
 */
class SignalManager {
private:
    /* type definitions */
    using SignalHandler = void (*)(int);
    using SignalCallback = std::function<void(int)>;

    /* signal handler stack entry structure */
    struct HandlerStackEntry {
        SignalHandler previous_handler;  /* the handler before this one was installed */
        SignalCallback callback;         /* callback function for this handler */
    };

    /* per-signal handler stack */
    std::unordered_map<int, std::vector<HandlerStackEntry>> handler_stacks_;

    /* map to store original signal handlers (system defaults) */
    std::unordered_map<int, SignalHandler> original_handlers_;

    /* flag to check if manager is active */
    bool active_;

    /* Static singleton instance */
    static std::unique_ptr<SignalManager> instance_;
    static std::mutex instance_mutex_;

    /* mutex for thread-safe operations on handler stacks */
    mutable std::mutex handlers_mutex_;

    /* static signal handler function */
    static void staticSignalHandler(int signal);

    /* private constructor for singleton pattern */
    SignalManager();

    /* get the current handler for a signal */
    SignalHandler getCurrentSignalHandler(int signal);

    /* get singleton instance */
    static SignalManager* getInstance();

    /* internal method to register signal handler */
    bool registerSignalHandlerInternal(int signal, SignalCallback callback);

    /* internal method to unregister signal handler */
    bool unregisterSignalHandlerInternal(int signal);

public:
    ~SignalManager();

    SignalManager(const SignalManager&) = delete;
    SignalManager& operator=(const SignalManager&) = delete;
    SignalManager(SignalManager&&) = delete;
    SignalManager& operator=(SignalManager&&) = delete;

    /**
     * create global signal manager instance
     * should be called once at program startup
     */
    static void initialize();

    /**
     * destroy global signal manager instance
     * should be called once at program shutdown
     */
    static void shutdown();

    /**
     * register signal handler for specific signal
     * @param signal signal number to handle
     * @param callback function to call when signal is received
     * @return true if registration successful, false otherwise
     */
    static bool registerSignalHandler(int signal, SignalCallback callback);

    /**
     * unregister signal handler for specific signal
     * @param signal signal number to unregister
     * @return true if unregistration successful, false otherwise
     */
    static bool unregisterSignalHandler(int signal);

    /**
     * check if manager is active
     * @return true if active, false otherwise
     */
    static bool isActive();

    /**
     * SignalGuard class for temporary signal handling
     * automatically registers and unregisters signal handlers
     */
    class SignalGuard {
    private:
        int signal_;
        bool registered_;

    public:
        /**
         * constructor that registers a signal handler
         * @param signal signal number to handle
         * @param callback function to call when signal is received
         */
        SignalGuard(int signal, SignalCallback callback);

        ~SignalGuard();

        /* disable copying */
        SignalGuard(const SignalGuard&) = delete;
        SignalGuard& operator=(const SignalGuard&) = delete;

        /* allow moving */
        SignalGuard(SignalGuard&& other) noexcept;
        SignalGuard& operator=(SignalGuard&& other) noexcept;
    };
};

#endif /* SIGNAL_MANAGER_HPP */
