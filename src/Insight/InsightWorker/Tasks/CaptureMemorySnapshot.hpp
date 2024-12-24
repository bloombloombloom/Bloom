#pragma once

#include <QString>
#include <optional>

#include "InsightWorkerTask.hpp"

#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemorySnapshot.hpp"

class CaptureMemorySnapshot: public InsightWorkerTask
{
    Q_OBJECT

public:
    CaptureMemorySnapshot(
        const QString& name,
        const QString& description,
        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        const std::vector<FocusedMemoryRegion>& focusedRegions,
        const std::vector<ExcludedMemoryRegion>& excludedRegions,
        const std::optional<Targets::TargetMemoryBuffer>& data
    );

    [[nodiscard]] QString brief() const override;
    [[nodiscard]] TaskGroups taskGroups() const override;

signals:
    void memorySnapshotCaptured(MemorySnapshot snapshot);

protected:
    void run(Services::TargetControllerService& targetControllerService) override;

private:
    QString name;
    QString description;
    const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor;
    const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor;
    std::vector<FocusedMemoryRegion> focusedRegions;
    std::vector<ExcludedMemoryRegion> excludedRegions;

    std::optional<Targets::TargetMemoryBuffer> data;
};
