#pragma once

#include "field_view.hpp"

#include <stdex/format.hpp>

#include <cassert>

#include <type_traits>

namespace net {

template <typename T>
struct ipv4_addr_view_t {
    static_assert(std::is_same_v<T, byte_t> || std::is_same_v<T, byte_t const>);

    T* data{};

    T& operator[](unsigned i) const noexcept {
        assert(i <= 3);
        return data[i];
    }
};
static_assert(std::is_aggregate_v<ipv4_addr_view_t<byte_t>>);

using ipv4_addr_view = ipv4_addr_view_t<byte_t>;
using ipv4_addr_const_view = ipv4_addr_view_t<byte_t const>;

template <typename T>
std::ostream& operator<<(std::ostream& os, ipv4_addr_view_t<T> view) {
    return stdex::format_to(os, "{}", view);
}

} // namespace net

template <typename T>
struct std::formatter<net::ipv4_addr_view_t<T>>: stdex::naive_formatter {
    auto format(net::ipv4_addr_view_t<T> view, format_context& ctx) const {
        return format_to(ctx.out(), "{}.{}.{}.{}"
            , unsigned(view[0])
            , unsigned(view[1])
            , unsigned(view[2])
            , unsigned(view[3])
        );
    }
};

namespace net {

template <typename T>
struct ipv4_view_t {
    static_assert(std::is_same_v<T, byte_t> || std::is_same_v<T, byte_t const>);

    T* data{};
    
    uint_view_t<T, 1, 0, 4> version() const noexcept {
        return { data };
    }

    uint_view_t<T, 1, 4, 0> header_len() const noexcept {
        return { data };
    }

    uint_view_t<T, 1, 0, 2> diff_serv() const noexcept {
        return { data + 1 };
    }

    uint_view_t<T, 1, 6, 0> ecn() const noexcept {
        return { data + 1 };
    }

    uint_view_t<T, 2> total_len() const noexcept {
        return { data + 2 };
    }

    uint_view_t<T, 2> id() const noexcept {
        return { data + 4 };
    }

    bit_view_t<T, 1> no_frag() const noexcept {
        return { data + 6 };
    }

    bit_view_t<T, 2> more_frag() const noexcept {
        return { data + 6 };
    }

    uint_view_t<T, 2, 3> frag_offset() const noexcept {
        return { data + 6 };
    }

    uint_view_t<T, 1> ttl() const noexcept {
        return { data + 8 };
    }

    uint_view_t<T, 1> protocol() const noexcept {
        return { data + 9 };
    }

    uint_view_t<T, 2> header_checksum() const noexcept {
        return { data + 10 };
    }

    ipv4_addr_view_t<T> src_addr() const noexcept {
        return { data + 12 };
    }

    ipv4_addr_view_t<T> dst_addr() const noexcept {
        return { data + 16 };
    }

    T* options() const noexcept {
        return data + 20;
    }

    T* payload() const noexcept {
        return data + header_len() * 4;
    }

    T* end() const noexcept {
        return data + total_len();
    }
};
static_assert(std::is_aggregate_v<ipv4_view_t<byte_t>>);

using ipv4_view = ipv4_view_t<byte_t>;
using ipv4_const_view = ipv4_view_t<byte_t const>;

} // namespace net
