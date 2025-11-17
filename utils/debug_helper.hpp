#ifndef DEBUG_HELPER_HPP
#define DEBUG_HELPER_HPP

#include <string>

#ifdef REMOTE_DEBUG_ON
    #include <iostream>

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
     * @brief remote debug client class for initializing network connection and sending debug messages to specified port
     */
    class RemoteDebugClient {
    private:
        static std::unique_ptr<RemoteDebugClient> instance;
        static std::mutex mutex;

        bool isInitialized;
        int port;
        std::string host;
        SocketType sockfd; /* persistent socket connection */

        /* private constructor (singleton pattern) */
        RemoteDebugClient();

        /* initialize network connection */
        bool initNetwork();

    public:
        /* destructor */
        ~RemoteDebugClient();

        /* get singleton instance */
        static RemoteDebugClient& getInstance();

        /**
         * @brief initialize remote debug client with default host and port
         * @return whether initialization is successful
         */
        bool initialize();

        /**
         * @brief initialize remote debug client with specified host and port
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
         * @brief check whether remote debug client is initialized
         * @return whether remote debug client is initialized
         */
        bool isReady() const;

        /**
         * @brief shutdown remote debug client and close socket connection
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

    /**
     * @brief debug log macro
     * @param msg debug message
     */
    #define DEBUG_LOG(msg) RemoteDebugClient::getInstance().sendData(msg)

/**
 * @brief debug log macro with formatted message
 * @param format formatted debug message
 * @param ... variadic arguments
 */
#define DEBUG_LOG_FMT(format, ...) RemoteDebugClient::getInstance().sendFormattedData(format, __VA_ARGS__)

#else

/* empty implementation when REMOTE_DEBUG_ON is not defined */
#define DEBUG_LOG(msg) ((void)0)
#define DEBUG_LOG_FMT(format, ...) ((void)0)

/* empty implementation of RemoteDebugClient class when REMOTE_DEBUG_ON is not defined */
class RemoteDebugClient {
public:
    static RemoteDebugClient& getInstance() {
        static RemoteDebugClient instance;
        return instance;
    }

    bool initialize() { return false; }
    bool initialize(const std::string&, int) { return false; }
    bool sendData(const std::string&) { return false; }
    bool sendFormattedData(const char*, ...) { return false; }
    bool isReady() const { return false; }
    void shutdown() {}
};

#endif /* REMOTE_DEBUG_ON */

#endif /* DEBUG_HELPER_HPP */
