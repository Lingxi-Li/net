#pragma once

#include <windivert.h>

#undef min
#undef max

#include <net/ipv4_view.hpp>
#include <net/tcp_view.hpp>

#include <stdex/format.hpp>

#include <format>
#include <iterator>
#include <memory>
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
    Error(Api api, DWORD code = GetLastError())
        : FailedApi(api)
        , ErrorCode(code)
        , Message(std::format("{}, 0x{:08X}: {}", FailedApi, code, std::system_category().message(code))) {
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

template <typename T>
void operator&(T res, Api api) {
    Check(res, api);
}

// F: void (net::ipv4_view, net::tcp_view, WINDIVERT_ADDRESS &, UINT64)
// E: void (std::exception const&, UINT64) noexcept
template <typename F, typename E>
UINT64 RelayIpv4Tcp(HANDLE handle, F twiddle, E handleError) {
    auto packet = std::make_unique<net::byte_t[]>(WINDIVERT_MTU_MAX);
    WINDIVERT_ADDRESS addr{};
    UINT64 i = 0;
    while (true) {
        try {
            if (!WinDivertRecv(handle, packet.get(), WINDIVERT_MTU_MAX, NULL, &addr)) {
                auto error = GetLastError();
                if (error == ERROR_NO_DATA) break;
                throw Error(Api::WinDivertRecv, error);
            }
            net::ipv4_view ip{packet.get()};
            net::tcp_view tcp{ip.payload()};
            twiddle(ip, tcp, addr, i);
            WinDivertSend(handle, packet.get(), WINDIVERT_MTU_MAX, NULL, &addr) & Api::WinDivertSend;
        }
        catch (std::exception const& ex) {
            handleError(ex, i);
        }
        ++i;
    }
    return i;
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
                "{}\n"
                "----------------------------\n"
                " Sniffed: {}      Impostor: {}\n"
                "Loopback: {}      Outbound: {}\n"
                "    IPv6: {}    IPChecksum: {}\n"
                "   IfIdx: {}      SubIfIdx: {}\n"
                "----------------------------\n"
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
    }
    return os;
}
