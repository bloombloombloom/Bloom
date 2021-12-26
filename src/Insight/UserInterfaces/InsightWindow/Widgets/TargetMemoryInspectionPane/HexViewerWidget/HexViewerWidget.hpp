#pragma once

#include <QWidget>
#include <QLabel>
#include <QResizeEvent>
#include <QShowEvent>
#include <vector>

#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetState.hpp"

#include "src/Insight/InsightWorker/InsightWorker.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/SvgToolButton.hpp"

#include "HexViewerWidgetSettings.hpp"
#include "ByteItemContainerGraphicsView.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemoryRegion.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/FocusedMemoryRegion.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/ExcludedMemoryRegion.hpp"

namespace Bloom::Widgets
{
    class HexViewerWidget: public QWidget
    {
        Q_OBJECT

    public:
        SvgToolButton* refreshButton = nullptr;

        HexViewerWidget(
            const Targets::TargetMemoryDescriptor& targetMemoryDescriptor,
            HexViewerWidgetSettings& settings,
            std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
            std::vector<ExcludedMemoryRegion>& excludedMemoryRegions,
            InsightWorker& insightWorker,
            QWidget* parent
        );

        void updateValues(const Targets::TargetMemoryBuffer& buffer);
        void refreshRegions();
        void setStackPointer(std::uint32_t stackPointer);

    protected:
        void resizeEvent(QResizeEvent* event) override;
        void showEvent(QShowEvent* event) override;

    private:
        const Targets::TargetMemoryDescriptor& targetMemoryDescriptor;
        std::vector<FocusedMemoryRegion>& focusedMemoryRegions;
        std::vector<ExcludedMemoryRegion>& excludedMemoryRegions;

        InsightWorker& insightWorker;

        HexViewerWidgetSettings& settings;

        QWidget* container = nullptr;
        QWidget* toolBar = nullptr;
        QWidget* bottomBar = nullptr;

        QWidget* byteItemGraphicsViewContainer = nullptr;
        ByteItemContainerGraphicsView* byteItemGraphicsView = nullptr;
        ByteItemGraphicsScene* byteItemGraphicsScene = nullptr;
        QLabel* hoveredAddressLabel = nullptr;

        SvgToolButton* highlightStackMemoryButton = nullptr;
        SvgToolButton* highlightFocusedMemoryButton = nullptr;
        SvgToolButton* highlightHoveredRowAndColumnButton = nullptr;
        SvgToolButton* displayAnnotationsButton = nullptr;
        SvgToolButton* displayAsciiButton = nullptr;

        Targets::TargetState targetState = Targets::TargetState::UNKNOWN;

        void onTargetStateChanged(Targets::TargetState newState);
        void setStackMemoryHighlightingEnabled(bool enabled);
        void setHoveredRowAndColumnHighlightingEnabled(bool enabled);
        void setFocusedMemoryHighlightingEnabled(bool enabled);
        void setAnnotationsEnabled(bool enabled);
        void setDisplayAsciiEnabled(bool enabled);
    };
}
