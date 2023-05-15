#pragma once

#include "windivert.h"

#include <ostream>
#include <iterator>
#include <format>

namespace dvt {

enum struct Api: DWORD {
    WinDivertOpen,
    WinDivertClose
};

inline std::ostream& operator<<(std::ostream& os, Api api) {
    std::format_to(std::ostream_iterator<char>(os), "{}", api);
    return os;
}

} // namespace dvt

template <>
struct std::formatter<dvt::Api> {
    constexpr auto parse(format_parse_context& ctx) const {
        return ctx.begin();
    }

    auto format(dvt::Api api, format_context& ctx) const {
        return format_to(ctx.out(), "{}", StrRep[DWORD(api)]);
    }

private:
    constexpr static char const* StrRep[] = {
        "WinDivertOpen",
        "WinDivertClose"
    };
};
