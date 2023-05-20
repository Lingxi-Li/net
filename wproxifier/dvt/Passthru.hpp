#pragma once

#include <Divert.hpp>

#include <format>
#include <fstream>
#include <system_error>
#include <tuple>
#include <vector>

class Passthru {
    struct ThreadData {
        ThreadData(UINT packetCount, UINT threadIndex)
            : packets(WINDIVERT_MTU_MAX * packetCount)
            , addrs(packetCount) {
            auto path = std::format("Passthru_thread_{}.log", threadIndex);
            log.open(path);
            if (!log) {
                auto what = std::format(
                    "Failed to open file '{}': {}",
                    path,
                    std::system_category().message(GetLastError())
                );
                throw std::ios_base::failure(what);
            }
        }

        std::tuple<UINT8*, UINT, WINDIVERT_ADDRESS*, UINT, std::ofstream&> Arms() noexcept {
            return { 
                packets.data(),
                UINT(packets.size()),
                addrs.data(),
                UINT(sizeof(WINDIVERT_ADDRESS) * addrs.size()),
                log
            };
        }

    private:
        dvt::ByteVec packets;
        dvt::AddrVec addrs;
        std::ofstream log;
    };

public:
    Passthru(char const* filter, UINT threadCount = 4, UINT batchSize = 8) {
        threadData.reserve(threadCount);
        for (UINT i = 0; i < threadCount; ++i) {
            threadData.emplace_back(batchSize, i);
        }
        handle.Open(filter, WINDIVERT_LAYER_NETWORK, 0, 0);
        // TODO: spawn worker threads
    }

    void StartProcessing(UINT threadIndex) noexcept {
        auto const [packets, packetsByteLen, addrs, addrsByteLen, log] = threadData[threadIndex].Arms();
        while (true) {
            UINT packetsRecvedByteLen = 0;
            UINT addrsRecvedByteLen = addrsByteLen;
            if (handle.Recv(packets, packetsByteLen, &packetsRecvedByteLen, 0, addrs, &addrsRecvedByteLen)) {
                // TODO: log inspection
                if (!handle.Send(packets, packetsRecvedByteLen, NULL, 0, addrs, addrsRecvedByteLen)) {
                    // TODO: log error
                }
            }
            else {
                // TODO: log error
            }
        }
    }

private:
    std::vector<ThreadData> threadData;
    dvt::Handle handle;
};
