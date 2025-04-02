//
// Created by Storm on 2025/4/1.
//
// TCPPacket.h
// TCPPacket.h
#pragma once
#include <string>
#include <cstdint>

class TCPPacket {
public:
    uint32_t seq_num = 0;
    uint32_t ack_num = 0;
    bool SYN = false;
    bool ACK = false;
    bool FIN = false;
    std::string payload;  // 支持任意单词/符号/中文词组

    std::string serialize() const;
    static TCPPacket deserialize(const std::string& raw);
};