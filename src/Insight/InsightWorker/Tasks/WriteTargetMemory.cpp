#include "WriteTargetMemory.hpp"

#include <cmath>
#include <algorithm>

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
        const auto blockSize = std::max(
            TargetMemorySize(256),
            memoryDescriptor.pageSize.value_or(TargetMemorySize(0))
        );
        const auto totalBytes = this->data.size();

        TargetMemorySize bytesWritten = 0;

        while (bytesWritten < totalBytes) {
            const auto bytesToWrite = std::min(
                blockSize,
                static_cast<decltype(blockSize)>(totalBytes - bytesWritten)
            );

            targetControllerService.writeMemory(
                this->memoryDescriptor.type,
                this->startAddress + bytesWritten,
                Targets::TargetMemoryBuffer(
                    this->data.begin() + bytesWritten,
                    this->data.begin() + bytesWritten + bytesToWrite
                )
            );

            bytesWritten += bytesToWrite;

            this->setProgressPercentage(static_cast<std::uint8_t>(
                (static_cast<float>(bytesWritten) + 1) / (static_cast<float>(totalBytes) / 100)
            ));
        }

        emit this->targetMemoryWritten(bytesWritten);
    }
}
