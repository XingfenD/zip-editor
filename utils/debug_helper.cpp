#include "debug_helper.hpp"
#include <cstdarg>
#include <cstring>
#include <iostream>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>  /* include addrinfo struct */
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET SocketType;
    #define SOCKET_ERROR_CODE WSAGetLastError()
    #define CLOSE_SOCKET(s) closesocket(s)
    #define INIT_SOCKET() WSADATA wsaData; WSAStartup(MAKEWORD(2, 2), &wsaData)
    #define CLEANUP_SOCKET() WSACleanup()
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <fcntl.h>
    typedef int SocketType;
    #define SOCKET_ERROR_CODE errno
    #define CLOSE_SOCKET(s) close(s)
    #define INIT_SOCKET()
    #define CLEANUP_SOCKET()
    #define INVALID_SOCKET -1
#endif

/* init debug helper */
std::unique_ptr<DebugHelper> DebugHelper::instance = nullptr;
std::mutex DebugHelper::mutex;

DebugHelper::DebugHelper() : isInitialized(false), port(0), host("localhost"), sockfd(INVALID_SOCKET) {
    INIT_SOCKET();
}

/* check if debug helper is ready */
bool DebugHelper::isReady() const {
    return isInitialized;
}

DebugHelper::~DebugHelper() {
    /* close socket and cleanup socket */
    if (sockfd != INVALID_SOCKET) {
        CLOSE_SOCKET(sockfd);
        sockfd = INVALID_SOCKET;
    }
    isInitialized = false;
    CLEANUP_SOCKET();
}

/* get debug helper instance */
DebugHelper& DebugHelper::getInstance() {
    std::lock_guard<std::mutex> lock(mutex);
    if (!instance) {
        instance.reset(new DebugHelper());
    }
    return *instance;
}

bool DebugHelper::initNetwork() {
    return true; /* basic init, connect in sendData */
}

/* initialize debug helper */
bool DebugHelper::initialize(const std::string& host, int port) {
    std::lock_guard<std::mutex> lock(mutex);

    if (port <= 0 || port > 65535) {
        std::cerr << "Invalid port number: " << port << std::endl;
        return false;
    }

    this->host = host;
    this->port = port;

    /* close existing socket */
    if (sockfd != INVALID_SOCKET) {
        CLOSE_SOCKET(sockfd);
        sockfd = INVALID_SOCKET;
    }

    /* try to connect to server */
    if (!connectToServer()) {
        std::cerr << "Failed to establish initial connection to " << host << ":" << port << std::endl;
        return false;
    }

    isInitialized = true;
    std::cout << "DebugHelper initialized with host: " << host << ", port: " << port << std::endl;
    return true;
}

/* connect to server */
bool DebugHelper::connectToServer() {
    if (sockfd != INVALID_SOCKET) {
        CLOSE_SOCKET(sockfd);
        sockfd = INVALID_SOCKET;
    }

    struct addrinfo hints, *res, *p;
    int status;
    char portStr[6];

    snprintf(portStr, sizeof(portStr), "%d", port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(host.c_str(), portStr, &hints, &res)) != 0) {
        std::cerr << "Failed to resolve host: " << host
                  << ", error: " << gai_strerror(status) << std::endl;
        return false;
    }

    bool connected = false;
    for (p = res; p != nullptr; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == INVALID_SOCKET) {
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == 0) {
            connected = true;
            break;
        }

        CLOSE_SOCKET(sockfd);
        sockfd = INVALID_SOCKET;
    }

    freeaddrinfo(res);

    if (!connected) {
        std::cerr << "Failed to connect to " << host << ":" << port << std::endl;
        return false;
    }

    return true;
}

bool DebugHelper::sendData(const std::string& data) {
    if (!isReady()) {
        std::cerr << "DebugHelper not initialized" << std::endl;
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex);

    /* check if socket is valid, if not, try to connect */
    if (sockfd == INVALID_SOCKET) {
        if (!connectToServer()) {
            return false;
        }
    }

    /* try to send data */
    size_t bytesSent = send(sockfd, data.c_str(), data.length(), 0);

    /* if send failed, try to reconnect and send again */
    if (bytesSent != data.length()) {
        std::cerr << "Failed to send complete data, trying to reconnect..." << std::endl;

        /* close current connection and try to reconnect */
        CLOSE_SOCKET(sockfd);
        sockfd = INVALID_SOCKET;

        if (!connectToServer()) {
            return false;
        }

        /* try to send data again */
        bytesSent = send(sockfd, data.c_str(), data.length(), 0);

        if (bytesSent != data.length()) {
            std::cerr << "Failed to send complete data after reconnection. Sent: " << bytesSent
                      << " Expected: " << data.length() << std::endl;
            return false;
        }
    }

    return true;
}

bool DebugHelper::sendFormattedData(const char* format, ...) {
    if (!isReady()) {
        std::cerr << "DebugHelper not initialized" << std::endl;
        return false;
    }

    /* calculate required buffer size */
    va_list args;
    va_start(args, format);
    int size = vsnprintf(nullptr, 0, format, args);
    va_end(args);

    if (size <= 0) {
        return false;
    }

    /* allocate enough buffer size */
    std::string buffer(size + 1, '\0');

    /* fill formatted string */
    va_start(args, format);
    vsnprintf(&buffer[0], buffer.size(), format, args);
    va_end(args);

    /* send formatted data */
    return sendData(buffer);
}

void DebugHelper::shutdown() {
    /* use try-catch block to ensure lock operation won't throw exception */
    try {
        std::lock_guard<std::mutex> lock(mutex);
        /* close socket */
        if (sockfd != INVALID_SOCKET) {
            CLOSE_SOCKET(sockfd);
            sockfd = INVALID_SOCKET;
        }
        isInitialized = false;
        std::cout << "DebugHelper shutdown" << std::endl;
    } catch (const std::exception& e) {
        /* if lock operation failed, at least set flag and close socket */
        if (sockfd != INVALID_SOCKET) {
            CLOSE_SOCKET(sockfd);
            sockfd = INVALID_SOCKET;
        }
        isInitialized = false;
        std::cerr << "Error in DebugHelper::shutdown: " << e.what() << std::endl;
    }
}
