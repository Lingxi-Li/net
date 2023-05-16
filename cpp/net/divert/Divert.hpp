#pragma once

#include "windivert.h"

#include <system_error>
#include <type_traits>
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
struct std::formatter<dvt::Api>: formatter<char const*> {
    auto format(dvt::Api api, format_context& ctx) {
        return formatter<char const*>::format(StrRep[DWORD(api)], ctx);
    }

private:
    constexpr static char const* StrRep[] = {
        "WinDivertOpen",
        "WinDivertClose"
    };
};

namespace dvt {

struct Error {
    Api const FailedApi;
    DWORD const ErrorCode{GetLastError()};
};
static_assert(std::is_aggregate_v<Error>);

inline std::ostream& operator<<(std::ostream& os, Error err) {
    std::format_to(std::ostream_iterator<char>(os), "{}", err);
    return os;
}

} // namespace dvt

template <>
struct std::formatter<dvt::Error>: formatter<string> {
    auto format(dvt::Error err, format_context& ctx) {
        auto str = std::format("{}, {}: {}"
            , err.FailedApi
            , err.ErrorCode
            , std::system_category().message(err.ErrorCode)
        );
        return formatter<string>::format(str, ctx);
    }
};
