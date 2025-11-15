#include "debug_helper.hpp"
#include <iostream>
#include <thread>
#include <chrono>

/**
 * example of using DebugHelper
 *
 * This example shows how to use DebugHelper:
 * 1. initialize DebugHelper
 * 2. send simple debug messages
 * 3. send formatted debug messages
 * 4. use debug features in actual scenarios
 */

void runDebugDemo() {
    /* example 1: initialize DebugHelper */
    /* example 2: initialize DebugHelper with default port 9000 */
    if (!DebugHelper::getInstance().initialize("localhost", 9000)) {
        std::cerr << "Failed to initialize DebugHelper!" << std::endl;
        return;
    }

    std::cout << "DebugHelper initialized successfully. Sending test messages..." << std::endl;
    std::cout << "Make sure the Python debug server is running: python3 debug_server.py" << std::endl;

    /* example 1: send simple debug messages */
    DEBUG_LOG("This is a simple debug message");
    std::this_thread::sleep_for(std::chrono::seconds(1));

    /* example 2: send formatted debug messages */
    int version = 1;
    double pi = 3.14159;
    std::string status = "active";
    DEBUG_LOG_FMT("DebugHelper version: %d, PI value: %.2f, Status: %s",
                  version, pi, status.c_str());
    std::this_thread::sleep_for(std::chrono::seconds(1));

    /* example 3: simulate sending debug messages in a loop */
    std::cout << "Sending 5 test messages in a loop..." << std::endl;
    for (int i = 0; i < 5; i++) {
        DEBUG_LOG_FMT("Loop iteration #%d/%d - timestamp: %ld",
                      i+1, 5,
                      std::chrono::system_clock::now().time_since_epoch().count() / 1000000);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    /* example 4: simulate sending debug messages in an exception scenario */
    try {
        DEBUG_LOG("Starting critical operation...");
        throw std::runtime_error("Simulated error");
    } catch (const std::exception& e) {
        DEBUG_LOG_FMT("Exception caught: %s", e.what());
    }

    /* example 5: close debug connection */
    DEBUG_LOG("Debug session completed. Closing connection.");
    DebugHelper::getInstance().shutdown();

    std::cout << "Debug demo completed. Check the Python server output." << std::endl;
}
