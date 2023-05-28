#include <net/field_view.hpp>
#include <catch.hpp>

#include <test_utility.hpp>

using namespace std;
using namespace net;

TEST_CASE("uint_view") {
    SECTION("len 2") {
        constexpr auto len = 2u;
        SECTION("data 0xa55a") {
            byte_vec vec{ 0xa5, 0x5a };
            SECTION("hishf 0, loshf 0") {
                uint_view<len> view{vec.data()};
                REQUIRE(view == view.uint());
                REQUIRE(view == 0xa55a);
                REQUIRE(str_via_ostream(view) == "42330");
                REQUIRE(str_via_format(view) == "42330");
                view = 0x5aa5;
                REQUIRE(vec == byte_vec{ 0x5a, 0xa5 });
                REQUIRE(view == 0x5aa5);
            }
            SECTION("hishf 3, loshf 7") {
                uint_view<len, 3, 7> view{vec.data()};
                REQUIRE(view == view.uint());
                REQUIRE(view == 0xa);
                view = 0xffff;
                REQUIRE(vec == byte_vec{ 0xbf, 0xda });
                REQUIRE(view == 63);
            }
        }
    }
    SECTION("len 4") {
        constexpr unsigned len = 4u;
        SECTION("data 0xa500ff5a") {
            byte_vec vec{ 0xa5, 0x00, 0xff, 0x5a };
            SECTION("hishf 0, loshf 0") {
                uint_view<len> view{vec.data()};
                REQUIRE(view == view.uint());
                REQUIRE(view == 0xa500ff5a);
                view = 0x5aff00a5;
                REQUIRE(vec == byte_vec{ 0x5a, 0xff, 0x00, 0xa5 });
                REQUIRE(view == 0x5aff00a5);
            }
            SECTION("hishf 3, loshf 7") {
                uint_view<len, 3, 7> view{vec.data()};
                REQUIRE(view == view.uint());
                REQUIRE(view == 0xa01fe);
                view = 0x1f54ab5;
                REQUIRE(vec == byte_vec{ 0xba, 0xa5, 0x5a, 0xda });
                REQUIRE(view == 0x354ab5);
            }
        }
    }
    SECTION("len 1") {
        constexpr auto len = 1u;
        SECTION("0xa5") {
            byte_t data = 0xa5;
            SECTION("hishf 0, loshf 0") {
                uint_view<len> view{&data};
                REQUIRE(view == view.uint());
                REQUIRE(view == 0xa5);
                view = 0x5a;
                REQUIRE(data == 0x5a);
                REQUIRE(view == 0x5a);
            }
            SECTION("hishf 2, loshf 3") {
                uint_view<len, 2, 3> view{&data};
                REQUIRE(view == view.uint());
                REQUIRE(view == 0x4);
                view = 0x1b;
                REQUIRE(data == 0x9d);
                REQUIRE(view == 0x3);
            }
            SECTION("hishf 1, loshf 6") {
                uint_view<len, 1, 6> view{&data};
                REQUIRE(view == view.uint());
                REQUIRE(view == 0);
                view = 0xff;
                REQUIRE(data == 0xe5);
                REQUIRE(view == 1);
            }
            SECTION("bit_view") {
                bit_view<1> view{&data};
                REQUIRE(view == view.uint());
                REQUIRE(view == 0);
                view = 0xff;
                REQUIRE(data == 0xe5);
                REQUIRE(view == 1);
            }
        }
    }
}

TEST_CASE("flags_view") {
    SECTION("len 8, hishf 0, loshf 0, mask 0x8000000000000001") {
        constexpr unsigned len = 8;
        byte_vec vec(len, 0xff);
        flags_view<len, 0, 0, 0x8000000000000001> flags{vec.data()};
        REQUIRE(flags == 0x8000000000000001);
        REQUIRE(str_via_format(flags) == "0x8000000000000001");
        REQUIRE(str_via_ostream(flags) == "0x8000000000000001");
        flags = 0;
        REQUIRE(vec == byte_vec{ 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe });
        REQUIRE(flags.uint64() == 0);
        REQUIRE(str_via_format(flags) == "0x0");
        REQUIRE(str_via_ostream(flags) == "0x0");
        flags = 0x8000000000000000;
        REQUIRE(vec == byte_vec{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe });
        REQUIRE(flags == 0x8000000000000000);
        flags = 1;
        REQUIRE(vec == byte_vec{ 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff });
        REQUIRE(flags == 1);
    }
    SECTION("len 3, hishf 4, loshf 4, mask 0x300c") {
        constexpr unsigned len = 3;
        byte_vec vec(len, 0xff);
        flags_view<len, 4, 4, 0x300c> flags{vec.data()};
        REQUIRE(flags == 0x300c);
        flags = 0;
        REQUIRE(vec == byte_vec{ 0xfc, 0xff, 0x3f });
        REQUIRE(flags.uint64() == 0);
        flags = 0x1008;
        REQUIRE(vec == byte_vec{ 0xfd, 0xff, 0xbf });
        REQUIRE(flags == 0x1008);
    }
    SECTION("len 1, hishf 0, loshf 0, mask 0x55") {
        constexpr unsigned len = 1;
        byte_t byte = 0xff;
        flags_view<len, 0, 0, 0x55> flags{&byte};
        REQUIRE(flags == 0x55);
        flags = 0;
        REQUIRE(byte == 0xaa);
        REQUIRE(flags.uint64() == 0);
        flags = 0x41;
        REQUIRE(byte == 0xeb);
        REQUIRE(flags == 0x41);
    }
}
