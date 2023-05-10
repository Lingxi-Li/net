#pragma once

#include <cstdint>
#include <cstring>

#include <bit>
#include <ostream>
#include <format>

namespace net {

using byte_t = std::uint8_t;
using uint_t = std::uint64_t;

static_assert(
    std::endian::native == std::endian::little ||
    std::endian::native == std::endian::big
);

// Network byte order (big-endian)
// 
//  +------------+------+------------+
//  | hishf bits | uint | loshf bits |
//  ^------------+------+------------^
//  |                                |
// data                        (data + len) in byte

template <unsigned len, unsigned hishf = 0, unsigned loshf = 0>
struct uint_view {
    byte_t* data;

    operator uint_t() const noexcept {
        if constexpr (std::endian::native == std::endian::big) {
            return parse_uint_be();
        }
        else {
            return parse_uint_le();
        }
    }

    void operator=(uint_t uint) const noexcept {
        if constexpr (std::endian::native == std::endian::big) {
            write_uint_be(uint);
        }
        else {
            write_uint_le(uint);
        }
    }

private:
    static_assert(2 <= len && len <= sizeof(uint_t));
    static_assert(hishf <= 7 && loshf <= 7);
    constexpr static unsigned hi_uint_mask = (unsigned(-1) << hishf & 0xff) >> hishf;
    constexpr static unsigned hi_data_mask = ~hi_uint_mask & 0xff;
    constexpr static unsigned lo_uint_mask = (unsigned(-1) & 0xff) >> loshf << loshf;
    constexpr static unsigned lo_data_mask = ~lo_uint_mask & 0xff;

    uint_t parse_uint_le() const noexcept {
        uint_t uint{};
        auto dst = (byte_t*)(&uint);
        auto src = data + (len - 1);
        do {
            *dst++ = *src--;
        }
        while (src > data);
        *dst = *src & hi_uint_mask;
        return uint >> loshf;
    }

    uint_t parse_uint_be() const noexcept {
        uint_t uint{};
        auto dst = (byte_t*)(&uint) + (sizeof(uint_t) - len);
        auto src = data;
        *dst++ = *src++ & hi_uint_mask;
        std::memcpy(dst, src, len - 1);
        return uint >> loshf;
    }

    void write_uint_le(uint_t uint) const noexcept {
        uint <<= loshf;
        auto dst = data + (len - 1);
        auto src = (byte_t const*)(&uint);
        *dst = (*dst & lo_data_mask) | (*src & lo_uint_mask);
        --dst;
        ++src;
        while (dst > data) {
            *dst-- = *src++;
        }
        *dst = (*dst & hi_data_mask) | (*src & hi_uint_mask);
    }

    void write_uint_be(uint_t uint) const noexcept {
        uint <<= loshf;
        auto dst = data;
        auto src = (byte_t const*)(&uint) + (sizeof(uint_t) - len);
        *dst = (*dst & hi_data_mask) | (*src & hi_uint_mask);
        std::memcpy(++dst, ++src, len - 2);
        dst += len - 2;
        src += len - 2;
        *dst = (*dst & lo_data_mask) | (*src & lo_uint_mask);
    }
};

template <unsigned hishf, unsigned loshf>
struct uint_view<1, hishf, loshf> {
    byte_t* data;

    operator uint_t() const noexcept {
        return (*data & uint_mask) >> loshf;
    }

    void operator=(uint_t uint) const noexcept {
        uint <<= loshf;
        *data = (*data & data_mask) | (uint & uint_mask);
    }

private:
    static_assert(hishf <= 7 && loshf <= 7);
    static_assert(hishf + loshf <= 7);
    constexpr static unsigned uint_mask = (unsigned(-1) << hishf & 0xff) >> hishf >> loshf << loshf;
    constexpr static unsigned data_mask = ~uint_mask & 0xff;
};

//     _ _ _ _ _ _ _ _
// pos 0 1 2 3 4 5 6 7

template <unsigned pos>
using bit_view = uint_view<1, pos, 7 - pos>;

template <unsigned len, unsigned hishf, unsigned loshf> inline
std::ostream& operator<<(std::ostream& os, uint_view<len, hishf, loshf> view) {
    return os << uint_t(view);
}

} // namespace net

template <unsigned len, unsigned hishf, unsigned loshf>
struct std::formatter<net::uint_view<len, hishf, loshf>>: formatter<net::uint_t> {
    auto format(net::uint_view<len, hishf, loshf> view, format_context& ctx) {
        return formatter<net::uint_t>::format(view, ctx);
    }
};
