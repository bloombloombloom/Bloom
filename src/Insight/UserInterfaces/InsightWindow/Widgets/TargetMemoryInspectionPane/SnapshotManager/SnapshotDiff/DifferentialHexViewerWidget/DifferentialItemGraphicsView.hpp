#pragma once

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/ItemGraphicsView.hpp"

#include "DifferentialHexViewerWidgetType.hpp"
#include "DifferentialHexViewerSharedState.hpp"
#include "DifferentialItemGraphicsScene.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/SnapshotManager/SnapshotDiff/SnapshotDiffSettings.hpp"

namespace Widgets
{
    class DifferentialItemGraphicsView final: public ItemGraphicsView
    {
        Q_OBJECT

    public:
        DifferentialItemGraphicsView(
            DifferentialHexViewerWidgetType differentialHexViewerWidgetType,
            DifferentialHexViewerSharedState& state,
            const SnapshotDiffSettings& snapshotDiffSettings,
            const Targets::TargetMemoryDescriptor& targetMemoryDescriptor,
            const std::optional<Targets::TargetMemoryBuffer>& data,
            const std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
            const std::vector<ExcludedMemoryRegion>& excludedMemoryRegions,
            HexViewerWidgetSettings& settings,
            QWidget* parent
        );

        void initScene() override;

        void setOther(DifferentialItemGraphicsView* other);
        void alignScroll(Targets::TargetMemoryAddress otherByteItemAddress, int otherByteItemYOffset);

    protected:
        DifferentialHexViewerWidgetType differentialHexViewerWidgetType;
        DifferentialHexViewerSharedState& state;
        const SnapshotDiffSettings& snapshotDiffSettings;

        DifferentialItemGraphicsScene* differentialScene = nullptr;
        DifferentialItemGraphicsView* other = nullptr;

        void scrollContentsBy(int dx, int dy) override;
    };
}
