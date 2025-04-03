//
// Created by Storm on 2025/4/1.
//
// TCPConnection.h
#pragma once

#include "TCPPacket.h"
#include "TCPBuffer.h"
#include "SocketWrapper.h"
#include <map>
#include <string>

class TCPConnection {
public:
    TCPConnection(SocketWrapper* socket, const std::string& role = "TCP");

    // 连接管理
    bool initiateHandshake();
    bool receiveHandshake();
    void initiateClose();           // 主动关闭连接
    void handleClose();             // 被动处理关闭

    // 报文处理
    TCPPacket createDataPacket(const std::string& word);
    bool sendPacket(const TCPPacket& pkt);
    void receivePacketLoop();
    void processReceivedPacket(const TCPPacket& pkt);
    void handleACK(uint32_t ack_num);

    // 缓冲区输出
    void printFromBuffer();
    std::string popWord();
    // 状态查询
    bool isClosed() const { return connection_closed; }

private:
    SocketWrapper* sock;
    uint32_t seq_num = 0;
    uint32_t ack_num = 0;
    bool connection_closed = false;
    std::string role_label = "TCP";

    TCPBuffer recv_buffer;
    std::map<uint32_t, TCPPacket> send_window;
};