#ifdef REMOTE_DEBUG_ON

#include "debug_helper.hpp"
#include <cstdarg>
#include <cstring>
#include <iostream>
#include <csignal>
#include <chrono>
#include <thread>

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

/* init remote debug client */
std::unique_ptr<RemoteDebugClient> RemoteDebugClient::instance = nullptr;
std::mutex RemoteDebugClient::mutex;

RemoteDebugClient::RemoteDebugClient() : isInitialized(false), port(0), host("localhost"), sockfd(INVALID_SOCKET) {
    INIT_SOCKET();
    /* ignore SIGPIPE signal, prevent program termination when writing to closed socket */
    signal(SIGPIPE, SIG_IGN);
}

/* check if remote debug client is ready */
bool RemoteDebugClient::isReady() const {
    return isInitialized;
}

RemoteDebugClient::~RemoteDebugClient() {
    /* close socket and cleanup socket */
    if (sockfd != INVALID_SOCKET) {
        CLOSE_SOCKET(sockfd);
        sockfd = INVALID_SOCKET;
    }
    isInitialized = false;
    CLEANUP_SOCKET();
}

/* get remote debug client instance */
RemoteDebugClient& RemoteDebugClient::getInstance() {
    std::lock_guard<std::mutex> lock(mutex);
    if (!instance) {
        instance.reset(new RemoteDebugClient());
    }
    return *instance;
}

bool RemoteDebugClient::initNetwork() {
    return true; /* basic init, connect in sendData */
}

bool RemoteDebugClient::initialize() {
    return initialize("localhost", 9000);
}

/* initialize remote debug client */
bool RemoteDebugClient::initialize(const std::string& host, int port) {
    std::lock_guard<std::mutex> lock(mutex);

    if (port <= 0 || port > 65535) {
        std::cerr << "Invalid port number: " << port << std::endl;
        return false;
    }

    if (isInitialized) {
        std::cerr << "RemoteDebugClient is already initialized" << std::endl;
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
    std::cout << "RemoteDebugClient initialized with host: " << host << ", port: " << port << std::endl;
    return true;
}

/* connect to server */
bool RemoteDebugClient::connectToServer() {
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
        return false;
    }

    bool connected = false;
    for (p = res; p != nullptr; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == INVALID_SOCKET) {
            continue;
        }

        /* set connection timeout to 2 seconds */
        #ifdef _WIN32
        DWORD timeout = 2000;
        setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
        #else
        struct timeval timeout;
        timeout.tv_sec = 2;
        timeout.tv_usec = 0;
        setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
        #endif

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == 0) {
            connected = true;
            break;
        }

        CLOSE_SOCKET(sockfd);
        sockfd = INVALID_SOCKET;
    }

    freeaddrinfo(res);

    if (!connected) {
        isInitialized = false;
        return false;
    }

    return true;
}

bool RemoteDebugClient::sendData(const std::string& data) {
    if (!isReady()) {
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex);

    /* set socket timeout, 2 seconds */
    int retries = 2;

    while (retries > 0) {
        /* check if socket is valid, if not, try to connect */
        if (sockfd == INVALID_SOCKET) {
            if (!connectToServer()) {
                retries--;
                if (retries > 0) {
                    /* sleep for 100ms */
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    continue;
                }
                return false;
            }
        }

        /* send data length prefix (4 bytes, big endian) */
        uint32_t dataLength = static_cast<uint32_t>(data.length());
        uint32_t networkOrderLength = 0;

        /* convert to network byte order (big endian) */
        #ifdef _WIN32
        networkOrderLength = htonl(dataLength);
        #else
        networkOrderLength = htonl(dataLength);
        #endif

        /* send length prefix */
        size_t lengthSent = send(sockfd, reinterpret_cast<const char*>(&networkOrderLength), sizeof(networkOrderLength), 0);
        if (lengthSent != sizeof(networkOrderLength)) {
            /* length send failed */
            CLOSE_SOCKET(sockfd);
            sockfd = INVALID_SOCKET;

            retries--;
            if (retries > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            continue;
        }

        /* send actual data */
        size_t bytesSent = send(sockfd, data.c_str(), data.length(), 0);

        /* check if send is successful */
        if (bytesSent == data.length()) {
            /* send successful */
            return true;
        } else {
            /* send failed, may be connection broken */
            // std::cerr << "send data failed, try to reconnect..." << std::endl;

            /* close current connection */
            CLOSE_SOCKET(sockfd);
            sockfd = INVALID_SOCKET;

            retries--;
            if (retries > 0) {
                /* sleep for 100ms */
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }

    /* all retries failed */
    // std::cerr << "send data failed after " << retries << " retries" << std::endl;
    return false;
}

bool RemoteDebugClient::sendFormattedData(const char* format, ...) {
    if (!isReady()) {
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

void RemoteDebugClient::shutdown() {
    /* use try-catch block to ensure lock operation won't throw exception */
    try {
        std::lock_guard<std::mutex> lock(mutex);
        /* close socket */
        if (sockfd != INVALID_SOCKET) {
            CLOSE_SOCKET(sockfd);
            sockfd = INVALID_SOCKET;
        }
        isInitialized = false;
        std::cout << "RemoteDebugClient shutdown" << std::endl;
    } catch (const std::exception& e) {
        /* if lock operation failed, at least set flag and close socket */
        if (sockfd != INVALID_SOCKET) {
            CLOSE_SOCKET(sockfd);
            sockfd = INVALID_SOCKET;
        }
        isInitialized = false;
        std::cerr << "Error in RemoteDebugClient::shutdown: " << e.what() << std::endl;
    }
}

#endif /* REMOTE_DEBUG_ON */
