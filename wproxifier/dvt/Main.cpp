#include "Passthru.hpp"

#include <iostream>

int main() {
    try {
        Passthru passthru("false");
    }
    catch (std::exception const& err) {
        std::cout << err.what() << '\n';
    }
}
