//
// Created by Storm on 2025/4/1.
//
// AppCommunicator.h
#pragma once
#include "TCPConnection.h"
#include <string>
#include <vector>
#include <queue>

class AppCommunicator {
public:
    AppCommunicator(TCPConnection* conn);

    void sendMessage(const std::string& message);
    void startReceiving();     // 启动接收线程
    void startPrinting();      // 启动打印线程

private:
    TCPConnection* connection;
    std::vector<std::string> splitToChars(const std::string& input);
};
