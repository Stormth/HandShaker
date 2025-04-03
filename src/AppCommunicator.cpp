//
// Created by Storm on 2025/4/1.
//
// AppCommunicator.cpp
// AppCommunicator.cpp
#include "AppCommunicator.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <cctype>
#include <queue>

AppCommunicator::AppCommunicator(TCPConnection* conn) {
    this->connection = conn;
}

void AppCommunicator::sendMessage(const std::string& sentence) {
    auto words = splitToWords(sentence);
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
        std::queue<std::string> display_buffer;
        while (!connection->isClosed()) {
            std::string word = connection->popWord();  // 你需要实现 popWord()
            if (!word.empty()) {
                display_buffer.push(word);
            }

            while (!display_buffer.empty()) {
                std::string current = display_buffer.front(); display_buffer.pop();
                std::cout << current << std::flush;

                if (!display_buffer.empty()) {
                    std::string next = display_buffer.front();
                    bool curr_is_word = std::isalnum((unsigned char)current[0]);
                    bool next_is_word = std::isalnum((unsigned char)next[0]);

                    if (curr_is_word && next_is_word) {
                        std::cout << " " << std::flush;
                    }
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }).detach();
}

std::vector<std::string> AppCommunicator::splitToWords(const std::string& input) {
    std::vector<std::string> result;
    std::string temp;

    for (size_t i = 0; i < input.size();) {
        unsigned char c = input[i];

        if (std::ispunct(c)) {
            result.push_back(std::string(1, c));
            ++i;
        }
        else if (std::isalnum(c)) {
            temp.clear();
            while (i < input.size() && std::isalnum((unsigned char)input[i])) {
                temp += input[i];
                ++i;
            }
            result.push_back(temp);
        }
        else {
            ++i;
        }
    }

    return result;
}
