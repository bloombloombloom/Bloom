#pragma once

#include <vector>
#include <QFileInfo>
#include <QString>

#include "InsightWorkerTask.hpp"

#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"
#include "src/Targets/TargetDescriptor.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemorySnapshot.hpp"
#include "src/Helpers/EnumToStringMappings.hpp"

class RetrieveMemorySnapshots: public InsightWorkerTask
{
    Q_OBJECT

public:
    RetrieveMemorySnapshots(
        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        const Targets::TargetDescriptor& targetDescriptor
    );
    [[nodiscard]] QString brief() const override;

signals:
    void memorySnapshotsRetrieved(std::vector<MemorySnapshot> snapshots);

protected:
    void run(Services::TargetControllerService& targetControllerService) override;

private:
    const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor;
    const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor;
    const Targets::TargetDescriptor& targetDescriptor;

    std::vector<MemorySnapshot> getSnapshots();
    void migrateOldSnapshotFiles();
};
