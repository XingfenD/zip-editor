#ifndef DEBUG_HELPER_HPP
#define DEBUG_HELPER_HPP

#include <iostream>
#include <string>
#include <memory>
#include <mutex>

/* cross-platform socket type definition */
#ifdef _WIN32
    #include <winsock2.h>
    typedef SOCKET SocketType;
#else
    #include <sys/socket.h>
    typedef int SocketType;
#endif

/**
 * @brief debug helper class for initializing network connection and sending debug messages to specified port
 */
class DebugHelper {
private:
    static std::unique_ptr<DebugHelper> instance;
    static std::mutex mutex;

    bool isInitialized;
    int port;
    std::string host;
    SocketType sockfd; /* persistent socket connection */

    /* private constructor (singleton pattern) */
    DebugHelper();

    /* initialize network connection */
    bool initNetwork();

public:
    /* destructor */
    ~DebugHelper();

    /* get singleton instance */
    static DebugHelper& getInstance();

    /**
     * @brief initialize debug helper with specified host and port
     * @param host target host address
     * @param port target port number
     * @return whether initialization is successful
     */
    bool initialize(const std::string& host, int port);

    /**
     * @brief send raw data to debug server
     * @param data raw debug message
     * @return whether data is sent successfully
     */
    bool sendData(const std::string& data);

    /**
     * @brief send formatted data to debug server
     * @param format formatted debug message
     * @param ... variadic arguments
     * @return whether data is sent successfully
     */
    bool sendFormattedData(const char* format, ...);

    /**
     * @brief check whether debug helper is initialized
     * @return whether debug helper is initialized
     */
    bool isReady() const;

    /**
     * @brief shutdown debug helper and close socket connection
     */
    void shutdown();

private:
    /**
     * @brief connect to debug server
     * @return whether connection is successful
     */
    bool connectToServer();

    /**
     * @brief close debug socket connection
     */
    void closeSocket();
};


void runDebugDemo();

/**
 * @brief debug log macro
 * @param msg debug message
 */
#define DEBUG_LOG(msg) DebugHelper::getInstance().sendData(msg)

/**
 * @brief debug log macro with formatted message
 * @param format formatted debug message
 * @param ... variadic arguments
 */
#define DEBUG_LOG_FMT(format, ...) DebugHelper::getInstance().sendFormattedData(format, __VA_ARGS__)

#endif /* DEBUG_HELPER_HPP */
