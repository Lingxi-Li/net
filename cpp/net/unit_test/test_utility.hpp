#pragma once

#include <cstddef>

#include <format>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using byte_vec = std::vector<unsigned char>;

template <std::size_t N> inline
auto make_byte_vec(unsigned char const (&arr)[N]) {
    return byte_vec(arr, arr + N);
}

template <typename T> inline
std::string str_via_ostream(T const& v) {
    std::ostringstream os;
    os << v;
    return os.str();
}

template <typename T> inline
std::string str_via_format(T const& v) {
    return std::format("{}", v);
}
