#include "signal_manager.hpp"
#include <iostream>
#include <utility>

/* initialize static members */
std::unique_ptr<SignalManager> SignalManager::instance_ = nullptr;
std::mutex SignalManager::instance_mutex_;

SignalManager::SignalManager() : active_(false) {
    active_ = true;
}

SignalManager::~SignalManager() {
    std::lock_guard<std::mutex> lock(handlers_mutex_);

    /* Restore all signals to their original handlers */
    for (const auto& pair : original_handlers_) {
        std::signal(pair.first, pair.second);
    }

    handler_stacks_.clear();
    original_handlers_.clear();
    active_ = false;
}

/* get singleton instance */
SignalManager* SignalManager::getInstance() {
    std::lock_guard<std::mutex> lock(instance_mutex_);

    if (!instance_) {
        instance_.reset(createInstance());
    }

    return instance_.get();
}

/* get current signal handler for a specific signal */
SignalManager::SignalHandler SignalManager::getCurrentSignalHandler(int signal) {
    /* save the current handler by temporarily installing a dummy handler */
    SignalHandler current_handler = std::signal(signal, SIG_DFL);
    /* restore the original handler */
    std::signal(signal, current_handler);
    return current_handler;
}


/* initialize the signal manager */
void SignalManager::initialize() {
    getInstance(); // Simply getting the instance will initialize it
}

/* shutdown the signal manager */
void SignalManager::shutdown() {
    std::lock_guard<std::mutex> lock(instance_mutex_);
    instance_.reset();
}

/* register signal handler for specific signal (static method) */
bool SignalManager::registerSignalHandler(int signal, SignalCallback callback) {
    SignalManager* instance = getInstance();
    if (!instance) {
        return false;
    }
    return instance->registerSignalHandlerInternal(signal, std::move(callback));
}

/* unregister signal handler for specific signal (static method) */
bool SignalManager::unregisterSignalHandler(int signal) {
    SignalManager* instance = getInstance();
    if (!instance) {
        return false;
    }
    return instance->unregisterSignalHandlerInternal(signal);
}

/* internal method to register signal handler */
bool SignalManager::registerSignalHandlerInternal(int signal, SignalCallback callback) {
    if (!active_) {
        return false;
    }

    std::lock_guard<std::mutex> lock(handlers_mutex_);

    /* get current handler for this signal */
    SignalHandler current_handler = getCurrentSignalHandler(signal);

    /* If this is the first time this signal is registered, save the original handler */
    if (original_handlers_.find(signal) == original_handlers_.end()) {
        original_handlers_[signal] = current_handler;
    }

    /* Push the new handler onto the stack */
    HandlerStackEntry entry;
    entry.previous_handler = current_handler;
    entry.callback = std::move(callback);
    handler_stacks_[signal].push_back(entry);

    /* Install our static signal handler */
    if (std::signal(signal, staticSignalHandler) == SIG_ERR) {
        /* Failed to set signal handler, remove the entry we just added */
        handler_stacks_[signal].pop_back();
        return false;
    }

    return true;
}

/* internal method to unregister signal handler */
bool SignalManager::unregisterSignalHandlerInternal(int signal) {
    if (!active_) {
        return false;
    }

    std::lock_guard<std::mutex> lock(handlers_mutex_);

    auto it = handler_stacks_.find(signal);
    if (it == handler_stacks_.end() || it->second.empty()) {
        return false;
    }

    // Pop the last handler from the stack
    HandlerStackEntry entry = it->second.back();
    it->second.pop_back();

    // If the stack is now empty, restore the original handler
    if (it->second.empty()) {
        handler_stacks_.erase(it);
        std::signal(signal, original_handlers_[signal]);
        original_handlers_.erase(signal);
    } else {
        // Otherwise, restore the previous handler in the stack
        std::signal(signal, it->second.back().previous_handler);
    }

    return true;
}


/* check if manager is active */
bool SignalManager::isActive() {
    SignalManager* instance = getInstance();
    return instance && instance->active_;
}

/* static signal handler function - calls all registered callbacks in reverse order */
void SignalManager::staticSignalHandler(int signal) {
    SignalManager* instance = getInstance();
    if (!instance || !instance->active_) {
        /* if no instance or not active, restore default and re-raise */
        std::signal(signal, SIG_DFL);
        std::raise(signal);
        return;
    }

    /* Make a copy of the handler stack to avoid modifying it during signal handling */
    std::vector<SignalCallback> callbacks;
    {
        std::lock_guard<std::mutex> lock(instance->handlers_mutex_);
        auto it = instance->handler_stacks_.find(signal);
        if (it != instance->handler_stacks_.end()) {
            /* collect callbacks in reverse order (most recent first) */
            for (auto it_stack = it->second.rbegin(); it_stack != it->second.rend(); ++it_stack) {
                callbacks.push_back(it_stack->callback);
            }
        }
    }

    /* call all callbacks */
    for (const auto& callback : callbacks) {
        try {
            callback(signal);
        } catch (...) {
            /* silently catch exceptions to avoid termination during signal handling */
            /* continue with next callback */
        }
    }

    /* Restore default behavior and re-raise the signal */
    std::signal(signal, SIG_DFL);
    std::raise(signal);
}

/* signal guard implementation */
SignalManager::SignalGuard::SignalGuard(int signal, SignalCallback callback)
    : signal_(signal), registered_(false) {
    registered_ = SignalManager::registerSignalHandler(signal, std::move(callback));
}

SignalManager::SignalGuard::~SignalGuard() {
    if (registered_) {
        SignalManager::unregisterSignalHandler(signal_);
    }
}

SignalManager::SignalGuard::SignalGuard(SignalGuard&& other) noexcept
    : signal_(other.signal_), registered_(other.registered_) {
    other.registered_ = false;
}

SignalManager::SignalGuard& SignalManager::SignalGuard::operator=(SignalGuard&& other) noexcept {
    if (this != &other) {
        if (registered_) {
            SignalManager::unregisterSignalHandler(signal_);
        }

        signal_ = other.signal_;
        registered_ = other.registered_;
        other.registered_ = false;
    }
    return *this;
}
