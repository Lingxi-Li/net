#pragma once

#include <stdex/format.hpp>

#include <cstdint>

#include <type_traits>

namespace net {

// `std::byte` is not used due to potential overhead of back-and-forth
// (integer) promotion and truncation of intermediate results.
using byte_t = std::uint8_t;
using uint_t = std::uint64_t;

// most sig. bit
//       v
//       | most sig. byte |                | least sig. byte |
//       +----------------+----------------+-----------------+
//       | hishf bits |   |      ...       |    | loshf bits |
//       +----------------+----------------+-----------------+
//                    |<--------- uint -------->|             
//       ^                ^                ^                 ^
//      data          data + 1       data + len - 1     data + len

template <typename T, unsigned len, unsigned hishf = 0, unsigned loshf = 0>
struct uint_view_t {
    static_assert(std::is_same_v<T, byte_t> || std::is_same_v<T, byte_t const>);
    static_assert(2 <= len && len <= sizeof(uint_t));
    static_assert(hishf <= 7 && loshf <= 7);

    T* data{};

    operator uint_t() const noexcept {
        auto p = data;
        auto end = data + len;
        uint_t uint = *p++ & hi_uint_mask;
        do {
            uint = uint << 8 | *p++;
        }
        while (p < end);
        return uint >> loshf;
    }

    void operator=(uint_t uint) const noexcept {
        uint <<= loshf;
        auto p = data + (len - 1);
        *p = (*p & lo_data_mask) | unsigned(uint);
        --p;
        uint >>= 8;
        while (p > data) {
            *p-- = uint & 0xffu;
            uint >>= 8;
        }
        *p = (*p & hi_data_mask) | (uint & hi_uint_mask);
    }

    uint_t uint() const noexcept {
        return operator uint_t();
    }

private:
    constexpr static unsigned hi_uint_mask = (unsigned(-1) << hishf & 0xffu) >> hishf;
    constexpr static unsigned hi_data_mask = ~hi_uint_mask & 0xffu;
    constexpr static unsigned lo_uint_mask = (unsigned(-1) & 0xffu) >> loshf << loshf;
    constexpr static unsigned lo_data_mask = ~lo_uint_mask & 0xffu;
};
static_assert(std::is_aggregate_v<uint_view_t<byte_t, 2>>);

template <typename T, unsigned hishf, unsigned loshf>
struct uint_view_t<T, 1, hishf, loshf> {
    static_assert(std::is_same_v<T, byte_t> || std::is_same_v<T, byte_t const>);
    static_assert(hishf + loshf <= 7);

    T* data{};

    operator uint_t() const noexcept {
        return (*data & uint_mask) >> loshf;
    }

    void operator=(uint_t uint) const noexcept {
        uint <<= loshf;
        *data = (*data & data_mask) | (uint & uint_mask);
    }

    uint_t uint() const noexcept {
        return operator uint_t();
    }

private:
    constexpr static unsigned uint_mask = (unsigned(-1) << hishf & 0xffu) >> hishf >> loshf << loshf;
    constexpr static unsigned data_mask = ~uint_mask & 0xffu;
};
static_assert(std::is_aggregate_v<uint_view_t<byte_t, 1>>);

template <unsigned len, unsigned hishf = 0, unsigned loshf = 0>
using uint_view = uint_view_t<byte_t, len, hishf, loshf>;

template <unsigned len, unsigned hishf = 0, unsigned loshf = 0>
using uint_const_view = uint_view_t<byte_t const, len, hishf, loshf>;

// pos:  0   1   2   3   4   5   6   7
//       ^                           ^
// most sig. bit              least sig. bit

template <typename T, unsigned pos>
using bit_view_t = uint_view_t<T, 1, pos, 7 - pos>;

template <unsigned pos>
using bit_view = bit_view_t<byte_t, pos>;

template <unsigned pos>
using bit_const_view = bit_view_t<byte_t const, pos>;

template <typename T, unsigned len, unsigned hishf, unsigned loshf>
std::ostream& operator<<(std::ostream& os, uint_view_t<T, len, hishf, loshf> view) {
    return os << uint_t(view);
}

template <typename T, unsigned len, unsigned hishf, unsigned loshf, std::uint64_t mask>
struct flags_view_t {
    static_assert(mask);
    static_assert(mask < (std::uint64_t(1) << 1 << (len * 8 - hishf - loshf - 1)) - 1, "Use 'uint_view' for better perf");
    
    uint_view_t<T, len, hishf, loshf> data;

    operator std::uint64_t() const noexcept {
        return data.uint() & mask;
    }

    void operator=(std::uint64_t flags) const noexcept {
        data = data.uint() & ~uint_t(mask) | flags;
    }

    std::uint64_t uint64() const noexcept {
        return *this;
    }
};
static_assert(std::is_aggregate_v<flags_view_t<byte_t, 1, 0, 0, 0x55>>);

template <unsigned len, unsigned hishf, unsigned loshf, std::uint64_t mask>
using flags_view = flags_view_t<byte_t, len, hishf, loshf, mask>;

template <unsigned len, unsigned hishf, unsigned loshf, std::uint64_t mask>
using flags_const_view = flags_view_t<byte_t const, len, hishf, loshf, mask>;

template <typename T, unsigned len, unsigned hishf, unsigned loshf, std::uint64_t mask>
std::ostream& operator<<(std::ostream& os, flags_view_t<T, len, hishf, loshf, mask> view) {
    return stdex::format_to(os, "{}", view);
}

} // namespace net

template <typename T, unsigned len, unsigned hishf, unsigned loshf>
struct std::formatter<net::uint_view_t<T, len, hishf, loshf>>: formatter<net::uint_t> {
    auto format(net::uint_view_t<T, len, hishf, loshf> view, format_context& ctx) {
        return formatter<net::uint_t>::format(view, ctx);
    }
};

template <typename T, unsigned len, unsigned hishf, unsigned loshf, std::uint64_t mask>
struct std::formatter<net::flags_view_t<T, len, hishf, loshf, mask>>: stdex::naive_formatter {
    auto format(net::flags_view_t<T, len, hishf, loshf, mask> view, format_context& ctx) const {
        return format_to(ctx.out(), "{:#x}", view.uint64());
    }
};
