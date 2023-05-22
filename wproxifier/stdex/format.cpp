#include <stdex/format.hpp>
#include <catch.hpp>

#include <sstream>

using namespace std;
using namespace stdex;

TEST_CASE("format_to") {
    ostringstream os;
    auto& ref = format_to(os, "{:x}", 65535);
    REQUIRE(os.str() == "ffff");
    REQUIRE(&ref == &os);
}
