#include <net/ipv4_view.hpp>
#include <catch.hpp>

#include <test_utility.hpp>
#include "sample_packet.hpp"

TEST_CASE("ipv4_addr_view") {
    byte_vec vec{ 192, 168, 2, 103 };
    net::ipv4_addr_view view{vec.data()};
    REQUIRE(view[0] == 192);
    REQUIRE(view[1] == 168);
    REQUIRE(view[2] == 2);
    REQUIRE(view[3] == 103);
    REQUIRE(str_via_ostream(view) == "192.168.2.103");
    REQUIRE(str_via_format(view) == "192.168.2.103");
    REQUIRE(net::ipv4_addr_const_view{ view } == 0xc0a80267);
    view[0] = 128;
    view[1] = 119;
    view[2] = 245;
    view[3] = 12;
    REQUIRE(view[0] == 128);
    REQUIRE(view[1] == 119);
    REQUIRE(view[2] == 245);
    REQUIRE(view[3] == 12);
    view = 0x5db8d822;
    REQUIRE(str_via_format(view) == "93.184.216.34");
}

TEST_CASE("ipv4_addr") {
    REQUIRE(net::ipv4_addr("93.184.216.34") == 0x5db8d822);
}

TEST_CASE("ipv4_view") {
    net::ipv4_const_view view{sample_ipv4_tcp_syn};
    REQUIRE(view.version() == 4);
    REQUIRE(view.header_len() == 5);
    REQUIRE(view.diff_serv() == 0);
    REQUIRE(view.ecn() == 0);
    REQUIRE(view.total_len() == 52);
    REQUIRE(view.id() == 0xde65);
    REQUIRE(view.no_frag() == 1);
    REQUIRE(view.more_frag() == 0);
    REQUIRE(view.frag_offset() == 0);
    REQUIRE(view.ttl() == 128);
    REQUIRE(view.protocol() == 6);
    REQUIRE(view.header_checksum() == 0);
    REQUIRE(str_via_format(view.src_addr()) == "192.168.2.103");
    REQUIRE(str_via_format(view.dst_addr()) == "128.119.245.12");
    REQUIRE(view.options() == view.payload());
    REQUIRE(view.end() == sample_ipv4_tcp_syn + sizeof(sample_ipv4_tcp_syn));
    REQUIRE(str_via_ostream(view) == "192.168.2.103 -> 128.119.245.12");
    REQUIRE(str_via_format(view) == "192.168.2.103 -> 128.119.245.12");
}
