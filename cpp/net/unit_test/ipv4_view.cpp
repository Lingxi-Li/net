#include "ipv4_view.hpp"
#include "catch.hpp"

#include "sample_packet.hpp"

#include <cstring>
#include <vector>
#include <sstream>
#include <iostream>
#include <format>

using namespace net;
using namespace std;

TEST_CASE("ipv4_addr_view") {
    ostringstream os;
    SECTION("192.168.2.103") {
        byte_t data[] = { 192, 168, 2, 103 };
        ipv4_addr_view view{data};
        REQUIRE((
            data[0] == 192 &&
            data[1] == 168 &&
            data[2] == 2   &&
            data[3] == 103
        ));
        os << view;
        REQUIRE(os.str() == "192.168.2.103");
        REQUIRE(format("{}", view) == "192.168.2.103");
        data[1] = 0;
        REQUIRE(data[1] == 0);
    }
}

TEST_CASE("ipv4_view") {
    vector<byte_t> vec(
        sample_ipv4_tcp_syn,
        sample_ipv4_tcp_syn + sizeof(sample_ipv4_tcp_syn)
    );
    ipv4_view view{vec.data()};

    REQUIRE(view.version() == 4);
    REQUIRE(view.header_len() == 5);
    REQUIRE(view.diffserv() == 0);
    REQUIRE(view.ecn() == 0);
    REQUIRE(view.id() == 0xde65u);
    REQUIRE(view.no_frag() == 1);
    REQUIRE(view.more_frag() == 0);
    REQUIRE(view.total_len() == 52);
    REQUIRE(view.protocol() == 6);
    REQUIRE(format("{}", view.src_addr()) == "192.168.2.103");
    REQUIRE(format("{}", view.dst_addr()) == "128.119.245.12");

    view.version() = 0;
    view.header_len() = 0;
    view.total_len() = 0;
    view.protocol() = 0;
    memset(view.src_addr().data, 0, 4);
    memset(view.dst_addr().data, 0, 4);

    REQUIRE(view.version() == 0);
    REQUIRE(view.header_len() == 0);
    REQUIRE(view.total_len() == 0);
    REQUIRE(view.protocol() == 0);
    REQUIRE(format("{}", view.src_addr()) == "0.0.0.0");
    REQUIRE(format("{}", view.dst_addr()) == "0.0.0.0");
}
