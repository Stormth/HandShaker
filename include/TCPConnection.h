//
// Created by Storm on 2025/4/1.
//
// TCPConnection.h
#pragma once
#include "SocketWrapper.h"
#include "TCPPacket.h"
#include "TCPBuffer.h"
#include <map>
#include <string>
#include <vector>

class TCPConnection {
public:
    TCPConnection(SocketWrapper* socket, const std::string& role);

    bool initiateHandshake();
    bool receiveHandshake();

    TCPPacket createDataPacket(const std::string& word);
    bool sendPacket(const TCPPacket& pkt);
    void receivePacketLoop();
    void processReceivedPacket(const TCPPacket& pkt);

    void handleACK(uint32_t ack);
    void initiateClose();
    void handleClose();

    void printFromBuffer();
    std::string popWord();
    void debugPrintRecvBuffer() const;

    bool isClosed() const { return connection_closed; }
    std::string getRoleLabel() const { return role_label; }
    void setLogEnabled(bool enabled) { log_enabled = enabled; }

private:
    SocketWrapper* sock;
    TCPBuffer recv_buffer;
    std::map<uint32_t, TCPPacket> send_window;

    uint32_t seq_num;
    uint32_t ack_num;
    std::string role_label;
    bool connection_closed = false;
    bool log_enabled = false;
};
