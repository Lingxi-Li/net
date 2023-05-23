#include "Passthru.hpp"

#include <iostream>
#include <string>

int main() {
    try {
        std::string cmd;
        Passthru passthru;
        passthru.Start("false");
        std::cout << "Press <Enter> to stop.\n";
        std::getline(std::cin, cmd);
        passthru.Stop();
    }
    catch (std::exception const& err) {
        std::cout << err.what() << '\n';
    }
}
