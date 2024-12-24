#include "ReadTargetMemory.hpp"

#include <cstdint>
#include <QLocale>
#include <cmath>
#include <algorithm>

#include "src/Exceptions/Exception.hpp"

using Services::TargetControllerService;

ReadTargetMemory::ReadTargetMemory(
    const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
    const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
    Targets::TargetMemoryAddress startAddress,
    Targets::TargetMemorySize size,
    const std::set<Targets::TargetMemoryAddressRange>& excludedAddressRanges
)
    : addressSpaceDescriptor(addressSpaceDescriptor)
    , memorySegmentDescriptor(memorySegmentDescriptor)
    , startAddress(startAddress)
    , size(size)
    , excludedAddressRanges(excludedAddressRanges)
{}

QString ReadTargetMemory::brief() const {
    return "Reading " + QLocale{QLocale::English}.toString(this->size) + " byte(s) from \""
       + QString::fromStdString(this->memorySegmentDescriptor.name) + "\" segment, via \""
       + QString::fromStdString(this->addressSpaceDescriptor.key) + "\" address space";
}

TaskGroups ReadTargetMemory::taskGroups() const {
    return {
        TaskGroup::USES_TARGET_CONTROLLER,
    };
}

void ReadTargetMemory::run(TargetControllerService& targetControllerService) {
    using Targets::TargetMemorySize;

    /*
     * To prevent locking up the TargetController for too long, we split the read into numerous reads.
     *
     * This allows the TargetController to service other commands in-between reads, reducing the likelihood of
     * command timeouts when we're reading lots of data.
     */
    const auto readSize = std::max(
        TargetMemorySize{256},
        this->memorySegmentDescriptor.pageSize.value_or(TargetMemorySize{0})
    );
    const auto readsRequired = static_cast<std::uint32_t>(
        std::ceil(static_cast<float>(this->size) / static_cast<float>(readSize))
    );

    auto data = Targets::TargetMemoryBuffer{};
    data.reserve(this->size);

    for (auto i = std::size_t{0}; i < readsRequired; i++) {
        auto dataSegment = targetControllerService.readMemory(
            this->addressSpaceDescriptor,
            this->memorySegmentDescriptor,
            this->startAddress + static_cast<Targets::TargetMemoryAddress>(readSize * i),
            (this->size - data.size()) >= readSize ? readSize : static_cast<TargetMemorySize>(this->size - data.size()),
            true,
            this->excludedAddressRanges
        );

        data.insert(data.end(), dataSegment.begin(), dataSegment.end());
        this->setProgressPercentage(static_cast<std::uint8_t>(
            (static_cast<float>(i) + 1) / (static_cast<float>(readsRequired) / 100)
        ));
    }

    emit this->targetMemoryRead(data);
}
