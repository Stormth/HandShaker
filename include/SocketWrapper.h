//
// Created by Storm on 2025/4/1.
//
// SocketWrapper.h
#pragma once

#include <string>
#include <vector>

#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;
#else
#include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netinet/in.h>
    typedef int SOCKET;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
#endif

class SocketWrapper {
public:
    SocketWrapper();
    ~SocketWrapper();

    bool connectTo(const std::string& ip, int port);   // 客户端
    bool bindAndListen(int port);                      // 服务端
    SocketWrapper acceptConnection();                  // 服务端接受连接

    bool sendRaw(const std::string& data);             // 发送原始数据
    std::string receiveRaw();                          // 接收原始数据

    void close();

private:
    SOCKET sock = INVALID_SOCKET;
};

