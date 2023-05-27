#include <stdex/scope.hpp>
#include <catch.hpp>

TEST_CASE("scope_exit") {
    int a = 0;
    {
        auto guard = stdex::make_scope_exit([&] { ++a; });
        STDEX_FINALLY(
            ++a;
        );
        STDEX_FINALLY(
            ++a;
        );
    }
    REQUIRE(a == 3);
}
