#pragma once

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/ItemGraphicsScene.hpp"

#include "DifferentialHexViewerSharedState.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/SnapshotManager/SnapshotDiff/SnapshotDiffSettings.hpp"

namespace Bloom::Widgets
{
    class DifferentialItemGraphicsScene final: public ItemGraphicsScene
    {
        Q_OBJECT

    public:
        DifferentialItemGraphicsScene(
            DifferentialHexViewerSharedState& state,
            const SnapshotDiffSettings& snapshotDiffSettings,
            const Targets::TargetMemoryDescriptor& targetMemoryDescriptor,
            const std::optional<Targets::TargetMemoryBuffer>& data,
            const std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
            const std::vector<ExcludedMemoryRegion>& excludedMemoryRegions,
            HexViewerWidgetSettings& settings,
            QGraphicsView* parent
        );

        void setOther(DifferentialItemGraphicsScene* other);
        ByteItem* byteItemAtViewportVerticalCenter();

    protected:
        DifferentialHexViewerSharedState& diffHexViewerState;
        const SnapshotDiffSettings& snapshotDiffSettings;
        DifferentialItemGraphicsScene* other = nullptr;

        void onOtherHoveredAddress(const std::optional<Targets::TargetMemoryAddress>& address);
        void onOtherSelectionChanged(
            const std::unordered_map<Targets::TargetMemoryAddress, ByteItem*>& selectedByteItemsByAddress
        );
    };
}
