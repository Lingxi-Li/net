#pragma once

#include <Divert.hpp>

#include <net/constants.hpp>
#include <net/ipv4_view.hpp>
#include <net/tcp_view.hpp>

#include <stdex/format.hpp>

#include <cstring>
#include <iostream>
#include <type_traits>

inline constexpr UINT MTU = net::mtu * 10;

struct Connection {
    void SetClientSyn(net::ipv4_const_view ip, WINDIVERT_ADDRESS const& addr) noexcept {
        net::tcp_const_view tcp{ip.payload()};
        std::memcpy(DstIp, ip.dst_addr().data, sizeof(DstIp));
        DstPort = UINT16(tcp.dst_port());
        std::memcpy(ClientSynPacket, ip.data, ip.total_len());
        ClientSynAddr = addr;
        ClientSynIp.data = ClientSynPacket;
        ClientSynTcp.data = ClientSynIp.payload();
    }

    void SetProxySynAck(net::ipv4_const_view ip, WINDIVERT_ADDRESS const& addr) noexcept {
        net::tcp_const_view tcp{ip.payload()};
        std::memcpy(ProxySynAckPacket, ip.data, ip.total_len());
        ProxySynAckAddr = addr;
        ProxySynAckIp.data = ProxySynAckPacket;
        ProxySynAckTcp.data = ProxySynAckIp.payload();
    }

    net::byte_t DstIp[4];
    UINT16 DstPort;

    net::byte_t ClientSynPacket[MTU];
    WINDIVERT_ADDRESS ClientSynAddr;
    net::byte_t ProxySynAckPacket[MTU];
    WINDIVERT_ADDRESS ProxySynAckAddr;

    net::ipv4_view ClientSynIp;
    net::tcp_view ClientSynTcp;
    net::ipv4_view ProxySynAckIp;
    net::tcp_view ProxySynAckTcp;
};
static_assert(std::is_aggregate_v<Connection>);

inline Connection Connections[0x10000];

inline void Proxify() {
    net::byte_t packet[MTU]{};
    WINDIVERT_ADDRESS addr{};

    constexpr char filter[] =
        "remoteAddr == 180.101.50.188"
        " && tcp.Syn";
    auto handle = WinDivertOpen(filter, WINDIVERT_LAYER_NETWORK, 0, 0);
    dvt::Check(handle, dvt::Api::WinDivertOpen);
    for (UINT i = 0; true; ++i) {
        auto res = WinDivertRecv(handle, packet, MTU, NULL, &addr);
        dvt::Check(res, dvt::Api::WinDivertRecv);
        
        net::ipv4_view ip{packet};
        net::tcp_view tcp{ip.payload()};
        stdex::format_to(std::cout, "{}, {}\n", tcp, ip);

        if (tcp.ack()) {
            // SYN ACK
            auto clientPort = UINT16(tcp.dst_port());
            auto& conn = Connections[clientPort];
            conn.SetProxySynAck({ ip.data }, addr);
            stdex::format_to(std::cout, "{}, {}\n\n",
                conn.ProxySynAckTcp, conn.ProxySynAckIp);
        }
        else {
            // SYN
            auto clientPort = UINT16(tcp.src_port());
            auto& conn = Connections[clientPort];
            conn.SetClientSyn({ ip.data }, addr);
            stdex::format_to(std::cout, "{}, {}\n\n",
                conn.ClientSynTcp, conn.ClientSynIp);
        }

        res = WinDivertSend(handle, packet, MTU, NULL, &addr);
        dvt::Check(res, dvt::Api::WinDivertSend);
    }
}
