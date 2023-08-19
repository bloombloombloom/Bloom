#pragma once

#include <memory>

#include "InsightWorkerTask.hpp"
#include "src/Targets/TargetMemory.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/TopLevelGroupItem.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/HexViewerSharedState.hpp"

class ConstructHexViewerTopLevelGroupItem: public InsightWorkerTask
{
    Q_OBJECT

public:
    ConstructHexViewerTopLevelGroupItem(
        const std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
        const std::vector<ExcludedMemoryRegion>& excludedMemoryRegions,
        const Widgets::HexViewerSharedState& hexViewerState
    );

    QString brief() const override {
        return "Preparing hex viewer";
    }

    TaskGroups taskGroups() const override {
        return TaskGroups();
    };

signals:
    void topLevelGroupItem(Widgets::TopLevelGroupItem* item);

protected:
    void run(Services::TargetControllerService&) override;

private:
    const std::vector<FocusedMemoryRegion>& focusedMemoryRegions;
    const std::vector<ExcludedMemoryRegion>& excludedMemoryRegions;
    const Widgets::HexViewerSharedState& hexViewerState;
};
