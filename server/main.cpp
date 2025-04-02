//
// Created by Storm on 2025/4/1.
//
// server/main.cpp
// server/main.cpp
#include "../include/SocketWrapper.h"
#include "../include/TCPConnection.h"
#include "../include/AppCommunicator.h"
#include <iostream>
#include <string>

int main() {
    std::cout << "[Server] Starting TCP server..." << std::endl;

    SocketWrapper server_socket;
    if (!server_socket.bindAndListen(8888)) {
        std::cerr << "[Server] Failed to bind and listen on port 8888." << std::endl;
        return -1;
    }

    std::cout << "[Server] Waiting for client to connect..." << std::endl;
    SocketWrapper client_socket = server_socket.acceptConnection();
    std::cout << "[Server] Client connected." << std::endl;

    TCPConnection conn(&client_socket, "Server");
    if (!conn.receiveHandshake()) {
        std::cerr << "[Server] Handshake failed." << std::endl;
        return -1;
    }
    std::cout << "[Server] Connection established." << std::endl;

    AppCommunicator app(&conn);
    app.startReceiving();
    app.startPrinting();

    std::string input;
    while (true) {
        std::getline(std::cin, input);

        if (input == "exit") {
            conn.initiateClose();
            conn.handleClose();
            break;
        }

        app.sendMessage(input);
    }

    std::cout << "[Server] Connection closed." << std::endl;
    return 0;
}
