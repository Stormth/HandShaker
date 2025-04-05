//
// Created by Storm on 2025/4/1.
//
// TCPConnection.cpp
// TCPConnection.cpp
// TCPConnection.cpp
// TCPConnection.cpp
// TCPConnection.cpp
// TCPConnection.cpp
// TCPConnection.cpp
//
// TCPConnection.cpp
//
#include "TCPConnection.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>

TCPConnection::TCPConnection(SocketWrapper* socket, const std::string& role)
        : sock(socket), recv_buffer(10) // 将接收缓存大小设置为10
{
    this->ack_num = 0;
    this->role_label = role;
    this->seq_num = (role == "Client") ? 1000 : 2000;  // 客户端从1000开始，服务端从2000开始
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

        std::cout << "[" << role_label << "] 握手完成（客户端）" << std::endl;
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
    seq_num = 2000;  // 服务端初始序列号

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
        std::cout << "[" << role_label << "] 握手完成（服务端）" << std::endl;
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
    // 滑动窗口控制：如果发送窗口中未确认的报文数达到2，则等待
    while (send_window.size() >= 2) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::string data = pkt.serialize();
    bool success = sock->sendRaw(data);
    if (success && !pkt.payload.empty()) {
        send_window[pkt.seq_num] = pkt;
        seq_num++;  // 更新序列号
        // 控制发送速率：每个报文间隔 0.5 秒
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    return success;
}

void TCPConnection::receivePacketLoop() {
    while (!connection_closed) {
        std::string raw = sock->receiveRaw();
        if (raw.empty()) continue;
        TCPPacket pkt = TCPPacket::deserialize(raw);
        std::cout << "[" << role_label << "] 收到报文 -- SEQ: "
                  << pkt.seq_num << ", ACK: " << pkt.ack_num
                  << ", PAYLOAD: '" << pkt.payload << "'" << std::endl;
        processReceivedPacket(pkt);
    }
}

void TCPConnection::processReceivedPacket(const TCPPacket& pkt) {
    // 处理 ACK 报文
    if (pkt.ACK) {
        // 若收到特殊 ACK，表示对方接收缓存已满
        if (pkt.payload == "[BUFFER_FULL]") {
            std::cout << "[" << role_label << "] 对方接收缓存已满，等待2秒后重传未确认数据。\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            // 重传发送窗口中所有报文
            for (auto& entry : send_window) {
                std::cout << "[" << role_label << "] 重传报文 SEQ: " << entry.first << "\n";
                sendPacket(entry.second);
            }
        } else {
            handleACK(pkt.ack_num);
        }
        return;
    }

    // 处理 FIN 报文：进入挥手断连流程
    if (pkt.FIN) {
        std::cout << "[" << role_label << "] 收到 FIN 报文，开始挥手断连流程。\n";
        // 发送 ACK 对 FIN 的确认
        TCPPacket ack_pkt;
        ack_pkt.seq_num = 0;
        ack_pkt.ack_num = pkt.seq_num + 1;
        ack_pkt.ACK = true;
        ack_pkt.payload = "";
        sendPacket(ack_pkt);
        connection_closed = true; // 关闭连接
        return;
    }

    // 如果接收缓存已满，则发送特殊报文通知对方暂停发送2秒后重传
    if (recv_buffer.isFull()) {
        TCPPacket wait_pkt;
        wait_pkt.seq_num = 0;
        wait_pkt.ack_num = ack_num;
        wait_pkt.ACK = true;
        wait_pkt.payload = "[BUFFER_FULL]";
        sendPacket(wait_pkt);
        return;
    }

    // 正常数据处理：如果报文序号与预期相符，则加入接收缓存，并发送 ACK
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
    std::cout << "[" << role_label << "] 当前接收缓存状态 (" << snapshot.size() << " 个数据):\n";
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
    std::cout << "[" << role_label << "] 发送 FIN，等待对方确认断连..." << std::endl;
}

void TCPConnection::handleClose() {
    while (!connection_closed) {
        std::string raw = sock->receiveRaw();
        if (raw.empty()) continue;

        TCPPacket pkt = TCPPacket::deserialize(raw);

        if (pkt.FIN) {
            std::cout << "[" << role_label << "] 收到 FIN 报文，开始挥手断连流程。\n";
            std::cout << "[" << role_label << "] 还有数据要发送吗？ (y/n): ";
            std::string choice;
            std::getline(std::cin, choice);
            if (choice == "y") {
                std::cout << "[" << role_label << "] 请完成数据发送后再关闭连接。\n";
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

            std::cout << "[" << role_label << "] 已发送 FIN，等待最终 ACK...\n";
        } else if (pkt.ACK && pkt.ack_num == seq_num + 1) {
            std::cout << "[" << role_label << "] 收到最终 ACK，连接关闭。\n";
            connection_closed = true;
        }
    }
}
