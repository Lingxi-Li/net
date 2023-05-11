#include "ipv6_view.hpp"
#include "catch.hpp"

#include "test_utility.hpp"

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
}
