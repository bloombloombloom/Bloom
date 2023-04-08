#include "WriteTargetMemory.hpp"

#include <cmath>
#include <algorithm>
#include <numeric>

namespace Bloom
{
    using Services::TargetControllerService;

    void WriteTargetMemory::run(TargetControllerService& targetControllerService) {
        using Targets::TargetMemorySize;

        /*
         * To prevent locking up the TargetController for too long, we split the write operation into numerous
         * operations.
         *
         * This allows the TargetController to service other commands in-between reads, reducing the likelihood of
         * command timeouts when we're writing lots of data.
         */
        const auto maxBlockSize = std::max(
            TargetMemorySize(256),
            memoryDescriptor.pageSize.value_or(TargetMemorySize(0))
        );

        const TargetMemorySize totalBytesToWrite = std::accumulate(
            this->blocks.begin(),
            this->blocks.end(),
            TargetMemorySize{0},
            [] (TargetMemorySize bytes, const Block& block) {
                return bytes + block.data.size();
            }
        );

        TargetMemorySize totalBytesWritten = 0;

        for (const auto& block : this->blocks) {
            const auto totalBytes = block.data.size();

            TargetMemorySize bytesWritten = 0;

            while (bytesWritten < totalBytes) {
                const auto bytesToWrite = std::min(
                    maxBlockSize,
                    static_cast<decltype(maxBlockSize)>(totalBytes - bytesWritten)
                );

                targetControllerService.writeMemory(
                    this->memoryDescriptor.type,
                    block.startAddress + bytesWritten,
                    Targets::TargetMemoryBuffer(
                        block.data.begin() + bytesWritten,
                        block.data.begin() + bytesWritten + bytesToWrite
                    )
                );

                bytesWritten += bytesToWrite;
                totalBytesWritten += bytesToWrite;

                this->setProgressPercentage(static_cast<std::uint8_t>(
                    (static_cast<float>(totalBytesWritten) + 1) / (static_cast<float>(totalBytesToWrite) / 100)
                ));
            }
        }

        emit this->targetMemoryWritten(totalBytesWritten);
    }
}
