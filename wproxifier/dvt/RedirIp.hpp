#pragma once

#include <Divert.hpp>

#include <net/constants.hpp>
#include <net/ipv4_view.hpp>
#include <net/tcp_view.hpp>

#include <stdex/format.hpp>

#include <cstring>
#include <future>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <type_traits>

struct RedirIp {
    RedirIp(
          char const* localIp
        , char const* originalRemoteIp
        , char const* redirRemoteIp
    )
        : connections(std::make_unique<Connection[]>(65536))
        , filter(dvt::flt::RelateToIp(localIp) + " && tcp && !loopback")
        , localIp(net::ipv4_addr(localIp))
        , originalRemoteIp(net::ipv4_addr(originalRemoteIp))
        , redirRemoteIp(net::ipv4_addr(redirRemoteIp))
        , handle(INVALID_HANDLE_VALUE) {
    }

    void Start() {
        handle = WinDivertOpen(filter.c_str(), WINDIVERT_LAYER_NETWORK, 0, 0) & dvt::Api::WinDivertOpen;
    }

    struct Connection {
        char const* State{};
        BOOL Outbound{};

        operator bool() const noexcept {
            return State != NULL;
        }

        bool operator==(Connection const& other) const noexcept {
            return State == other.State && Outbound == other.Outbound;
        }

        bool operator!=(Connection const& other) const noexcept {
            return !(*this == other);
        }
    };

private:
    struct State {
        constexpr static char Syn[] = "[SYN]";
        constexpr static char SynAck[] = "[SYN ACK]";
        constexpr static char FinAck[] = "[FIN ACK]";
        constexpr static char Ack[] = "[ACK]";
    };

    BOOL Twiddle(net::ipv4_view ip, net::tcp_view tcp, WINDIVERT_ADDRESS& addr, UINT64) {
        constexpr UINT MASK = 0x17;
        constexpr UINT SYN = 0x02;
        constexpr UINT SYN_ACK = 0x12;
        constexpr UINT FIN_ACK = 0x11;
        constexpr UINT ACK = 0x10;

        std::uint32_t srcIp = ip.src_addr();
        UINT flags = UINT(tcp.flags().uint()) & MASK;
        if (srcIp == localIp) {
            // outbound
            auto localPort = UINT(tcp.src_port());
            auto& conn = connections[localPort];
            switch (flags) {
                case SYN: // connect to remote IP
                    if (ip.dst_addr() != originalRemoteIp) {
                        return FALSE; // not interested
                    }
                    if (conn) {
                        // drop and log problematic packet
                        auto what = std::format("-> [SYN] conflicts with {}", conn);
                        throw std::logic_error(what);
                    }
                    conn = Connection{ State::Syn, TRUE }; // register connection
                    ip.dst_addr() = redirRemoteIp; // redir
                    return TRUE;
                case ACK:
                    if (!conn) return FALSE; // not interested

                    
                    
            }
        }
        else {
            // inbound
            auto localPort = UINT(tcp.dst_port());
            auto& conn = connections[localPort];
            if (!conn) {
                // connection not tracked
                return FALSE;
            }
            switch (flags) {
                case SYN_ACK:
                    if (conn != Connection{ State::Syn, TRUE }) {
                        auto what = std::format("<- [SYN ACK] conflicts with {}", conn);
                        throw std::logic_error(what);
                    }
                    conn = Connection{ State::SynAck, FALSE };
                    ip.src_addr() = originalRemoteIp;
                    return TRUE;
            }
        }
    }

    std::unique_ptr<Connection[]> connections;
    std::string filter;
    UINT32 localIp;
    UINT32 originalRemoteIp;
    UINT32 redirRemoteIp;
    HANDLE handle;
    std::future<UINT64> future;
};

template <>
struct std::formatter<RedirIp::Connection>: stdex::naive_formatter {
    auto format(RedirIp::Connection const& conn, format_context& ctx) const {
        return format_to(ctx.out(), "{} {}",
            conn.Outbound ? "->" : "<-",
            conn.State);
    }
};

////////////////////////////////////////

inline constexpr UINT MTU = net::mtu * 10;

struct Connection {
    enum struct StateT: unsigned {
        Null,
        ClientSyn,
        ProxySynAck
    };

    void SetClientSyn(net::ipv4_const_view ip, WINDIVERT_ADDRESS const& addr) noexcept {
        net::tcp_const_view tcp{ip.payload()};
        std::memcpy(DstIp, ip.dst_addr().data, sizeof(DstIp));
        DstPort = UINT16(tcp.dst_port());
        std::memcpy(ClientSynPacket, ip.data, ip.total_len());
        ClientSynAddr = addr;
        ClientSynIp.data = ClientSynPacket;
        ClientSynTcp.data = ClientSynIp.payload();
        State = StateT::ClientSyn;
    }

    void SetProxySynAck(net::ipv4_const_view ip, WINDIVERT_ADDRESS const& addr) noexcept {
        net::tcp_const_view tcp{ip.payload()};
        std::memcpy(ProxySynAckPacket, ip.data, ip.total_len());
        ProxySynAckAddr = addr;
        ProxySynAckIp.data = ProxySynAckPacket;
        ProxySynAckTcp.data = ProxySynAckIp.payload();
        State = StateT::ProxySynAck;
    }

    StateT State;

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

#define PROXIFIER_DST_IP "93.184.216.34"
#define PROXIFIER_OUTBOUND_IP "20.44.227.185"
#define PROXIFIER_LOCAL_IP 

inline void ModifyDst(net::ipv4_view ip) noexcept {
    // 93.184.216.34
    auto dstIp = ip.dst_addr().data;
    dstIp[0] = 93;
    dstIp[1] = 184;
    dstIp[2] = 216;
    dstIp[3] = 34;
    //net::tcp_view tcp{ip.payload()};
    //tcp.dst_port() = 8080;
}

inline void ModifySrc(net::ipv4_view ip) noexcept {
    // 180.101.50.242
    auto srcAddr = ip.src_addr();
    srcAddr[0] = 180;
    srcAddr[1] = 101;
    srcAddr[2] = 50;
    srcAddr[3] = 242;
}

inline UINT16 ClientPort(net::ipv4_const_view ip) noexcept {
    net::tcp_const_view tcp{ip.payload()};
    auto srcIp = ip.src_addr().data;
    if (srcIp[0] == 192) {
        return UINT16(tcp.src_port());
    }
    else {
        return UINT16(tcp.dst_port());
    }
}

inline BOOL Outbound(net::ipv4_const_view ip) noexcept {
    return ip.src_addr()[0] == 192;
}

inline void Listen() {
    net::byte_t packet[MTU]{};
    WINDIVERT_ADDRESS addr{};

    constexpr char filter[] =
        "ip"
        " && (tcp.Syn && !tcp.Ack)"
        ;
    auto handle = WinDivertOpen(filter, WINDIVERT_LAYER_NETWORK, 0, 0);
    dvt::Check(handle, dvt::Api::WinDivertOpen);
    for (UINT i = 0; true; ++i) {
        auto res = WinDivertRecv(handle, packet, MTU, NULL, &addr);
        dvt::Check(res, dvt::Api::WinDivertRecv);
        
        net::ipv4_view ip{packet};
        net::tcp_view tcp{ip.payload()};

        
        if (tcp.ack()) {
            // SYN ACK
            auto clientPort = ClientPort({ ip.data });
            auto& conn = Connections[clientPort];
            if (conn.State != Connection::StateT::Null) {
                if (Outbound({ ip.data })) {
                    ModifyDst(ip);
                }
                else {
                    ModifySrc(ip);
                }
                WinDivertHelperCalcChecksums(packet, MTU, &addr, 0);
            }
        }
        else {
            // SYN
            auto dstAddr = ip.dst_addr();
            // 180.101.50.242
            if (dstAddr[0] == 180 &&
                dstAddr[1] == 101 &&
                dstAddr[2] == 50 &&
                dstAddr[3] == 242)
            {
                auto clientPort = UINT16(tcp.src_port());
                auto& conn = Connections[clientPort];
                conn.SetClientSyn({ ip.data }, addr);
                //stdex::format_to(std::cout, "{}:{} SYN\n"
                //    , net::ipv4_addr_const_view{ conn.DstIp }
                //    , conn.DstPort
                //);
                ModifyDst(ip);
                WinDivertHelperCalcChecksums(packet, MTU, &addr, 0);
            }
        }

        res = WinDivertSend(handle, packet, MTU, NULL, &addr);
        dvt::Check(res, dvt::Api::WinDivertSend);
    }
}

inline void Bridge() {
    net::byte_t packet[MTU]{};
    WINDIVERT_ADDRESS addr{};

    constexpr char filter[] =
        "ip"
        " && !tcp.Syn"
        ;
    auto handle = WinDivertOpen(filter, WINDIVERT_LAYER_NETWORK, 0, 0);
    dvt::Check(handle, dvt::Api::WinDivertOpen);
    for (UINT i = 0; true; ++i) {
        auto res = WinDivertRecv(handle, packet, MTU, NULL, &addr);
        dvt::Check(res, dvt::Api::WinDivertRecv);
        
        net::ipv4_view ip{packet};
        net::tcp_view tcp{ip.payload()};

        auto clientPort = ClientPort({ ip.data });
        auto& conn = Connections[clientPort];
        if (conn.State != Connection::StateT::Null) {
            if (Outbound({ ip.data })) {
                ModifyDst(ip);
            }
            else {
                ModifySrc(ip);
            }
            WinDivertHelperCalcChecksums(packet, MTU, &addr, 0);
        }

        res = WinDivertSend(handle, packet, MTU, NULL, &addr);
        dvt::Check(res, dvt::Api::WinDivertSend);
    }
}

inline void Proxify() {
    auto future = std::async(std::launch::async, &Listen);
    Bridge();
}
