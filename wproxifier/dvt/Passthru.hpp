#pragma once

#include <Divert.hpp>

#include <format>
#include <fstream>
#include <system_error>
#include <thread>
#include <tuple>
#include <vector>

class Passthru {

    struct ThreadData {
        ThreadData(UINT packetCount, UINT threadIndex)
            : packets(WINDIVERT_MTU_MAX* packetCount)
            , addrs(packetCount)
        {
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
    void Start(char const* filter, UINT threadCount = 4, UINT batchSize = 8) {
        threadData.reserve(threadCount);
        threads.reserve(threadCount);
        do {
            threadData.emplace_back(batchSize, UINT(threadData.size()));
        }
        while (threadData.size() < threadCount);
        handle.Open(filter, WINDIVERT_LAYER_NETWORK, 0, 0);
        for (auto& data : threadData) {
            threads.emplace_back(&Passthru::Process, &handle, &data);
        }
    }

    void Stop() {
        handle.Shutdown();
        for (auto& t : threads) {
            t.join();
        }
        threads.clear();
        handle.Close();
        threadData.clear();
    }

private:
    static void Process(dvt::Handle const* handle, ThreadData* data) noexcept {
        auto const [packets, packetsByteLen, addrs, addrsByteLen, log] = data->Arms();
        while (true) {
            UINT packetsRecvedByteLen = 0;
            UINT addrsRecvedByteLen = addrsByteLen;
            auto res = handle->Recv(
                packets,
                packetsByteLen,
                &packetsRecvedByteLen,
                0,
                addrs,
                &addrsRecvedByteLen
            );
            if (res) {
                // TODO: log inspection
                res = handle->Send(packets, packetsRecvedByteLen, NULL, 0, addrs, addrsRecvedByteLen);
                if (!res) {
                    // TODO: log error
                }
            }
            else {
                auto error = GetLastError();
                if (error == ERROR_NO_DATA) break;
                // TODO: log error
            }
        }
    }

    std::vector<ThreadData> threadData;
    dvt::Handle handle;
    std::vector<std::thread> threads;
};
