//
// Created by Storm on 2025/4/1.
//
// client/main.cpp
// client/main.cpp
// 修改自原始代码，使用 Logger 输出日志
#include "../include/SocketWrapper.h"
#include "../include/TCPConnection.h"
#include "../include/AppCommunicator.h"
#include "Logger.h"
#include <iostream>
#include <string>

int main() {
    SocketWrapper sock;
    if (!sock.connectTo("192.168.31.42", 8888)) {
        std::cerr << "[Client] Connection failed." << std::endl;
        return -1;
    }

    TCPConnection conn(&sock, "Client");
    if (!conn.initiateHandshake()) {
        std::cerr << "[Client] Handshake failed." << std::endl;
        return -1;
    }
    Logger() << "[Client] Connected to server.";

    AppCommunicator app(&conn);
    app.startReceiving();
    app.startPrinting();

    std::string input;
    while (true) {
        std::cout << "\nClient>> " << std::flush;
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
            Logger() << "[Client] Log enabled.";
            continue;
        } else if (input == "/log off") {
            conn.setLogEnabled(false);
            Logger() << "[Client] Log disabled.";
            continue;
        }

        app.sendMessage(input);
    }

    return 0;
}
