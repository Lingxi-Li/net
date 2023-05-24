#pragma once

#include <windivert.h>

#undef min
#undef max

#include <stdex/format.hpp>

#include <format>
#include <iterator>
#include <ostream>
#include <system_error>
#include <type_traits>
#include <vector>

namespace dvt {

enum struct Api: DWORD {
    WinDivertOpen,
    WinDivertClose,
    WinDivertShutdown,
    WinDivertRecv,
    WinDivertSend
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
        "WinDivertClose",
        "WinDivertShutdown",
        "WinDivertRecv",
        "WinDivertSend"
    };
};

namespace dvt {

struct Error: std::exception {
    Error(Api api)
        : FailedApi(api)
        , ErrorCode(GetLastError())
        , Message(std::format("{}, 0x{:08X}: {}", FailedApi, ErrorCode, std::system_category().message(ErrorCode))) {
    }

    char const* what() const noexcept override {
        return Message.c_str();
    }

    Api FailedApi;
    DWORD ErrorCode;
    std::string Message;
};

inline std::ostream& operator<<(std::ostream& os, Error const& err) {
    return os << err.Message;
}

} // namespace dvt

template <>
struct std::formatter<dvt::Error>: formatter<string> {
    auto format(dvt::Error const& err, format_context& ctx) {
        return formatter<string>::format(err.Message, ctx);
    }
};

namespace dvt {

static_assert(std::is_aggregate_v<WINDIVERT_ADDRESS>);

using ByteVec = std::vector<UINT8>;
using AddrVec = std::vector<WINDIVERT_ADDRESS>;

inline void Check(BOOL res, Api api) {
    if (!res) throw Error(api);
}

inline void Check(HANDLE handle, Api api) {
    if (handle == INVALID_HANDLE_VALUE) throw Error(api);
}

class Handle {
public:
    Handle() noexcept = default;

    Handle(char const* filter, WINDIVERT_LAYER layer, INT16 priority, UINT64 flags)
        : handle(WinDivertOpen(filter, layer, priority, flags)) {
        if (handle == INVALID_HANDLE_VALUE) throw Error(Api::WinDivertOpen);
    }

    Handle(Handle const&) = delete;

    Handle(Handle&& other) noexcept
        : handle(std::exchange(other.handle, INVALID_HANDLE_VALUE)) {
    }
    
    ~Handle() {
        if (handle == INVALID_HANDLE_VALUE) return;
        WinDivertClose(handle);
        handle = INVALID_HANDLE_VALUE;
    }

    Handle& operator=(Handle const&) = delete;

    Handle& operator=(Handle&& other) noexcept {
        if (this != &other) {
            if (handle != INVALID_HANDLE_VALUE) {
                WinDivertClose(handle);
            }
            handle = std::exchange(other.handle, INVALID_HANDLE_VALUE);
        }
        return *this;
    }

    operator bool() const noexcept {
        return handle != INVALID_HANDLE_VALUE;
    }

    explicit operator HANDLE() const noexcept {
        return handle;
    }

    // close existing before opening new in case the same `priority` is used
    void Open(char const* filter, WINDIVERT_LAYER layer, INT16 priority, UINT64 flags) {
        if (handle != INVALID_HANDLE_VALUE) {
            WinDivertClose(handle);
        }
        handle = WinDivertOpen(filter, layer, priority, flags);
        if (handle == INVALID_HANDLE_VALUE) throw Error(Api::WinDivertOpen);
    }

    void Close() {
        Check(WinDivertClose(handle), Api::WinDivertClose);
        handle = INVALID_HANDLE_VALUE;
    }

    // can be forced out of wait with `Shutdown`
    BOOL Recv(
        void* packets,
        UINT packetsSize,
        UINT* packetsRecvedSize,
        UINT64 flags,
        WINDIVERT_ADDRESS* addrs,
        UINT* addrsSize) const noexcept
    {
        return WinDivertRecvEx(handle, packets, packetsSize, packetsRecvedSize, flags, addrs, addrsSize, NULL);
    }

    BOOL Send(
        void const* packets,
        UINT packetsSize,
        UINT* packetsSentSize,
        UINT64 flags,
        WINDIVERT_ADDRESS const* addrs,
        UINT addrsSize) const noexcept
    {
        return WinDivertSendEx(handle, packets, packetsSize, packetsSentSize, flags, addrs, addrsSize, NULL);
    }

    void Shutdown(WINDIVERT_SHUTDOWN how = WINDIVERT_SHUTDOWN_RECV) const {
        auto res = WinDivertShutdown(handle, how);
        Check(res, Api::WinDivertShutdown);
    }

private:
    HANDLE handle{INVALID_HANDLE_VALUE};
};

inline std::ostream& operator<<(std::ostream& os, Handle const& hdl) {
    return os << HANDLE(hdl);
}

} // namespace dvt

template <>
struct std::formatter<dvt::Handle>: formatter<void*> {
    auto format(dvt::Handle const& hdl, format_context& ctx) {
        return formatter<void*>::format(HANDLE(hdl), ctx);
    }
};

//////////////////// output ////////////////////

template <>
struct std::formatter<WINDIVERT_LAYER>: formatter<char const*> {
    auto format(WINDIVERT_LAYER layer, format_context& ctx) {
        return formatter<char const*>::format(StrRep[layer], ctx);
    }

private:
    constexpr static char const* StrRep[] = {
        "NETWORK",
        "NETWORK_FORWARD",
        "FLOW",
        "SOCKET",
        "REFLECT"
    };
};

inline std::ostream& operator<<(std::ostream& os, WINDIVERT_LAYER layer) {
    return stdex::format_to(os, "{}", layer);
}

inline std::ostream& operator<<(std::ostream& os, WINDIVERT_ADDRESS const& addr) {
    auto layer = WINDIVERT_LAYER(addr.Layer);
    switch (layer) {
        case WINDIVERT_LAYER_NETWORK:
            return stdex::format_to(os,
                "     Layer: {}\n"
                "   Sniffed: {}\n"
                "  Outbound: {}\n"
                "  Loopback: {}\n"
                "  Impostor: {}\n"
                "      IPv6: {}\n"
                "IPChecksum: {}\n"
                "     IfIdx: {}\n"
                "  SubIfIdx: {}\n"
                , layer
                , addr.Sniffed
                , addr.Outbound
                , addr.Loopback
                , addr.Impostor
                , addr.IPv6
                , addr.IPChecksum
                , addr.Network.IfIdx
                , addr.Network.SubIfIdx
            );
        case WINDIVERT_LAYER_NETWORK_FORWARD:
            return stdex::format_to(os,
                "     Layer: {}\n"
                "   Sniffed: {}\n"
                "  Impostor: {}\n"
                "      IPv6: {}\n"
                "IPChecksum: {}\n"
                "     IfIdx: {}\n"
                "  SubIfIdx: {}\n"
                , layer
                , addr.Sniffed
                , addr.Impostor
                , addr.IPv6
                , addr.IPChecksum
                , addr.Network.IfIdx
                , addr.Network.SubIfIdx
            );
    }
    return os;
}
