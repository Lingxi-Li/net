#include <stdex/format.hpp>
#include <catch.hpp>

#include <test_utility.hpp>

namespace {

enum struct pet: unsigned {
    cat,
    dog
};

struct nest {
    enum struct bird: unsigned {
        swallow,
        eagle
    };
};

} // unnamed namespace

STDEX_DEFINE_ENUM_FORMATTER(, pet,
    "cat",
    "dog"
);

STDEX_DEFINE_ENUM_FORMATTER(, nest::bird,
    "swallow",
    "eagle"
);

namespace fmt_test {

enum struct direction: unsigned {
    inbound,
    outbound
};

struct outer {
    struct inner {
        enum struct name: unsigned {
            lingxi,
            echo
        };
    };
};

} // namespace fmt_test

STDEX_DEFINE_ENUM_FORMATTER(fmt_test, direction,
    "<-",
    "->"
);

STDEX_DEFINE_ENUM_FORMATTER(fmt_test, outer::inner::name,
    "lingxi",
    "echo"
);

TEST_CASE("format_to") {
    std::ostringstream os;
    auto& ref = stdex::format_to(os, "{:x}", 65535);
    REQUIRE(os.str() == "ffff");
    REQUIRE(&ref == &os);
}

TEST_CASE("DEFINE_ENUM_FORMATTER") {
    REQUIRE(str_via_format(pet::cat) == "cat");
    REQUIRE(str_via_ostream(pet::dog) == "dog");
    REQUIRE(str_via_format(nest::bird::swallow) == "swallow");
    REQUIRE(str_via_ostream(nest::bird::eagle) == "eagle");
    REQUIRE(str_via_format(fmt_test::direction::inbound) == "<-");
    REQUIRE(str_via_ostream(fmt_test::direction::outbound) == "->");
    REQUIRE(str_via_format(fmt_test::outer::inner::name::lingxi) == "lingxi");
    REQUIRE(str_via_ostream(fmt_test::outer::inner::name::echo) == "echo");
}
