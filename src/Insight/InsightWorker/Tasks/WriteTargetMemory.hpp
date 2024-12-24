#pragma once

#include <vector>

#include "InsightWorkerTask.hpp"

#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"

class WriteTargetMemory: public InsightWorkerTask
{
    Q_OBJECT

public:
    /*
     * A Block is just a block of contiguous data to write. A single WriteTargetMemory task can write multiple
     * blocks to a particular memory.
     */
    struct Block {
        Targets::TargetMemoryAddress startAddress;
        const Targets::TargetMemoryBuffer data;

        Block(
            Targets::TargetMemoryAddress startAddress,
            const Targets::TargetMemoryBuffer& data
        )
            : startAddress(startAddress)
            , data(data)
        {}
    };

    WriteTargetMemory(
        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        std::vector<Block>&& blocks
    );
    WriteTargetMemory(
        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        Targets::TargetMemoryAddress startAddress,
        const Targets::TargetMemoryBuffer& data
    );
    [[nodiscard]] QString brief() const override;
    [[nodiscard]] TaskGroups taskGroups() const override;

signals:
    void targetMemoryWritten(Targets::TargetMemorySize bytesWritten);

protected:
    void run(Services::TargetControllerService& targetControllerService) override;

private:
    const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor;
    const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor;
    std::vector<Block> blocks;

    [[nodiscard]] Targets::TargetMemorySize totalSize() const;
};
