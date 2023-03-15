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

#include "src/Targets/TargetState.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PanelWidget.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/SvgToolButton.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/Label.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TaskProgressIndicator/TaskProgressIndicator.hpp"

#include "src/Insight/InsightWorker/Tasks/CaptureMemorySnapshot.hpp"

#include "HexViewerWidget/HexViewerWidget.hpp"
#include "MemoryRegionManager/MemoryRegionManagerWindow.hpp"
#include "SnapshotManager/SnapshotManager.hpp"

#include "TargetMemoryInspectionPaneSettings.hpp"

namespace Bloom::Widgets
{
    class TargetMemoryInspectionPane: public PaneWidget
    {
        Q_OBJECT

    public:
        TargetMemoryInspectionPaneSettings& settings;

        TargetMemoryInspectionPane(
            const Targets::TargetMemoryDescriptor& targetMemoryDescriptor,
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
        const Targets::TargetMemoryDescriptor& targetMemoryDescriptor;

        std::optional<Targets::TargetMemoryBuffer> data;

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
        QSpacerItem* bottomBarHorizontalSpacer = nullptr;
        QWidget* staleDataLabelContainer = nullptr;

        Targets::TargetState targetState = Targets::TargetState::UNKNOWN;

        MemoryRegionManagerWindow* memoryRegionManagerWindow = nullptr;

        bool staleData = false;

        void sanitiseSettings();
        void onTargetStateChanged(Targets::TargetState newState);
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
            Bloom::Targets::TargetMemoryType memoryType,
            Targets::TargetMemoryAddressRange addressRange
        );
        void onCaptureMemoryTaskCreated(const QSharedPointer<CaptureMemorySnapshot>& task);
        void setTaskProgressIndicator(const QSharedPointer<InsightWorkerTask>& task);
        void setStaleData(bool staleData);
    };
}
