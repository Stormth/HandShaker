//
// Created by Storm on 2025/4/1.
//
// client/main.cpp
// client/main.cpp
// 修改自原始代码，使用 Logger 输出日志
// client/main.cpp
#include "../include/SocketWrapper.h"
#include "../include/TCPConnection.h"
#include "../include/AppCommunicator.h"
#include <iostream>
#include <string>

int main() {
    SocketWrapper sock;
    if (!sock.connectTo("127.0.0.1", 8888)) {
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
        std::cout << "\nClient>> " << std::flush;
        std::getline(std::cin, input);

        // 当输入为 "exit" 时，不将其作为数据发送，而是直接启动断连流程
        if (input == "exit") {
            conn.initiateClose();
            conn.handleClose();
            break;
        } else if (input == "/debug") {
            conn.debugPrintRecvBuffer();
            continue;
        } else if (input == "/log on") {
            conn.setLogEnabled(true);
            std::cout << "[Client] Log enabled." << std::endl;
            continue;
        } else if (input == "/log off") {
            conn.setLogEnabled(false);
            std::cout << "[Client] Log disabled." << std::endl;
            continue;
        }

        app.sendMessage(input);
    }

    return 0;
}
