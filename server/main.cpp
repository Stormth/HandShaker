//
// Created by Storm on 2025/4/1.
//
// server/main.cpp
// server/main.cpp
// server/main.cpp
// 修改自原始代码，使用 Logger 输出日志
#include "../include/SocketWrapper.h"
#include "../include/TCPConnection.h"
#include "../include/AppCommunicator.h"
#include "Logger.h"
#include <iostream>
#include <string>

int main() {
    Logger() << "[Server] Starting TCP server...";

    SocketWrapper server_socket;
    if (!server_socket.bindAndListen(8888)) {
        std::cerr << "[Server] Failed to bind and listen on port 8888." << std::endl;
        return -1;
    }

    Logger() << "[Server] Waiting for client to connect...";
    SocketWrapper client_socket = server_socket.acceptConnection();
    Logger() << "[Server] Client connected.";

    TCPConnection conn(&client_socket, "Server");
    if (!conn.receiveHandshake()) {
        std::cerr << "[Server] Handshake failed." << std::endl;
        return -1;
    }
    Logger() << "[Server] Connection established.";

    AppCommunicator app(&conn);
    app.startReceiving();
    app.startPrinting();

    std::string input;
    while (true) {
        std::cout << "\nServer>> " << std::flush;
        std::getline(std::cin, input);

        if (input == "exit") {
            conn.initiateClose();
            conn.handleClose();
            break;
        } else if (input == "/debug") {
            conn.debugPrintRecvBuffer();
            continue;
        } else if (input == "/log on") {
            conn.setLogEnabled(true);
            Logger() << "[Server] Log enabled.";
            continue;
        } else if (input == "/log off") {
            conn.setLogEnabled(false);
            Logger() << "[Server] Log disabled.";
            continue;
        }

        app.sendMessage(input);
    }

    Logger() << "[Server] Connection closed.";
    return 0;
}

