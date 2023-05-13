#include "ipv6_view.hpp"
#include "catch.hpp"

#include "test_utility.hpp"
#include "sample_packet.hpp"

using namespace std;
using namespace net;

TEST_CASE("ipv6_addr_view") {
    byte_vec vec{ 
        0x26, 0x00, 0x14, 0x13,
        0xb0, 0x00, 0x00, 0x1d,
        0x00, 0x00, 0x00, 0x00,
        0x17, 0xd1, 0x2e, 0x96 
    };
    ipv6_addr_view view{vec.data()};
    REQUIRE(str_via_ostream(view) == "2600:1413:b000:1d:0:0:17d1:2e96");
    REQUIRE(str_via_format(view) == "2600:1413:b000:1d:0:0:17d1:2e96");
    view[0] = 0;
    REQUIRE(vec[0] == 0);
    REQUIRE(str_via_ostream(view) == "0:1413:b000:1d:0:0:17d1:2e96");
    view[0] = 0xff;
    view[1] = 0xff;
    REQUIRE(str_via_ostream(view) == "ffff:1413:b000:1d:0:0:17d1:2e96");
}

TEST_CASE("ipv6_view") {
    ipv6_const_view view{sample_ipv6_udp_dns_query_a};
    REQUIRE(view.version() == 6);
    REQUIRE(view.diff_serv() == 0);
    REQUIRE(view.ecn() == 0);
    REQUIRE(view.flow() == 0x2c0f4);
    REQUIRE(view.payload_len() == 36);
    REQUIRE(view.next_header() == 17);
    REQUIRE(view.hop_limit() == 128);
    REQUIRE(str_via_format(view.src_addr()) == "2404:f801:10:102:0:0:0:35");
    REQUIRE(str_via_format(view.dst_addr()) == "2001:4898:0:0:0:0:1050:1050");
}
