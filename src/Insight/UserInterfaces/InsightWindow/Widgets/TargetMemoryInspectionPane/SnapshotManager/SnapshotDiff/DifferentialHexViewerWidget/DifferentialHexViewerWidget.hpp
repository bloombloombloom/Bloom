#pragma once

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/HexViewerWidget.hpp"

#include "DifferentialHexViewerWidgetType.hpp"
#include "DifferentialHexViewerSharedState.hpp"
#include "DifferentialItemGraphicsView.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/SnapshotManager/SnapshotDiff/SnapshotDiffSettings.hpp"

namespace Widgets
{
    class DifferentialHexViewerWidget final: public HexViewerWidget
    {
        Q_OBJECT

    public:
        DifferentialHexViewerWidget(
            DifferentialHexViewerWidgetType type,
            DifferentialHexViewerSharedState& state,
            const SnapshotDiffSettings& snapshotDiffSettings,
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            const Targets::TargetState& targetState,
            const std::optional<Targets::TargetMemoryBuffer>& data,
            HexViewerWidgetSettings& settings,
            const std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
            const std::vector<ExcludedMemoryRegion>& excludedMemoryRegions,
            QWidget* parent
        );

        void init() override;
        void updateValues() override;

        void setOther(DifferentialHexViewerWidget* other);

    private:
        DifferentialHexViewerWidgetType type;
        DifferentialHexViewerSharedState& state;

        const SnapshotDiffSettings& snapshotDiffSettings;
        DifferentialItemGraphicsView* differentialView = nullptr;
        DifferentialItemGraphicsScene* differentialScene = nullptr;

        DifferentialHexViewerWidget* other = nullptr;

        void onOtherSettingsChanged(const HexViewerWidgetSettings& settings);
    };
}
