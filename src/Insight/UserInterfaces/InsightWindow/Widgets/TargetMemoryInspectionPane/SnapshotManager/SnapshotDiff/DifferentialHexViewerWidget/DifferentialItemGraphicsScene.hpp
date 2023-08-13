#pragma once

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/ItemGraphicsScene.hpp"

#include "DifferentialHexViewerWidgetType.hpp"
#include "DifferentialHexViewerSharedState.hpp"
#include "DifferentialHexViewerItemRenderer.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/SnapshotManager/SnapshotDiff/SnapshotDiffSettings.hpp"

namespace Widgets
{
    class DifferentialItemGraphicsScene final: public ItemGraphicsScene
    {
        Q_OBJECT

    public:
        DifferentialItemGraphicsScene(
            DifferentialHexViewerWidgetType differentialHexViewerWidgetType,
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
        ByteItem* byteItemAtViewportTop();
        void updateByteItemChangedStates();

    protected:
        static constexpr auto CENTER_WIDTH = 46;

        DifferentialHexViewerWidgetType differentialHexViewerWidgetType;
        DifferentialHexViewerSharedState& diffHexViewerState;
        const SnapshotDiffSettings& snapshotDiffSettings;
        DifferentialHexViewerItemRenderer* differentialHexViewerItemRenderer = nullptr;
        DifferentialItemGraphicsScene* other = nullptr;

        void initRenderer() override;
        QMargins margins() override;

        void onOtherHoveredAddress(const std::optional<Targets::TargetMemoryAddress>& address);
        void onOtherSelectionChanged(
            const std::unordered_map<Targets::TargetMemoryAddress, ByteItem*>& selectedByteItemsByAddress
        );
    };
}
