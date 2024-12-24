#include "WriteTargetMemory.hpp"

#include <cmath>
#include <QLocale>
#include <algorithm>
#include <numeric>

#include "src/Exceptions/Exception.hpp"

using Services::TargetControllerService;
using Targets::TargetMemorySize;

WriteTargetMemory::WriteTargetMemory(
    const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
    const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
    std::vector<Block>&& blocks
)
    : addressSpaceDescriptor(addressSpaceDescriptor)
    , memorySegmentDescriptor(memorySegmentDescriptor)
    , blocks(std::move(blocks))
{}

WriteTargetMemory::WriteTargetMemory(
    const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
    const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
    Targets::TargetMemoryAddress startAddress,
    const Targets::TargetMemoryBuffer& data
)
    : WriteTargetMemory(
        addressSpaceDescriptor,
        memorySegmentDescriptor,
        std::vector<Block>{{startAddress, data}}
    )
{}

QString WriteTargetMemory::brief() const {
    return "Writing " + QLocale{QLocale::English}.toString(this->totalSize()) + " byte(s) to \""
        + QString::fromStdString(this->memorySegmentDescriptor.name) + "\"";
}

TaskGroups WriteTargetMemory::taskGroups() const {
    return {
        TaskGroup::USES_TARGET_CONTROLLER,
    };
}

void WriteTargetMemory::run(TargetControllerService& targetControllerService) {
    if (!this->memorySegmentDescriptor.debugModeAccess.writeable) {
        throw Exceptions::Exception{"Invalid request - cannot write to this memory segment during a debug session."};
    }

    /*
     * To prevent locking up the TargetController for too long, we split the write operation into numerous
     * operations.
     *
     * This allows the TargetController to service other commands in-between reads, reducing the likelihood of
     * command timeouts when we're writing lots of data.
     */
    const auto maxBlockSize = std::max(
        TargetMemorySize{256},
        this->memorySegmentDescriptor.pageSize.value_or(TargetMemorySize{0})
    );

    const auto writeSize = this->totalSize();
    auto totalBytesWritten = TargetMemorySize{0};

    for (const auto& block : this->blocks) {
        const auto totalBytes = block.data.size();

        TargetMemorySize bytesWritten = 0;

        while (bytesWritten < totalBytes) {
            const auto bytesToWrite = std::min(
                maxBlockSize,
                static_cast<decltype(maxBlockSize)>(totalBytes - bytesWritten)
            );

            targetControllerService.writeMemory(
                this->addressSpaceDescriptor,
                this->memorySegmentDescriptor,
                block.startAddress + bytesWritten,
                Targets::TargetMemoryBuffer{
                    block.data.begin() + bytesWritten,
                    block.data.begin() + bytesWritten + bytesToWrite
                }
            );

            bytesWritten += bytesToWrite;
            totalBytesWritten += bytesToWrite;

            this->setProgressPercentage(static_cast<std::uint8_t>(
                (static_cast<float>(totalBytesWritten) + 1) / (static_cast<float>(writeSize) / 100)
            ));
        }
    }

    emit this->targetMemoryWritten(totalBytesWritten);
}

TargetMemorySize WriteTargetMemory::totalSize() const {
    return std::accumulate(
        this->blocks.begin(),
        this->blocks.end(),
        TargetMemorySize{0},
        [] (TargetMemorySize bytes, const Block& block) {
            return bytes + block.data.size();
        }
    );
}
