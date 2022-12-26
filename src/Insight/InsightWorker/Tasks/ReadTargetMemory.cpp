#include "ReadTargetMemory.hpp"

#include <cmath>
#include <algorithm>

#include "src/Targets/TargetMemory.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Bloom
{
    using Services::TargetControllerService;

    void ReadTargetMemory::run(TargetControllerService& targetControllerService) {
        using Targets::TargetMemorySize;

        const auto& targetDescriptor = targetControllerService.getTargetDescriptor();
        const auto memoryDescriptorIt = targetDescriptor.memoryDescriptorsByType.find(this->memoryType);

        if (memoryDescriptorIt == targetDescriptor.memoryDescriptorsByType.end()) {
            throw Exceptions::Exception("Invalid memory type");
        }

        const auto& memoryDescriptor = memoryDescriptorIt->second;

        /*
         * To prevent locking up the TargetController for too long, we split the read into numerous reads.
         *
         * This allows the TargetController to service other commands in-between reads, reducing the likelihood of
         * command timeouts when we're reading lots of data.
         */
        const auto readSize = std::max(
            TargetMemorySize(256),
            memoryDescriptor.pageSize.value_or(TargetMemorySize(0))
        );
        const auto readsRequired = static_cast<std::uint32_t>(
            std::ceil(static_cast<float>(this->size) / static_cast<float>(readSize))
        );

        Targets::TargetMemoryBuffer data;

        for (std::uint32_t i = 0; i < readsRequired; i++) {
            auto dataSegment = targetControllerService.readMemory(
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
