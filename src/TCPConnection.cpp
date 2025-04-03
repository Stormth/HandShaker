//
// Created by Storm on 2025/4/1.
//
// TCPConnection.cpp
// TCPConnection.cpp
// TCPConnection.cpp
// TCPConnection.cpp
#include "TCPConnection.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>

bool TCPConnection::initiateHandshake() {
    TCPPacket syn;
    syn.SYN = true;
    syn.seq_num = seq_num;
    syn.payload = "";
    sendPacket(syn);

    std::string raw = sock->receiveRaw();
    if (raw.empty()) return false;

    TCPPacket response = TCPPacket::deserialize(raw);
    if (response.SYN && response.ACK) {
        ack_num = response.seq_num + 1;
        TCPPacket ack_pkt;
        ack_pkt.seq_num = ++seq_num;
        ack_pkt.ack_num = ack_num;
        ack_pkt.ACK = true;
        ack_pkt.payload = "";
        sendPacket(ack_pkt);
        std::cout << "[" << role_label << "] Handshake completed (client side)." << std::endl;
        return true;
    }

    return false;
}

TCPConnection::TCPConnection(SocketWrapper* socket, const std::string& role) {
    this->sock = socket;
    this->seq_num = 0;
    this->ack_num = 0;
    this->role_label = role;
}

TCPPacket TCPConnection::createDataPacket(const std::string& word) {
    TCPPacket pkt;
    pkt.seq_num = seq_num;
    pkt.ack_num = ack_num;
    pkt.SYN = false;
    pkt.ACK = false;
    pkt.FIN = false;
    pkt.payload = word;
    return pkt;
}

bool TCPConnection::sendPacket(const TCPPacket& pkt) {
    std::string data = pkt.serialize();
    bool success = sock->sendRaw(data);
    if (success && !pkt.payload.empty()) {
        send_window[pkt.seq_num] = pkt;
        seq_num++;
    }
    return success;
}

void TCPConnection::receivePacketLoop() {
    while (!connection_closed) {
        std::string raw = sock->receiveRaw();
        if (raw.empty()) continue;
        TCPPacket pkt = TCPPacket::deserialize(raw);
        processReceivedPacket(pkt);
    }
}

void TCPConnection::processReceivedPacket(const TCPPacket& pkt) {
    if (pkt.ACK) {
        handleACK(pkt.ack_num);
        return;
    }

    if (pkt.FIN) {
        std::cout << "[" << role_label << "] Received FIN from remote." << std::endl;
        std::cout << "[" << role_label << "] Do you still have something to send? (y/n): ";
        std::string choice;
        std::getline(std::cin, choice);

        if (choice == "y") {
            std::cout << "[" << role_label << "] Please finish sending before closing." << std::endl;
            return;
        }

        TCPPacket ack_pkt;
        ack_pkt.seq_num = 0;
        ack_pkt.ack_num = pkt.seq_num + 1;
        ack_pkt.ACK = true;
        ack_pkt.payload = "";
        sendPacket(ack_pkt);

        TCPPacket fin_pkt;
        fin_pkt.seq_num = seq_num;
        fin_pkt.ack_num = pkt.seq_num + 1;
        fin_pkt.FIN = true;
        fin_pkt.payload = "";
        sendPacket(fin_pkt);

        std::cout << "[" << role_label << "] Sent FIN, waiting for final ACK..." << std::endl;
        return;
    }

    if (recv_buffer.isFull()) return;

    if (pkt.seq_num == ack_num) {
        recv_buffer.push(pkt.seq_num, pkt.payload);
        ack_num++;

        TCPPacket ack_pkt;
        ack_pkt.seq_num = 0;
        ack_pkt.ack_num = ack_num;
        ack_pkt.ACK = true;
        ack_pkt.payload = "";
        sendPacket(ack_pkt);
    }
}

void TCPConnection::handleACK(uint32_t ack) {
    for (auto it = send_window.begin(); it != send_window.end();) {
        if (it->first < ack) {
            it = send_window.erase(it);
        } else {
            ++it;
        }
    }
}

void TCPConnection::printFromBuffer() {
    if (!recv_buffer.isEmpty()) {
        std::string word = recv_buffer.pop();
        std::cout << "[" << role_label << "] << " << word << std::endl;
    }
}

std::string TCPConnection::popWord() {
    if (!recv_buffer.isEmpty()) {
        return recv_buffer.pop();
    }
    return "";
}

void TCPConnection::initiateClose() {
    TCPPacket fin_pkt;
    fin_pkt.seq_num = seq_num;
    fin_pkt.ack_num = ack_num;
    fin_pkt.FIN = true;
    fin_pkt.payload = "";
    sendPacket(fin_pkt);
    std::cout << "[" << role_label << "] Sent FIN, waiting for ACK and remote FIN..." << std::endl;
}

void TCPConnection::handleClose() {
    while (!connection_closed) {
        std::string raw = sock->receiveRaw();
        if (raw.empty()) continue;

        TCPPacket pkt = TCPPacket::deserialize(raw);

        if (pkt.FIN) {
            std::cout << "[" << role_label << "] Received FIN from remote." << std::endl;
            std::cout << "[" << role_label << "] Do you still have something to send? (y/n): ";
            std::string choice;
            std::getline(std::cin, choice);

            if (choice == "y") {
                std::cout << "[" << role_label << "] Please finish sending before closing." << std::endl;
                continue;
            }

            TCPPacket ack_pkt;
            ack_pkt.seq_num = 0;
            ack_pkt.ack_num = pkt.seq_num + 1;
            ack_pkt.ACK = true;
            ack_pkt.payload = "";
            sendPacket(ack_pkt);

            TCPPacket fin_pkt;
            fin_pkt.seq_num = seq_num;
            fin_pkt.ack_num = pkt.seq_num + 1;
            fin_pkt.FIN = true;
            fin_pkt.payload = "";
            sendPacket(fin_pkt);

            std::cout << "[" << role_label << "] Sent FIN, waiting for final ACK..." << std::endl;
        } else if (pkt.ACK && pkt.ack_num == seq_num + 1) {
            std::cout << "[" << role_label << "] Received final ACK. Connection closed." << std::endl;
            connection_closed = true;
        }
    }
}

bool TCPConnection::receiveHandshake() {
    std::string raw = sock->receiveRaw();
    if (raw.empty()) return false;

    TCPPacket syn_pkt = TCPPacket::deserialize(raw);
    if (!syn_pkt.SYN) return false;

    ack_num = syn_pkt.seq_num + 1;
    seq_num = 100;  // 初始可自定义

    TCPPacket syn_ack;
    syn_ack.seq_num = seq_num;
    syn_ack.ack_num = ack_num;
    syn_ack.SYN = true;
    syn_ack.ACK = true;
    syn_ack.payload = "";
    sendPacket(syn_ack);

    std::string ack_raw = sock->receiveRaw();
    if (ack_raw.empty()) return false;

    TCPPacket ack_pkt = TCPPacket::deserialize(ack_raw);
    if (ack_pkt.ACK && ack_pkt.ack_num == seq_num + 1) {
        seq_num++;
        std::cout << "[" << role_label << "] Handshake completed (server side)." << std::endl;
        return true;
    }

    return false;
}

void TCPConnection::debugPrintRecvBuffer() const {
    auto snapshot = recv_buffer.snapshot();
    std::cout << "[" << role_label << "] Current recv_buffer state (" << snapshot.size() << " items):\n";
    for (const auto& [seq, word] : snapshot) {
        std::cout << "  [" << seq << "] " << word << "\n";
    }
}
