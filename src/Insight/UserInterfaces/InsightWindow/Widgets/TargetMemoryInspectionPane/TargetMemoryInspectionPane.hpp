#pragma once

#include <QWidget>
#include <QResizeEvent>
#include <vector>

#include "src/Insight/InsightWorker/InsightWorker.hpp"

#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetState.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PanelWidget.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/SvgToolButton.hpp"

#include "HexViewerWidget/HexViewerWidget.hpp"

#include "MemoryRegionManager/MemoryRegionManagerWindow.hpp"
#include "MemoryRegion.hpp"
#include "FocusedMemoryRegion.hpp"
#include "ExcludedMemoryRegion.hpp"

namespace Bloom::Widgets
{
    class TargetMemoryInspectionPane: public QWidget
    {
        Q_OBJECT

    public:
        bool activated = false;

        TargetMemoryInspectionPane(
            const Targets::TargetMemoryDescriptor& targetMemoryDescriptor,
            InsightWorker& insightWorker,
            PanelWidget *parent
        );

        void refreshMemoryValues(std::optional<std::function<void(void)>> callback = std::nullopt);
        void activate();
        void deactivate();

    protected:
        void resizeEvent(QResizeEvent* event) override;

        virtual void postActivate();
        virtual void postDeactivate();

    private:
        const Targets::TargetMemoryDescriptor& targetMemoryDescriptor;
        InsightWorker& insightWorker;

        PanelWidget* parent = nullptr;
        QWidget* container = nullptr;

        QWidget* titleBar = nullptr;
        SvgToolButton* manageMemoryRegionsButton = nullptr;
        HexViewerWidget* hexViewerWidget = nullptr;

        Targets::TargetState targetState = Targets::TargetState::UNKNOWN;

        std::vector<FocusedMemoryRegion> focusedMemoryRegions;
        std::vector<ExcludedMemoryRegion> excludedMemoryRegions;
        MemoryRegionManagerWindow* memoryRegionManagerWindow = nullptr;

        void onTargetStateChanged(Targets::TargetState newState);
        void onMemoryRead(const Targets::TargetMemoryBuffer& buffer);
        void openMemoryRegionManagerWindow();
    };
}
