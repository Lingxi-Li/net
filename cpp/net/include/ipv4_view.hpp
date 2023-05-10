#pragma once

#include "field_view.hpp"

#include <cassert>

#include <iterator>

namespace net {

struct ipv4_addr_view {
    byte_t* data;

    byte_t& operator[](unsigned i) const noexcept {
        assert(i <= 3);
        return data[i];
    }
};

inline std::ostream& operator<<(std::ostream& os, ipv4_addr_view view) {
    std::format_to(std::ostream_iterator<char>(os), "{}", view);
    return os;
}

} // namespace net

template <>
struct std::formatter<net::ipv4_addr_view> {
    constexpr auto parse(format_parse_context& ctx) const {
        return ctx.begin();
    }

    auto format(net::ipv4_addr_view view, format_context& ctx) const {
        return format_to(ctx.out(), "{}.{}.{}.{}"
            , unsigned(view[0])
            , unsigned(view[1])
            , unsigned(view[2])
            , unsigned(view[3])
        );
    }
};

namespace net {

struct ipv4_view {
    byte_t* data;
    
    uint_view<1, 0, 4> version() const noexcept {
        return { data };
    }

    uint_view<1, 4, 0> header_len() const noexcept {
        return { data };
    }

    uint_view<1, 0, 2> diff_serv() const noexcept {
        return { data + 1 };
    }

    uint_view<1, 6, 0> ecn() const noexcept {
        return { data + 1 };
    }

    uint_view<2> total_len() const noexcept {
        return { data + 2 };
    }

    uint_view<2> id() const noexcept {
        return { data + 4 };
    }

    bit_view<1> no_frag() const noexcept {
        return { data + 6 };
    }

    bit_view<2> more_frag() const noexcept {
        return { data + 6 };
    }

    uint_view<2, 3> frag_offset() const noexcept {
        return { data + 6 };
    }

    uint_view<1> ttl() const noexcept {
        return { data + 8 };
    }

    uint_view<1> protocol() const noexcept {
        return { data + 9 };
    }

    uint_view<2> header_checksum() const noexcept {
        return { data + 10 };
    }

    ipv4_addr_view src_addr() const noexcept {
        return { data + 12 };
    }

    ipv4_addr_view dst_addr() const noexcept {
        return { data + 16 };
    }

    byte_t* options() const noexcept {
        return data + 20;
    }

    byte_t* payload() const noexcept {
        return data + header_len() * 4;
    }
};

//inline std::ostream& operator<<(std::ostream& os, ipv4_view ipv4) {
//    std::format_to(std::ostream_iterator<char>(os),
//        "   version {}\n"
//        "header_len {}\n"
//        " total_len {}\n"
//        "  protocol {}\n"
//        "  src_addr {}\n"
//        "  dst_addr {}\n"
//        , ipv4.version()
//        , ipv4.header_len()
//        , ipv4.total_len()
//        , ipv4.protocol()
//        , ipv4.src_addr()
//        , ipv4.dst_addr()
//    );
//    return os;
//}

} // namespace net
