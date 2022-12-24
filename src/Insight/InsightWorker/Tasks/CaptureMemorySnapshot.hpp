#pragma once

#include <QString>
#include <optional>

#include "InsightWorkerTask.hpp"

#include "src/Targets/TargetMemory.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemorySnapshot.hpp"

namespace Bloom
{
    class CaptureMemorySnapshot: public InsightWorkerTask
    {
        Q_OBJECT

    public:
        CaptureMemorySnapshot(
            const QString& name,
            const QString& description,
            Targets::TargetMemoryType memoryType,
            const std::vector<FocusedMemoryRegion>& focusedRegions,
            const std::vector<ExcludedMemoryRegion>& excludedRegions,
            const std::optional<Targets::TargetMemoryBuffer>& data
        );

    signals:
        void memorySnapshotCaptured(MemorySnapshot snapshot);

    protected:
        void run(TargetController::TargetControllerConsole& targetControllerConsole) override;

    private:
        QString name;
        QString description;
        Targets::TargetMemoryType memoryType;
        std::vector<FocusedMemoryRegion> focusedRegions;
        std::vector<ExcludedMemoryRegion> excludedRegions;

        std::optional<Targets::TargetMemoryBuffer> data;
    };
}
