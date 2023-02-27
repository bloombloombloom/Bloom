#pragma once

#include <memory>

#include "InsightWorkerTask.hpp"
#include "src/Targets/TargetMemory.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/TopLevelGroupItem.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/HexViewerSharedState.hpp"

namespace Bloom
{
    class ConstructHexViewerTopLevelGroupItem: public InsightWorkerTask
    {
        Q_OBJECT

    public:
        ConstructHexViewerTopLevelGroupItem(
            const std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
            const Widgets::HexViewerSharedState& hexViewerState
        );

        TaskGroups getTaskGroups() const override {
            return TaskGroups();
        };

    signals:
        void topLevelGroupItem(Widgets::TopLevelGroupItem* item);

    protected:
        void run(Services::TargetControllerService&) override;

    private:
        const Widgets::HexViewerSharedState& hexViewerState;
        const std::vector<FocusedMemoryRegion>& focusedMemoryRegions;
    };
}
