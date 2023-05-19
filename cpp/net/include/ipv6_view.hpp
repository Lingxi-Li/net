#pragma once

#include "field_view.hpp"
#include "utility.hpp"

#include <cassert>

#include <type_traits>

namespace net {

template <typename T>
struct ipv6_addr_view_t {
    static_assert(std::is_same_v<T, byte_t> || std::is_same_v<T, byte_t const>);

    T* data{};

    T& operator[](unsigned i) const noexcept {
        assert(i <= 15);
        return data[i];
    }
};
static_assert(std::is_aggregate_v<ipv6_addr_view_t<byte_t>>);

using ipv6_addr_view = ipv6_addr_view_t<byte_t>;
using ipv6_addr_const_view = ipv6_addr_view_t<byte_t const>;

template <typename T> inline
std::ostream& operator<<(std::ostream& os, ipv6_addr_view_t<T> view) {
    return format_to(os, "{}", view);
}

} // namespace net

template <typename T>
struct std::formatter<net::ipv6_addr_view_t<T>>: formatter<string> {
    auto format(net::ipv6_addr_view_t<T> view, format_context& ctx) {
        auto str = std::format("{:x}:{:x}:{:x}:{:x}:{:x}:{:x}:{:x}:{:x}"
            , unsigned(view[ 0]) << 8 | view[ 1]
            , unsigned(view[ 2]) << 8 | view[ 3]
            , unsigned(view[ 4]) << 8 | view[ 5]
            , unsigned(view[ 6]) << 8 | view[ 7]
            , unsigned(view[ 8]) << 8 | view[ 9]
            , unsigned(view[10]) << 8 | view[11]
            , unsigned(view[12]) << 8 | view[13]
            , unsigned(view[14]) << 8 | view[15]
        );
        return formatter<string>::format(str, ctx);
    }
};

namespace net {

template <typename T>
struct ipv6_view_t {
    static_assert(std::is_same_v<T, byte_t> || std::is_same_v<T, byte_t const>);

    T* data{};

    uint_view_t<T, 1, 0, 4> version() const noexcept {
        return { data };
    }

    uint_view_t<T, 2, 4, 6> diff_serv() const noexcept {
        return { data };
    }

    uint_view_t<T, 1, 2, 4> ecn() const noexcept {
        return { data + 1 };
    }

    uint_view_t<T, 3, 4> flow() const noexcept {
        return { data + 1 };
    }

    uint_view_t<T, 2> payload_len() const noexcept {
        return { data + 4 };
    }

    uint_view_t<T, 1> next_header() const noexcept {
        return { data + 6 };
    }

    uint_view_t<T, 1> hop_limit() const noexcept {
        return { data + 7 };
    }

    ipv6_addr_view_t<T> src_addr() const noexcept {
        return { data + 8 };
    }

    ipv6_addr_view_t<T> dst_addr() const noexcept {
        return { data + 24 };
    }

    T* next_header_data() const noexcept {
        return data + 40;
    }
};
static_assert(std::is_aggregate_v<ipv6_view_t<byte_t>>);

using ipv6_view = ipv6_view_t<byte_t>;
using ipv6_const_view = ipv6_view_t<byte_t const>;

} // namespace net
