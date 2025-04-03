//
// Created by Storm on 2025/4/1.
//
#include "../include/AppCommunicator.h"
#include <thread>
#include <sstream>
#include <iostream>
#include <chrono>

AppCommunicator::AppCommunicator(TCPConnection* conn) {
    this->connection = conn;
}

std::vector<std::string> AppCommunicator::splitToWords(const std::string& input) {
    std::vector<std::string> words;
    std::string word;
    std::istringstream iss(input);

    while (iss >> word) {
        words.push_back(word);
        //words.push_back(" "); // 保留空格为独立单元
    }

    if (!words.empty()) words.pop_back(); // 去掉最后一个多余空格
    return words;
}

void AppCommunicator::sendMessage(const std::string& message) {
    auto words = splitToWords(message);
    for (const auto& word : words) {
        TCPPacket pkt = connection->createDataPacket(word);
        connection->sendPacket(pkt);
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 模拟间隔
    }
}

void AppCommunicator::startReceiving() {
    std::thread recv_thread([this]() {
        connection->receivePacketLoop();
    });
    recv_thread.detach();
}

void AppCommunicator::startPrinting() {
    std::thread print_thread([this]() {
        while (true) {
            connection->printFromBuffer();
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    });
    print_thread.detach();
}
