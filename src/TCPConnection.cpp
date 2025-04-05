//
// Created by Storm on 2025/4/1.
//
//
// TCPConnection.cpp
//
#include "TCPConnection.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>

TCPConnection::TCPConnection(SocketWrapper* socket, const std::string& role)
        : sock(socket), recv_buffer(10) // Set receive buffer size to 10
{
    this->ack_num = 0;
    this->role_label = role;
    this->seq_num = (role == "Client") ? 1000 : 2000;  // Client starts from 1000, Server from 2000
}

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

        std::cout << "[" << role_label << "] Handshake completed (client side)" << std::endl;
        return true;
    }
    return false;
}

bool TCPConnection::receiveHandshake() {
    std::string raw = sock->receiveRaw();
    if (raw.empty()) return false;

    TCPPacket syn_pkt = TCPPacket::deserialize(raw);
    if (!syn_pkt.SYN) return false;

    ack_num = syn_pkt.seq_num + 1;
    seq_num = 2000;  // Server initial sequence number

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
        std::cout << "[" << role_label << "] Handshake completed (server side)" << std::endl;
        return true;
    }
    return false;
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
    // Sliding window control: if the number of unacknowledged packets in the send window reaches 2, wait.
    while (send_window.size() >= 2) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::string data = pkt.serialize();
    bool success = sock->sendRaw(data);
    if (success && !pkt.payload.empty()) {
        send_window[pkt.seq_num] = pkt;
        seq_num++;  // Update sequence number
        // Control sending rate: 0.5 second delay between packets
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    return success;
}

void TCPConnection::receivePacketLoop() {
    while (!connection_closed) {
        std::string raw = sock->receiveRaw();
        if (raw.empty()) continue;
        TCPPacket pkt = TCPPacket::deserialize(raw);
        std::cout << "[" << role_label << "] Received packet -- SEQ: "
                  << pkt.seq_num << ", ACK: " << pkt.ack_num
                  << ", PAYLOAD: '" << pkt.payload << "'" << std::endl;
        processReceivedPacket(pkt);
    }
}

void TCPConnection::processReceivedPacket(const TCPPacket& pkt) {
    // Process ACK packet
    if (pkt.ACK) {
        // If the received ACK packet has payload "[BUFFER_FULL]", it means the remote receive buffer is full.
        if (pkt.payload == "[BUFFER_FULL]") {
            std::cout << "[" << role_label << "] Remote receive buffer is full, waiting 2 seconds before retransmitting unacknowledged packets.\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            // Retransmit all packets in the send window
            for (auto& entry : send_window) {
                std::cout << "[" << role_label << "] Retransmitting packet SEQ: " << entry.first << "\n";
                sendPacket(entry.second);
            }
        } else {
            handleACK(pkt.ack_num);
        }
        return;
    }

    // Process FIN packet: initiate connection termination process
    if (pkt.FIN) {
        std::cout << "[" << role_label << "] Received FIN packet, initiating connection termination process.\n";
        // Send ACK to confirm FIN
        TCPPacket ack_pkt;
        ack_pkt.seq_num = 0;
        ack_pkt.ack_num = pkt.seq_num + 1;
        ack_pkt.ACK = true;
        ack_pkt.payload = "";
        sendPacket(ack_pkt);
        connection_closed = true; // Close connection
        return;
    }

    // If receive buffer is full, send special packet to notify remote to pause sending for 2 seconds before retransmitting.
    if (recv_buffer.isFull()) {
        TCPPacket wait_pkt;
        wait_pkt.seq_num = 0;
        wait_pkt.ack_num = ack_num;
        wait_pkt.ACK = true;
        wait_pkt.payload = "[BUFFER_FULL]";
        sendPacket(wait_pkt);
        return;
    }

    // Normal data processing: if packet sequence number matches expected, add to receive buffer and send ACK
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
    for (auto it = send_window.begin(); it != send_window.end(); ) {
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
        std::cout << "[" << role_label << "] Payload: '" << word << "'" << std::endl;
    }
}

std::string TCPConnection::popWord() {
    if (!recv_buffer.isEmpty()) {
        return recv_buffer.pop();
    }
    return "";
}

void TCPConnection::debugPrintRecvBuffer() const {
    auto snapshot = recv_buffer.snapshot();
    std::cout << "[" << role_label << "] Current receive buffer state (" << snapshot.size() << " items):\n";
    for (const auto& [seq, word] : snapshot) {
        std::cout << "  [" << seq << "] " << word << "\n";
    }
}

void TCPConnection::initiateClose() {
    TCPPacket fin_pkt;
    fin_pkt.seq_num = seq_num;
    fin_pkt.ack_num = ack_num;
    fin_pkt.FIN = true;
    fin_pkt.payload = "";
    sendPacket(fin_pkt);
    std::cout << "[" << role_label << "] Sent FIN, waiting for remote confirmation of connection termination..." << std::endl;
}

void TCPConnection::handleClose() {
    while (!connection_closed) {
        std::string raw = sock->receiveRaw();
        if (raw.empty()) continue;

        TCPPacket pkt = TCPPacket::deserialize(raw);

        if (pkt.FIN) {
            std::cout << "[" << role_label << "] Received FIN packet, initiating connection termination process.\n";
            // Directly send ACK and FIN without prompting
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

            std::cout << "[" << role_label << "] FIN sent, waiting for final ACK...\n";
        } else if (pkt.ACK && pkt.ack_num == seq_num + 1) {
            std::cout << "[" << role_label << "] Received final ACK, connection closed.\n";
            connection_closed = true;
        }
    }
}
