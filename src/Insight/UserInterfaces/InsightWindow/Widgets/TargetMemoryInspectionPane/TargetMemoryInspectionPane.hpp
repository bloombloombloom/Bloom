#pragma once

#include <QWidget>
#include <QResizeEvent>
#include <vector>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PaneWidget.hpp"

#include "src/Insight/InsightWorker/InsightWorker.hpp"
#include "src/Targets/TargetMemory.hpp"

#include "src/Targets/TargetState.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PanelWidget.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/SvgToolButton.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/Label.hpp"

#include "HexViewerWidget/HexViewerWidget.hpp"
#include "MemoryRegionManager/MemoryRegionManagerWindow.hpp"

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
            InsightWorker& insightWorker,
            PaneState& paneState,
            PanelWidget* parent
        );

        void refreshMemoryValues(std::optional<std::function<void(void)>> callback = std::nullopt);

    protected:
        void resizeEvent(QResizeEvent* event) override;

        void postActivate();
        void postDeactivate();
        void postAttach();
        void postDetach();

    private:
        const Targets::TargetMemoryDescriptor& targetMemoryDescriptor;
        InsightWorker& insightWorker;

        std::optional<Targets::TargetMemoryBuffer> data;

        QWidget* container = nullptr;

        QWidget* titleBar = nullptr;
        SvgToolButton* manageMemoryRegionsButton = nullptr;

        SvgToolButton* refreshButton = nullptr;
        QAction* refreshOnTargetStopAction = nullptr;
        QAction* refreshOnActivationAction = nullptr;

        SvgToolButton* detachPaneButton = nullptr;
        SvgToolButton* attachPaneButton = nullptr;
        HexViewerWidget* hexViewerWidget = nullptr;

        Targets::TargetState targetState = Targets::TargetState::UNKNOWN;

        MemoryRegionManagerWindow* memoryRegionManagerWindow = nullptr;

        void sanitiseSettings();
        void onTargetStateChanged(Targets::TargetState newState);
        void setRefreshOnTargetStopEnabled(bool enabled);
        void setRefreshOnActivationEnabled(bool enabled);
        void onMemoryRead(const Targets::TargetMemoryBuffer& data);
        void openMemoryRegionManagerWindow();
        void onMemoryRegionsChange();
        void onProgrammingModeEnabled();
        void onProgrammingModeDisabled();
    };
}
