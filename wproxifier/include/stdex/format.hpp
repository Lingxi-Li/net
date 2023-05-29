#pragma once

#include <cstddef>

#include <format>
#include <iterator>
#include <ostream>
#include <utility>

#define STDEX_DEFINE_ENUM_FORMATTER(ns, enum_type, ...)                      \
    template <>                                                              \
    struct ::std::formatter<ns::enum_type>: formatter<char const*> {         \
        constexpr static char const* strs[] = { __VA_ARGS__ };               \
        auto format(ns::enum_type value, format_context& ctx) const {        \
            auto idx = size_t(value);                                        \
            return formatter<char const*>::format(strs[idx], ctx);           \
        }                                                                    \
    };                                                                       \
    namespace ns {                                                           \
        inline ::std::ostream& operator<<(::std::ostream& os, enum_type v) { \
            return ::stdex::format_to(os, "{}", v);                          \
        }                                                                    \
    }

namespace stdex {

struct naive_formatter {
    constexpr auto parse(std::format_parse_context& ctx) const noexcept {
        return ctx.begin();
    }
};

template <typename... Ts>
std::ostream& format_to(std::ostream& os, std::format_string<Ts...> fmt, Ts&&... args) {
    std::format_to(std::ostream_iterator<char>(os), fmt, std::forward<Ts>(args)...);
    return os;
}

} // namespace stdex
