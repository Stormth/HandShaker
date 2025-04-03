//
// Created by Storm on 2025/4/1.
//
// AppCommunicator.cpp
// AppCommunicator.cpp
// AppCommunicator.cpp
// AppCommunicator.cpp
#include "AppCommunicator.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <cctype>

AppCommunicator::AppCommunicator(TCPConnection* conn) {
    this->connection = conn;
}

void AppCommunicator::sendMessage(const std::string& message) {
    auto words = splitToWords(message);
    for (const auto& word : words) {
        TCPPacket pkt = connection->createDataPacket(word);
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
            std::string word = connection->popWord();
            if (!word.empty()) {
                std::cout << "[receive message from "
                          << (connection->getRoleLabel() == "Client" ? "Server" : "Client")
                          << "]:" << word << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }).detach();
}

std::vector<std::string> AppCommunicator::splitToWords(const std::string& input) {
    std::vector<std::string> words;
    std::string token;
    for (size_t i = 0; i < input.size(); ++i) {
        unsigned char c = input[i];
        if (std::isspace(c)) {
            if (!token.empty()) {
                words.push_back(token);
                token.clear();
            }
        } else if (std::ispunct(c)) {
            if (!token.empty()) {
                words.push_back(token);
                token.clear();
            }
            words.emplace_back(1, c);
        } else {
            token += c;
        }
    }
    if (!token.empty()) {
        words.push_back(token);
    }
    return words;
}