#pragma once

#include <QWidget>
#include <QShowEvent>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QPlainTextEdit>
#include <optional>

#include "SnapshotDiffSettings.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PanelWidget.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/SvgToolButton.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/Label.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TaskProgressIndicator/TaskProgressIndicator.hpp"

#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"
#include "src/Targets/TargetState.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemorySnapshot.hpp"
#include "DifferentialHexViewerWidget/DifferentialHexViewerWidget.hpp"
#include "ChangeListPane/ChangeListPane.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/ContextMenuAction.hpp"

namespace Widgets
{
    class SnapshotDiff: public QWidget
    {
        Q_OBJECT

    public:
        SnapshotDiff(
            MemorySnapshot& snapshotA,
            MemorySnapshot& snapshotB,
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            const Targets::TargetState& targetState,
            QWidget* parent = nullptr
        );

        SnapshotDiff(
            MemorySnapshot& snapshotA,
            Targets::TargetMemoryBuffer dataB,
            std::vector<FocusedMemoryRegion> focusedRegionsB,
            std::vector<ExcludedMemoryRegion> excludedRegionsB,
            Targets::TargetStackPointer stackPointerB,
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            const Targets::TargetState& targetState,
            QWidget* parent = nullptr
        );

        void refreshB(
            Targets::TargetMemoryBuffer data,
            std::vector<FocusedMemoryRegion> focusedRegions,
            std::vector<ExcludedMemoryRegion> excludedRegions,
            Targets::TargetStackPointer stackPointer
        );

    protected:
        void showEvent(QShowEvent* event) override;
        void resizeEvent(QResizeEvent* event) override;
        void keyPressEvent(QKeyEvent* keyEvent) override;

    private:
        SnapshotDiffSettings settings;

        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor;
        const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor;
        const Targets::TargetState& targetState;

        QWidget* container = nullptr;

        SvgToolButton* syncHexViewerSettingsButton = nullptr;
        SvgToolButton* syncHexViewerScrollButton = nullptr;
        SvgToolButton* syncHexViewerHoverButton = nullptr;
        SvgToolButton* syncHexViewerSelectionButton = nullptr;

        QToolButton* viewChangeListButton = nullptr;

        PanelWidget* leftPanel = nullptr;
        PanelState leftPanelState = {300, true};

        PaneState changeListPaneState = PaneState(true, true, std::nullopt);
        ChangeListPane* changeListPane = nullptr;

        QWidget* dataAContainer = nullptr;
        QWidget* dataBContainer = nullptr;

        Label* dataAPrimaryLabel = nullptr;
        Label* dataBPrimaryLabel = nullptr;

        Label* dataASecondaryLabel = nullptr;
        Label* dataBSecondaryLabel = nullptr;

        DifferentialHexViewerSharedState differentialHexViewerSharedState;

        std::optional<Targets::TargetMemoryBuffer> hexViewerDataA;
        std::vector<FocusedMemoryRegion> focusedRegionsA;
        std::vector<ExcludedMemoryRegion> excludedRegionsA;
        Targets::TargetStackPointer stackPointerA;
        DifferentialHexViewerWidget* hexViewerWidgetA = nullptr;
        HexViewerWidgetSettings hexViewerWidgetSettingsA = HexViewerWidgetSettings();

        std::optional<Targets::TargetMemoryBuffer> hexViewerDataB;
        std::vector<FocusedMemoryRegion> focusedRegionsB;
        std::vector<ExcludedMemoryRegion> excludedRegionsB;
        Targets::TargetStackPointer stackPointerB;
        DifferentialHexViewerWidget* hexViewerWidgetB = nullptr;
        HexViewerWidgetSettings hexViewerWidgetSettingsB = HexViewerWidgetSettings();

        bool comparingWithCurrent = false;

        ContextMenuAction* restoreBytesAction = nullptr;

        QWidget* bottomBar = nullptr;
        QHBoxLayout* bottomBarLayout = nullptr;
        Label* memoryCapacityLabel = nullptr;
        Label* memorySegmentNameLabel = nullptr;
        Label* diffCountLabel = nullptr;

        TaskProgressIndicator* taskProgressIndicator = nullptr;

        void init();

        void onHexViewerAReady();
        void onHexViewerBReady();

        void refreshDifferences();

        void toggleChangeListPane();

        void setSyncHexViewerSettingsEnabled(bool enabled);
        void setSyncHexViewerScrollEnabled(bool enabled);
        void setSyncHexViewerHoverEnabled(bool enabled);
        void setSyncHexViewerSelectionEnabled(bool enabled);

        void restoreSelectedBytes(
            std::set<Targets::TargetMemoryAddress> addresses,
            bool confirmationPromptEnabled
        );

        void onTargetStateChanged();
    };
}
