#pragma once

#include <Divert.hpp>

#include <net/constants.hpp>
#include <net/ipv4_view.hpp>
#include <net/tcp_view.hpp>

#include <stdex/format.hpp>

#include <array>
#include <iostream>

inline void RedirPort() {
    constexpr char filter[] = "loopback == 1 && (tcp.DstPort == 8000 || tcp.SrcPort == 8001)";
    auto handle = WinDivertOpen(filter, WINDIVERT_LAYER_NETWORK, 0, 0);
    dvt::Check(handle, dvt::Api::WinDivertOpen);
    std::array<net::byte_t, net::mtu> packet{};
    WINDIVERT_ADDRESS addr{};
    while (true) {
        UINT recvPacketLen = 0;
        auto res = WinDivertRecv(handle, packet.data(), UINT(packet.size()), &recvPacketLen, &addr);
        dvt::Check(res, dvt::Api::WinDivertRecv);

        net::ipv4_view ip{packet.data()};
        net::tcp_view tcp{ip.payload()}; 
        
        stdex::format_to(std::cout, "{} => {}", tcp.src_port(), tcp.dst_port());
        if (tcp.syn()) std::cout << " SIN";
        if (tcp.ack()) std::cout << " ACK";
        if (tcp.fin()) std::cout << " FIN";
        stdex::format_to(std::cout, " {}\n", ip.total_len() - ip.header_len() * 4 - tcp.header_len() * 4);

        do {
            if (tcp.dst_port() == 8000) {
                tcp.dst_port() = 8001;
                break;
            }
            if (tcp.src_port() == 8001) {
                tcp.src_port() = 8000;
                break;
            }
        }
        while (false);
        WinDivertHelperCalcChecksums(packet.data(), recvPacketLen, &addr, 0);

        res = WinDivertSend(handle, packet.data(), UINT(packet.size()), NULL, &addr);
        dvt::Check(res, dvt::Api::WinDivertSend);
    }

}
