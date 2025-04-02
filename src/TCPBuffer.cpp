//
// Created by Storm on 2025/4/1.
//
// TCPBuffer.cpp
#include "../include/TCPBuffer.h"

TCPBuffer::TCPBuffer(size_t max_size) {
    this->max_size = max_size;
}

bool TCPBuffer::push(uint32_t seq, const std::string& word) {
    if (buffer.size() >= max_size) return false;
    buffer[seq] = word;
    return true;
}

std::string TCPBuffer::pop() {
    if (buffer.empty()) return "";
    auto it = buffer.begin();
    std::string word = it->second;
    buffer.erase(it);
    return word;
}

bool TCPBuffer::isFull() const {
    return buffer.size() >= max_size;
}

bool TCPBuffer::isEmpty() const {
    return buffer.empty();
}

void TCPBuffer::clearUpTo(uint32_t ack_num) {
    auto it = buffer.begin();
    while (it != buffer.end() && it->first < ack_num) {
        it = buffer.erase(it);
    }
}
