#include <net/udp_view.hpp>
#include <catch.hpp>

#include <net/ipv6_view.hpp>
#include "sample_packet.hpp"

TEST_CASE("udp_view") {
    net::ipv6_const_view ipv6{sample_ipv6_udp_dns_query_a};
    REQUIRE(ipv6.next_header() == 17);
    net::udp_const_view udp{ipv6.next_header_data()};
    REQUIRE(udp.src_port() == 65021);
    REQUIRE(udp.dst_port() == 53);
    REQUIRE(udp.len() == 36);
    REQUIRE(udp.checksum() == 0x4bd5);
}
