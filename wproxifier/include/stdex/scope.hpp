#pragma once

#define STDEX_JOIN_(a, b) a##b
#define STDEX_JOIN(a, b) STDEX_JOIN_(a, b)
#define STDEX_UNIQUE_NAME STDEX_JOIN(stdex_unique_name_at_line_, __LINE__)

#define STDEX_FINALLY(...) auto STDEX_UNIQUE_NAME = ::stdex::make_scope_exit([&] { __VA_ARGS__ })

namespace stdex {

template <typename T>
struct scope_exit {
    T callable;

    ~scope_exit() {
        callable();
    }
};

template <typename T>
scope_exit<T> make_scope_exit(T callable) noexcept {
    return { callable }; // mandatory copy elision
}

} // namespace stdex
