#include "ReadTargetMemory.hpp"

#include <cmath>

#include "src/Targets/TargetMemory.hpp"

namespace Bloom
{
    using TargetController::TargetControllerConsole;

    void ReadTargetMemory::run(TargetControllerConsole& targetControllerConsole) {
        /*
         * To prevent locking up the TargetController for too long, we split the read into numerous reads.
         *
         * This allows the TargetController to service other commands in-between reads, reducing the likelihood of
         * command timeouts when we're reading lots of data.
         */
        constexpr auto readSize = 128;
        const auto readsRequired = static_cast<std::uint32_t>(
            std::ceil(static_cast<float>(this->size) / static_cast<float>(readSize))
        );

        Targets::TargetMemoryBuffer data;

        for (auto i = 0; i < readsRequired; i++) {
            auto dataSegment = targetControllerConsole.readMemory(
                this->memoryType,
                this->startAddress + static_cast<Targets::TargetMemoryAddress>(readSize * i),
                (this->size - data.size()) >= readSize
                    ? readSize
                    : static_cast<Targets::TargetMemorySize>(this->size - data.size()),
                this->excludedAddressRanges
            );

            std::move(dataSegment.begin(), dataSegment.end(), std::back_inserter(data));
        }

        emit this->targetMemoryRead(data);
    }
}
