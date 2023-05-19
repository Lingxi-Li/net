#pragma once

#include <format>
#include <iterator>
#include <ostream>
#include <utility>

namespace net {

template <typename... Ts>
std::ostream& format_to(std::ostream& os, std::format_string<Ts...> fmt, Ts&&... args) {
    std::format_to(std::ostream_iterator<char>(os), fmt, std::forward<Ts>(args)...);
    return os;
}

} // namespace net
