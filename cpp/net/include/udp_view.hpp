#pragma once

#include "field_view.hpp"

#include <type_traits>

namespace net {

template <typename T>
struct udp_view_t {
    static_assert(std::is_same_v<T, byte_t> || std::is_same_v<T, byte_t const>);

    T* data;

    uint_view_t<T, 2> src_port() const noexcept {
        return { data };
    }

    uint_view_t<T, 2> dst_port() const noexcept {
        return { data + 2 };
    }

    uint_view_t<T, 2> len() const noexcept {
        return { data + 4 };
    }

    uint_view_t<T, 2> checksum() const noexcept {
        return { data + 6 };
    }

    T* payload() const noexcept {
        return { data + 8 };
    }
};
static_assert(std::is_aggregate_v<udp_view_t<byte_t>>);

using udp_view = udp_view_t<byte_t>;
using udp_const_view = udp_view_t<byte_t const>;

} // namespace net
