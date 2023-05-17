#include "Divert.hpp"
#include "field_view.hpp"

#include <iostream>

int main() {
    constexpr char const filter[] = "true";
    auto layer = WINDIVERT_LAYER_NETWORK;
    auto flags = WINDIVERT_FLAG_SNIFF | WINDIVERT_FLAG_RECV_ONLY;
    try {
        dvt::Handle handle("true", WINDIVERT_LAYER_NETWORK, 0, flags);
        handle.Open("true", WINDIVERT_LAYER_NETWORK, 0, flags);
    }
    catch (std::exception const& err) {
        std::cout << err.what() << std::endl;
    }
}
