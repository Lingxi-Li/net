#include "Passthru.hpp"
#include "RedirPort.hpp"

#include <iostream>
#include <string>

void PlayPassthru() {
    std::string cmd;
    Passthru passthru;
    passthru.Start("loopback == 1 && tcp.DstPort == 8000");
    std::cout << "Press <Enter> to stop.\n";
    std::getline(std::cin, cmd);
    passthru.Stop();
}

int main() {
    try {
        RedirPort();
    }
    catch (std::exception const& err) {
        std::cout << err.what() << '\n';
    }
}
