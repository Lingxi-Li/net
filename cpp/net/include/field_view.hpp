#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

#include <bitset>
#include <ostream>
#include <format>

namespace net {

using byte_t = std::uint8_t;
using uint_t = std::uint64_t;

namespace impl {

inline uint_t parse_uint_little_endian(byte_t const* data, std::uint8_t len) noexcept {
    uint_t uint{};
    auto dst = reinterpret_cast<byte_t*>(&uint);
    auto src = data + (len - 1);
    while (true) {
        *dst = *src;
        if (src == data) break;
        --src;
        ++dst;
    }
    return uint;
}

inline uint_t parse_uint_big_endian(byte_t const* data, std::uint8_t len) noexcept {
    uint_t uint{};
    std::memcpy(
        reinterpret_cast<byte_t*>(&uint) + (sizeof(uint_t) - len),
        data,
        len
    );
    return uint;
}

inline uint_t (* parse_uint)(byte_t const* data, std::uint8_t len) noexcept;

inline void write_uint_little_endian(byte_t* data, std::uint8_t len, uint_t uint) noexcept {
    auto src = reinterpret_cast<byte_t const*>(&uint);
    auto dst = data + (len - 1);
    while (true) {
        *dst = *src;
        if (dst == data) break;
        ++src;
        --dst;
    }
}

inline void write_uint_big_endian(byte_t* data, std::uint8_t len, uint_t uint) noexcept {
    auto src = reinterpret_cast<byte_t const*>(&uint);
    std::memcpy(data, src + (sizeof(uint_t) - len), len);
}

inline void (* write_uint)(byte_t* data, std::uint8_t len, uint_t uint) noexcept;

inline struct init_parse_write_uint {
    init_parse_write_uint() noexcept {
        auto v = 1u;
        if (*reinterpret_cast<byte_t const*>(&v)) {
            parse_uint = parse_uint_little_endian;
            write_uint = write_uint_little_endian;
        }
        else {
            parse_uint = parse_uint_big_endian;
            write_uint = write_uint_big_endian;
        }
    }
}
init_parse_write_uint;

} // namespace impl

template <std::uint8_t len>
struct uint_view {
    byte_t* data;

    operator uint_t() const noexcept {
        return impl::parse_uint(data, len);
    }

    void operator=(uint_t uint) const noexcept {
        impl::write_uint(data, len, uint);
    }
};

template <std::uint8_t len> inline
std::ostream& operator<<(std::ostream& os, uint_view<len> uintv) {
    return os << uint_t(uintv);
}

} // namespace net

template <std::uint8_t len>
struct std::formatter<net::uint_view<len>>: formatter<net::uint_t> {
    auto format(net::uint_view<len> view, format_context& ctx) {
        return formatter<net::uint_t>::format(net::uint_t(view), ctx);
    }
};

////////////////////////////////////////

namespace net {
namespace impl {

template <byte_t mask>
constexpr std::uint8_t num_trailing_zeros() noexcept {
    static_assert(mask);
    std::uint8_t num{};
    while (!((mask >> num) & 1u)) ++num;
    return num;
}

} // namespace impl

template <byte_t mask, std::uint8_t shf = impl::num_trailing_zeros<mask>()>
struct uint_view_bit {
    byte_t* data;

    operator uint_t() const noexcept {
        return (*data & mask) >> shf;
    }

    void operator=(uint_t uint) const noexcept {
        byte_t v = (uint << shf) & mask;
        *data = (*data & ~mask) | v;
    }
};

template <byte_t mask> inline
std::ostream& operator<<(std::ostream& os, uint_view_bit<mask> uintv) {
    return os << uint_t(uintv);
}

} // namespace net

template <net::byte_t mask>
struct std::formatter<net::uint_view_bit<mask>>: formatter<net::uint_t> {
    auto format(net::uint_view_bit<mask> view, format_context& ctx) {
        return formatter<net::uint_t>::format(net::uint_t(view), ctx);
    }
};

////////////////////////////////////////

namespace net {

template <byte_t mask>
struct bit_view {
    static_assert(
        mask == 0x01u || mask == 0x02u || mask == 0x04u || mask == 0x08u ||
        mask == 0x10u || mask == 0x20u || mask == 0x40u || mask == 0x80u
    );
    byte_t* data;

    operator bool() const noexcept {
        return *data & mask;
    }

    void operator=(bool flag) const noexcept {
        if (flag) {
            *data |= mask;
        }
        else {
            *data &= ~mask;
        }
    }
};

template <byte_t mask> inline
std::ostream& operator<<(std::ostream& os, bit_view<mask> view) {
    return os << (view ? '1' : '0');
}

} // namespace net

template <net::byte_t mask>
struct std::formatter<net::bit_view<mask>> {
    constexpr auto parse(format_parse_context& ctx) const {
        return ctx.begin();
    }

    auto format(net::bit_view<mask> view, format_context& ctx) const {
        return view
            ? format_to(ctx.out(), "1")
            : format_to(ctx.out(), "0");
    }
};
