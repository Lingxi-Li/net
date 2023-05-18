#pragma once

#include "Divert.hpp"

#include <vector>

class Passthru {
public:
    Passthru(char const* filter, UINT threadCount = 4, UINT batchSize = 16)
        : packetData(threadCount)
        , addrData(threadCount) {
        auto packetsSize = WINDIVERT_MTU_MAX * batchSize;
        for (UINT i = 0; i < threadCount; ++i) {
            packetData[i].resize(packetsSize);
            addrData[i].resize(batchSize);
        }
        handle.Open(filter, WINDIVERT_LAYER_NETWORK, 0, 0);
    }

private:
    std::vector<dvt::ByteVec> packetData;
    std::vector<dvt::AddrVec> addrData;
    dvt::Handle handle;
};
