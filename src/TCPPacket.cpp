//
// Created by Storm on 2025/4/1.
//
// TCPPacket.cpp
// TCPPacket.cpp
#include "../include/TCPPacket.h"
#include <cstring>

// 格式: [seq(4)][ack(4)][flags(1)][payload_len(4)][payload(N)]
std::string TCPPacket::serialize() const {
    std::string buffer;

    buffer.append(reinterpret_cast<const char*>(&seq_num), sizeof(seq_num));
    buffer.append(reinterpret_cast<const char*>(&ack_num), sizeof(ack_num));

    uint8_t flags = 0;
    if (SYN) flags |= 0b001;
    if (ACK) flags |= 0b010;
    if (FIN) flags |= 0b100;
    buffer.append(reinterpret_cast<const char*>(&flags), sizeof(flags));

    uint32_t len = static_cast<uint32_t>(payload.size());
    buffer.append(reinterpret_cast<const char*>(&len), sizeof(len));
    buffer.append(payload);

    return buffer;
}

TCPPacket TCPPacket::deserialize(const std::string& raw) {
    TCPPacket pkt;
    size_t offset = 0;

    std::memcpy(&pkt.seq_num, raw.data() + offset, sizeof(pkt.seq_num));
    offset += sizeof(pkt.seq_num);

    std::memcpy(&pkt.ack_num, raw.data() + offset, sizeof(pkt.ack_num));
    offset += sizeof(pkt.ack_num);

    uint8_t flags = 0;
    std::memcpy(&flags, raw.data() + offset, sizeof(flags));
    pkt.SYN = flags & 0b001;
    pkt.ACK = flags & 0b010;
    pkt.FIN = flags & 0b100;
    offset += sizeof(flags);

    uint32_t len = 0;
    std::memcpy(&len, raw.data() + offset, sizeof(len));
    offset += sizeof(len);

    pkt.payload = raw.substr(offset, len);
    return pkt;
}
