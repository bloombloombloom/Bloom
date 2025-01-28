#pragma once

#include <QWidget>
#include <optional>
#include <vector>
#include <QResizeEvent>
#include <QHBoxLayout>
#include <QToolButton>
#include <QSpacerItem>
#include <QKeyEvent>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PaneWidget.hpp"

#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetMemoryAddressRange.hpp"
#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"
#include "src/Targets/TargetState.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PanelWidget.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/SvgToolButton.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/Label.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TaskProgressIndicator/TaskProgressIndicator.hpp"

#include "src/Insight/InsightWorker/Tasks/InsightWorkerTask.hpp"
#include "src/Insight/InsightWorker/Tasks/ReadTargetMemory.hpp"

#include "HexViewerWidget/HexViewerWidget.hpp"
#include "MemoryRegionManager/MemoryRegionManagerWindow.hpp"
#include "SnapshotManager/SnapshotManager.hpp"

#include "TargetMemoryInspectionPaneSettings.hpp"

namespace Widgets
{
    class TargetMemoryInspectionPane: public PaneWidget
    {
        Q_OBJECT

    public:
        TargetMemoryInspectionPane(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            const Targets::TargetState& targetState,
            TargetMemoryInspectionPaneSettings& settings,
            PaneState& paneState,
            PanelWidget* parent
        );

        void refreshMemoryValues(std::optional<std::function<void(void)>> callback = std::nullopt);

    protected:
        void resizeEvent(QResizeEvent* event) override;
        void keyPressEvent(QKeyEvent* event) override;

        void postActivate();
        void postDeactivate();
        void postAttach();
        void postDetach();

    private:
        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor;
        const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor;
        const Targets::TargetDescriptor& targetDescriptor;
        const Targets::TargetState& targetState;

        TargetMemoryInspectionPaneSettings& settings;

        std::optional<Targets::TargetMemoryBuffer> data;
        std::optional<Targets::TargetStackPointer> stackPointer;
        std::optional<QSharedPointer<ReadTargetMemory>> activeRefreshTask;

        QWidget* container = nullptr;
        QHBoxLayout* subContainerLayout = nullptr;

        QWidget* titleBar = nullptr;
        QWidget* bottomBar = nullptr;
        QHBoxLayout* bottomBarLayout = nullptr;

        SvgToolButton* manageMemoryRegionsButton = nullptr;
        QToolButton* manageMemorySnapshotsButton = nullptr;

        SvgToolButton* refreshButton = nullptr;
        QAction* refreshOnTargetStopAction = nullptr;
        QAction* refreshOnActivationAction = nullptr;

        SvgToolButton* detachPaneButton = nullptr;
        SvgToolButton* attachPaneButton = nullptr;
        HexViewerWidget* hexViewerWidget = nullptr;

        PanelWidget* rightPanel = nullptr;
        SnapshotManager* snapshotManager = nullptr;

        TaskProgressIndicator* taskProgressIndicator = nullptr;
        QWidget* staleDataLabelContainer = nullptr;

        MemoryRegionManagerWindow* memoryRegionManagerWindow = nullptr;

        bool staleData = false;

        void sanitiseSettings();
        void onTargetStateChanged(Targets::TargetState newState, Targets::TargetState previousState);
        void setRefreshOnTargetStopEnabled(bool enabled);
        void setRefreshOnActivationEnabled(bool enabled);
        void onMemoryRead(const Targets::TargetMemoryBuffer& data);
        void openMemoryRegionManagerWindow();
        void toggleMemorySnapshotManagerPane();
        void onMemoryRegionsChange();
        void onTargetReset();
        void onProgrammingModeEnabled();
        void onProgrammingModeDisabled();
        void onTargetMemoryWritten(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            Targets::TargetMemoryAddressRange addressRange
        );
        void onSubtaskCreated(const QSharedPointer<InsightWorkerTask>& task);
        void onSnapshotRestored(const QString& snapshotId);
        void setStaleData(bool staleData);
    };
}
