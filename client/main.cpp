//
// Created by Storm on 2025/4/1.
//
// client/main.cpp
#include "../include/SocketWrapper.h"
#include "../include/TCPConnection.h"
#include "../include/AppCommunicator.h"
#include <iostream>
#include <string>

int main() {
    SocketWrapper sock;
    if (!sock.connectTo("192.168.31.48", 8888)) {
        std::cerr << "[Client] Connection failed." << std::endl;
        return -1;
    }

    TCPConnection conn(&sock, "Client");
    if (!conn.initiateHandshake()) {
        std::cerr << "[Client] Handshake failed." << std::endl;
        return -1;
    }
    std::cout << "[Client] Connected to server." << std::endl;

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
        } else if (input == "/debug") {
            conn.debugPrintRecvBuffer();
            continue;
        }

        app.sendMessage(input);
    }

    return 0;
}
