#pragma once

#include <format>
#include <iterator>
#include <ostream>
#include <utility>

namespace net {

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

} // namespace net
