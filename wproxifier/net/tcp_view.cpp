#include <net/tcp_view.hpp>
#include <catch.hpp>

#include <net/ipv4_view.hpp>
#include "sample_packet.hpp"

using namespace net;

TEST_CASE("tcp_view") {
    ipv4_const_view ip{sample_ipv4_tcp_syn};
    REQUIRE(ip.protocol() == 6);
    tcp_const_view tcp{ip.payload()};
    REQUIRE(tcp.src_port() == 54594);
    REQUIRE(tcp.dst_port() == 80);
    REQUIRE(tcp.seq_num() == 812484767);
    REQUIRE(tcp.ack_num() == 0);
    REQUIRE(tcp.header_len() == 8);
    REQUIRE(tcp.ack() == 0);
    REQUIRE(tcp.psh() == 0);
    REQUIRE(tcp.rst() == 0);
    REQUIRE(tcp.syn() == 1);
    REQUIRE(tcp.fin() == 0);
    REQUIRE(tcp.window_size() == 64240);
    REQUIRE(tcp.checksum() == 0x38ba);
}
