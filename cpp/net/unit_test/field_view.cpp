#include "field_view.hpp"
#include "catch.hpp"

#include <array>
#include <vector>
#include <sstream>

using namespace net;
using namespace impl;
using namespace std;

TEST_CASE("round_trip_uint_little_endian", "[impl]") {
    constexpr auto len = 3;

    SECTION("{ 0, 1, 1 }")
    {
        vector<byte_t> const vec{ 0, 1, 1, 0, 0 };
        auto uint = parse_uint_little_endian(vec.data(), len);
        auto data = reinterpret_cast<byte_t const*>(&uint);
        REQUIRE((
            data[0] == 1 &&
            data[1] == 1 &&
            data[2] == 0 &&
            data[3] == 0 &&
            data[4] == 0 &&
            data[5] == 0 &&
            data[6] == 0 &&
            data[7] == 0
        ));
        vector<byte_t> vec2(vec.size());
        write_uint_little_endian(vec2.data(), len, uint);
        REQUIRE(vec2 == vec);
    }
    SECTION("{ 1, 1, 0 }")
    {
        vector<byte_t> const vec{ 1, 1, 0, 0, 0 };
        auto uint = parse_uint_little_endian(vec.data(), len);
        auto data = reinterpret_cast<byte_t const*>(&uint);
        REQUIRE((
            data[0] == 0 &&
            data[1] == 1 &&
            data[2] == 1 &&
            data[3] == 0 &&
            data[4] == 0 &&
            data[5] == 0 &&
            data[6] == 0 &&
            data[7] == 0
        ));
        vector<byte_t> vec2(vec.size());
        write_uint_little_endian(vec2.data(), len, uint);
        REQUIRE(vec2 == vec);
    }
}

TEST_CASE("round_trip_big_endian", "[impl]") {
    constexpr auto len = 3;

    SECTION("{ 0, 1, 1 }")
    {
        vector<byte_t> const vec{ 0, 1, 1, 0, 0 };
        auto uint = parse_uint_big_endian(vec.data(), len);
        auto data = reinterpret_cast<byte_t const*>(&uint);
        REQUIRE((
            data[0] == 0 &&
            data[1] == 0 &&
            data[2] == 0 &&
            data[3] == 0 &&
            data[4] == 0 &&
            data[5] == 0 &&
            data[6] == 1 &&
            data[7] == 1
        ));
        vector<byte_t> vec2(vec.size());
        write_uint_big_endian(vec2.data(), len, uint);
        REQUIRE(vec2 == vec);
    }
    SECTION("{ 1, 1, 0 }")
    {
        vector<byte_t> const vec{ 1, 1, 0, 0, 0 };
        auto uint = parse_uint_big_endian(vec.data(), len);
        auto data = reinterpret_cast<byte_t const*>(&uint);
        REQUIRE((
            data[0] == 0 &&
            data[1] == 0 &&
            data[2] == 0 &&
            data[3] == 0 &&
            data[4] == 0 &&
            data[5] == 1 &&
            data[6] == 1 &&
            data[7] == 0
        ));
        vector<byte_t> vec2(vec.size());
        write_uint_big_endian(vec2.data(), len, uint);
        REQUIRE(vec2 == vec);
    }
}

TEST_CASE("init_parse_write_uint", "[impl]") {
    auto v = 1u;
    bool host_is_little_endian = *reinterpret_cast<uint8_t*>(&v);
    if (host_is_little_endian) {
        REQUIRE(parse_uint == &parse_uint_little_endian);
        REQUIRE(write_uint == &write_uint_little_endian);
    }
    else {
        REQUIRE(parse_uint == &parse_uint_big_endian);
        REQUIRE(write_uint == &write_uint_big_endian);
    }
}

TEST_CASE("uint_view") {
    ostringstream ss;
    constexpr auto len = 3;
   
    SECTION("{ 0, 1, 1 }")
    {
        array<byte_t, 5> arr{ 0, 1, 1, 0, 1 };
        uint_view<len> view{arr.data()};
        REQUIRE(view == 257);
        ss << view;
        REQUIRE(ss.str() == "257");
        REQUIRE(format("{}", view) == "257");
        view = 65536;
        REQUIRE((
            arr[0] == 1 &&
            arr[1] == 0 &&
            arr[2] == 0 &&
            arr[3] == 0 &&
            arr[4] == 1
        ));
    }
    SECTION("{ 1, 1, 0 }")
    {
        array<byte_t, 5> arr{ 1, 1, 0, 0, 1 };
        uint_view<len> view{arr.data()};
        REQUIRE(view == 65792);
        ss << view;
        REQUIRE(ss.str() == "65792");
        REQUIRE(format("{}", view) == "65792");
        view = 1;
        REQUIRE((
            arr[0] == 0 &&
            arr[1] == 0 &&
            arr[2] == 1 &&
            arr[3] == 0 &&
            arr[4] == 1
        ));
    }
}

TEST_CASE("num_trailing_zeros", "[impl]") {
    // REQUIRE(num_trailing_zeros<0>() == 8);
    REQUIRE(num_trailing_zeros<0x03>() == 0);
    REQUIRE(num_trailing_zeros<0x18>() == 3);
    REQUIRE(num_trailing_zeros<0x80>() == 7);
}

TEST_CASE("uint_view_bit") {
    ostringstream ss;
    byte_t data = 0x55;

    SECTION("0x07, 0x55") {
        uint_view_bit<0x07> view{&data};
        REQUIRE(view == 5);
        ss << view;
        REQUIRE(ss.str() == "5");
        REQUIRE(format("{}", view) == "5");
        view = 0xff;
        REQUIRE(data == 0x57);
    }
    SECTION("0x3c, 0x55") {
        uint_view_bit<0x3c> view{&data};
        REQUIRE(view == 5);
        view = 0xff;
        REQUIRE(data == 0x7d);
    }
    SECTION("0xe0, 0x55") {
        uint_view_bit<0xe0> view{&data};
        REQUIRE(view == 2);
        ss << view;
        REQUIRE(ss.str() == "2");
        REQUIRE(format("{}", view) == "2");
        view = 0xff;
        REQUIRE(data == 0xf5);
    }
}
