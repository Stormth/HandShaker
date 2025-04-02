//
// Created by Storm on 2025/4/1.
//
// TCPBuffer.h
#pragma once
#include <map>
#include <string>

class TCPBuffer {
public:
    TCPBuffer(size_t max_size = 50);

    bool push(uint32_t seq, const std::string& word);  // 添加单词（按序号）
    std::string pop();                                 // 弹出最早的词
    bool isFull() const;
    bool isEmpty() const;
    void clearUpTo(uint32_t ack_num);                  // 删除所有 < ack_num

private:
    std::map<uint32_t, std::string> buffer; // 有序缓存（按序号）
    size_t max_size;
};

