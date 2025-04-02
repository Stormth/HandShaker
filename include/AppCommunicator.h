//
// Created by Storm on 2025/4/1.
//
// AppCommunicator.h
#pragma once
#include "TCPConnection.h"
#include <string>
#include <vector>

class AppCommunicator {
public:
    AppCommunicator(TCPConnection* conn);

    void sendMessage(const std::string& message);
    void startReceiving();     // 启动接收线程
    void startPrinting();      // 启动打印线程

private:
    TCPConnection* connection;

    std::vector<std::string> splitToWords(const std::string& input);
};

