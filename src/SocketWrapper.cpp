//
// Created by Storm on 2025/4/1.
//
// SocketWrapper.cpp
#include "../include/SocketWrapper.h"
#include <iostream>
#include <cstring>

SocketWrapper::SocketWrapper() {
#ifdef _WIN32
    WSADATA wsaData;
    int err = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (err != 0) {
        std::cerr << "[Socket] WSAStartup failed: " << err << std::endl;
    }
#endif
}

SocketWrapper::~SocketWrapper() {
    close();
#ifdef _WIN32
    WSACleanup();
#endif
}

bool SocketWrapper::connectTo(const std::string& ip, int port) {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) return false;

    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
#ifdef _WIN32
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
#else
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);
#endif

    return connect(sock, (sockaddr*)&addr, sizeof(addr)) != SOCKET_ERROR;
}

bool SocketWrapper::bindAndListen(int port) {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) return false;

    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (::bind(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) return false;
    return listen(sock, 5) != SOCKET_ERROR;
}

SocketWrapper SocketWrapper::acceptConnection() {
    SOCKET client_sock = accept(sock, nullptr, nullptr);
    SocketWrapper client;
    client.sock = client_sock;
    return client;
}

bool SocketWrapper::sendRaw(const std::string& data) {
    int total_sent = 0;
    while (total_sent < data.size()) {
        int sent = send(sock, data.data() + total_sent, data.size() - total_sent, 0);
        if (sent <= 0) return false;
        total_sent += sent;
    }
    return true;
}

std::string SocketWrapper::receiveRaw() {
    char buffer[1024];
    int received = recv(sock, buffer, sizeof(buffer), 0);
    if (received <= 0) return "";
    return std::string(buffer, received);
}

void SocketWrapper::close() {
    if (sock != INVALID_SOCKET) {
#ifdef _WIN32
        closesocket(sock);
#else
        ::close(sock);
#endif
        sock = INVALID_SOCKET;
    }
}
