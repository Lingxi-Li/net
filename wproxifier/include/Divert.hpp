#pragma once

#include <windivert.h>

#undef min
#undef max

#include <format>
#include <iterator>
#include <ostream>
#include <system_error>
#include <tuple>
#include <type_traits>
#include <vector>

namespace dvt {

enum struct Api: DWORD {
    WinDivertOpen,
    WinDivertClose,
    WinDivertShutdown
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
        "WinDivertShutdown"
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

    std::tuple<BOOL, DWORD> Recv(
        void* packets,
        UINT packetsSize,
        UINT* packetsRecvedSize,
        UINT64 flags,
        WINDIVERT_ADDRESS* addrs,
        UINT* addrsSize,
        DWORD timeoutMs = INFINITE) const noexcept
    {
        OVERLAPPED overlap{};
        auto res = WinDivertRecvEx(handle, packets, packetsSize, packetsRecvedSize, flags, addrs, addrsSize, &overlap);
        if (res) return { res, NO_ERROR };
        auto error = GetLastError();
        if (error != ERROR_IO_PENDING) return { res, error };
        DWORD unused{};
        res = GetOverlappedResultEx(handle, &overlap, &unused, timeoutMs, TRUE);
        return { res, res ? DWORD(NO_ERROR) : GetLastError() };
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
    static void Check(BOOL res, Api api) {
        if (!res) throw Error(api);
    }

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
