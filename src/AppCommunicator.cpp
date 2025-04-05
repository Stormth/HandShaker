//
// Created by Storm on 2025/4/1.
//
// AppCommunicator.cpp
// AppCommunicator.cpp
// AppCommunicator.cpp
// AppCommunicator.cpp
// AppCommunicator.cpp
//
// 重构后的 AppCommunicator.cpp
//
#include "AppCommunicator.h"
#include <iostream>
#include <thread>
#include <mutex>

// 构造函数保持不变
AppCommunicator::AppCommunicator(TCPConnection* conn) {
    this->connection = conn;
}

// 将输入按字符拆分
std::vector<std::string> AppCommunicator::splitToChars(const std::string& input) {
    std::vector<std::string> chars;
    for (char c : input) {
        // 每个字符作为独立字符串
        chars.push_back(std::string(1, c));
    }
    return chars;
}

void AppCommunicator::sendMessage(const std::string& message) {
    auto chars = splitToChars(message);
    for (const auto& ch : chars) {
        TCPPacket pkt = connection->createDataPacket(ch);
        connection->sendPacket(pkt);
    }
}

void AppCommunicator::startReceiving() {
    std::thread([this]() {
        connection->receivePacketLoop();
    }).detach();
}

void AppCommunicator::startPrinting() {
    std::thread([this]() {
        while (!connection->isClosed()) {
            std::string ch = connection->popWord();
            if (!ch.empty()) {
                std::cout << "[receive message from "
                          << (connection->getRoleLabel() == "Client" ? "Server" : "Client")
                          << "]:" << ch;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }).detach();
}
