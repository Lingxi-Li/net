#pragma once

#include "field_view.hpp"

#include <iterator>

namespace net {

struct ipv4_addr_view {
    byte_t* data;

    byte_t& operator[](std::size_t i) const noexcept {
        return data[i];
    }
};

inline std::ostream& operator<<(std::ostream& os, ipv4_addr_view addrv) {
    std::format_to(std::ostream_iterator<char>(os), "{}", addrv);
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
            , net::uint_t(view[0])
            , net::uint_t(view[1])
            , net::uint_t(view[2])
            , net::uint_t(view[3])
        );
    }
};

namespace net {

////////////////////////////////////////

struct ipv4_view {
    byte_t* data;
    
    uint_view_bit<0xf0> version() const noexcept {
        return { data };
    }

    uint_view_bit<0x0f> header_len() const noexcept {
        return { data };
    }

    uint_view<2> total_len() const noexcept {
        return { data + 2 };
    }

    uint_view<1> protocol() const noexcept {
        return { data + 9 };
    }

    ipv4_addr_view src_addr() const noexcept {
        return { data + 12 };
    }

    ipv4_addr_view dst_addr() const noexcept {
        return { data + 16 };
    }
};

inline std::ostream& operator<<(std::ostream& os, ipv4_view ipv4) {
    std::format_to(std::ostream_iterator<char>(os),
        "   version {}\n"
        "header_len {}\n"
        " total_len {}\n"
        "  protocol {}\n"
        "  src_addr {}\n"
        "  dst_addr {}\n"
        , ipv4.version()
        , ipv4.header_len()
        , ipv4.total_len()
        , ipv4.protocol()
        , ipv4.src_addr()
        , ipv4.dst_addr()
    );
    return os;
}

} // namespace net
