#pragma once

#include "field_view.hpp"

#include <cassert>

#include <type_traits>
#include <iterator>

namespace net {

template <typename T>
struct ipv6_addr_view_t {
    static_assert(std::is_same_v<T, byte_t> || std::is_same_v<T, byte_t const>);

    T* data;

    T& operator[](unsigned i) const noexcept {
        assert(i <= 15);
        return data[i];
    }
};

using ipv6_addr_view = ipv6_addr_view_t<byte_t>;
using ipv6_addr_const_view = ipv6_addr_view_t<byte_t const>;

template <typename T> inline
std::ostream& operator<<(std::ostream& os, ipv6_addr_view_t<T> view) {
    std::format_to(std::ostream_iterator<char>(os), "{}", view);
    return os;
}

} // namespace net

template <typename T>
struct std::formatter<net::ipv6_addr_view_t<T>> {
    constexpr auto parse(format_parse_context& ctx) const {
        return ctx.begin();
    }

    auto format(net::ipv6_addr_view_t<T> view, format_context& ctx) const {
        return format_to(ctx.out(), "{:x}:{:x}:{:x}:{:x}:{:x}:{:x}:{:x}:{:x}"
            , std::uint64_t(view[ 0]) << 8 | view[ 1]
            , std::uint64_t(view[ 2]) << 8 | view[ 3]
            , std::uint64_t(view[ 4]) << 8 | view[ 5]
            , std::uint64_t(view[ 6]) << 8 | view[ 7]
            , std::uint64_t(view[ 8]) << 8 | view[ 9]
            , std::uint64_t(view[10]) << 8 | view[11]
            , std::uint64_t(view[12]) << 8 | view[13]
            , std::uint64_t(view[14]) << 8 | view[15]
        );
    }
};
