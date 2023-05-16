#include "Divert.hpp"
#include "field_view.hpp"

#include <iostream>

int main() {
    dvt::Error err{dvt::Api::WinDivertOpen, 1275};
    std::cout << err << std::endl;
}
