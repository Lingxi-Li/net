#pragma once

#include "field_view.hpp"

#include <type_traits>

namespace net {

template <typename T>
struct tcp_view_t {
    static_assert(std::is_same_v<T, byte_t> || std::is_same_v<T, byte_t const>);

    T* data;

    uint_view_t<T, 2> src_port() const noexcept {
        return { data };
    }

    uint_view_t<T, 2> dst_port() const noexcept {
        return { data + 2 };
    }

    uint_view_t<T, 4> seq_num() const noexcept {
        return { data + 4 };
    }

    uint_view_t<T, 4> ack_num() const noexcept {
        return { data + 8 };
    }

    uint_view_t<T, 1, 0, 4> header_len() const noexcept {
        return { data + 12 };
    }

    bit_view_t<T, 3> ack() const noexcept {
        return { data + 13 };
    }

    bit_view_t<T, 4> psh() const noexcept {
        return { data + 13 };
    }

    bit_view_t<T, 5> rst() const noexcept {
        return { data + 13 };
    }

    bit_view_t<T, 6> syn() const noexcept {
        return { data + 13 };
    }

    bit_view_t<T, 7> fin() const noexcept {
        return { data + 13 };
    }

    uint_view_t<T, 2> window_size() const noexcept {
        return { data + 14 };
    }

    uint_view_t<T, 2> checksum() const noexcept {
        return { data + 16 };
    }

    T* payload() const noexcept {
        return data + header_len() * 4;
    }
};
static_assert(std::is_aggregate_v<tcp_view_t<byte_t>>);

using tcp_view = tcp_view_t<byte_t>;
using tcp_const_view = tcp_view_t<byte_t const>;

} // namespace net
