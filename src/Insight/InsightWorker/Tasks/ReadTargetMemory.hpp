#pragma once

#include "InsightWorkerTask.hpp"

#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"

class ReadTargetMemory: public InsightWorkerTask
{
    Q_OBJECT

public:
    ReadTargetMemory(
        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        Targets::TargetMemoryAddress startAddress,
        Targets::TargetMemorySize size,
        const std::set<Targets::TargetMemoryAddressRange>& excludedAddressRanges = {}
    );
    [[nodiscard]] QString brief() const override ;
    [[nodiscard]] TaskGroups taskGroups() const override;

signals:
    void targetMemoryRead(Targets::TargetMemoryBuffer buffer);

protected:
    void run(Services::TargetControllerService& targetControllerService) override;

private:
    const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor;
    const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor;
    Targets::TargetMemoryAddress startAddress;
    Targets::TargetMemorySize size;
    std::set<Targets::TargetMemoryAddressRange> excludedAddressRanges;
};
