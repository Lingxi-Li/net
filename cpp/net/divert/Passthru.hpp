#pragma once

#include "Divert.hpp"

#include <format>
#include <fstream>
#include <vector>

class Passthru {
public:
    Passthru(char const* filter, UINT threadCount = 4, UINT batchSize = 8)
        : packetData(threadCount)
        , addrData(threadCount)
        , logs(threadCount) {
        auto packetsSize = WINDIVERT_MTU_MAX * batchSize;
        for (UINT i = 0; i < threadCount; ++i) {
            packetData[i].resize(packetsSize);
            addrData[i].resize(batchSize);
            logs[i].exceptions(std::ios_base::failbit);
            logs[i].open(std::format("Passthru_thread_{}.log", i));
            logs[i].exceptions(std::ios_base::goodbit);
        }
        
        handle.Open(filter, WINDIVERT_LAYER_NETWORK, 0, 0);
        // TODO: spawn worker threads
    }

    void StartProcessing(UINT threadIndex) noexcept {
        auto const packets = packetData[threadIndex].data();
        auto const packetsSize = packetData[threadIndex].size();
        auto const addrs = addrData[threadIndex].data();
        auto const addrsSize = addrData[threadIndex].size() * sizeof(WINDIVERT_ADDRESS);
        auto& log = logs[threadIndex];

        while (true) {
            UINT packetsRecvedSize = 0;
            UINT addrsRecvedSize = UINT(addrsSize);
            if (handle.Recv(packets, UINT(packetsSize), &packetsRecvedSize, 0, addrs, &addrsRecvedSize)) {
                // TODO: log inspection
                if (!handle.Send(packets, packetsRecvedSize, NULL, 0, addrs, addrsRecvedSize)) {
                    // TODO: log error
                }
            }
            else {
                // TODO: log error
            }
        }
    }

private:
    std::vector<dvt::ByteVec> packetData;
    std::vector<dvt::AddrVec> addrData;
    std::vector<std::ofstream> logs;
    dvt::Handle handle;
};
