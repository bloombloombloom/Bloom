#pragma once

#include <QWidget>
#include <QResizeEvent>
#include <QShowEvent>
#include <vector>

#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetState.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/Label.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/SvgToolButton.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TextInput.hpp"

#include "HexViewerWidgetSettings.hpp"
#include "ItemGraphicsView.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemoryRegion.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/FocusedMemoryRegion.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/ExcludedMemoryRegion.hpp"

namespace Bloom::Widgets
{
    class HexViewerWidget: public QWidget
    {
        Q_OBJECT

    public:
        HexViewerWidget(
            const Targets::TargetMemoryDescriptor& targetMemoryDescriptor,
            const std::optional<Targets::TargetMemoryBuffer>& data,
            HexViewerWidgetSettings& settings,
            std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
            std::vector<ExcludedMemoryRegion>& excludedMemoryRegions,
            QWidget* parent
        );

        void init();
        void updateValues();
        void refreshRegions();
        void setStackPointer(Targets::TargetStackPointer stackPointer);

    signals:
        void ready();

    protected:
        void resizeEvent(QResizeEvent* event) override;
        void showEvent(QShowEvent* event) override;

    private:
        const Targets::TargetMemoryDescriptor& targetMemoryDescriptor;
        const std::optional<Targets::TargetMemoryBuffer>& data;

        std::vector<FocusedMemoryRegion>& focusedMemoryRegions;
        std::vector<ExcludedMemoryRegion>& excludedMemoryRegions;

        HexViewerWidgetSettings& settings;

        QWidget* container = nullptr;
        QWidget* toolBar = nullptr;
        QWidget* bottomBar = nullptr;

        Label* loadingHexViewerLabel = nullptr;
        ItemGraphicsView* byteItemGraphicsView = nullptr;
        ItemGraphicsScene* byteItemGraphicsScene = nullptr;
        Label* hoveredAddressLabel = nullptr;

        SvgToolButton* highlightStackMemoryButton = nullptr;
        SvgToolButton* highlightFocusedMemoryButton = nullptr;
        SvgToolButton* highlightHoveredRowAndColumnButton = nullptr;
        SvgToolButton* displayAnnotationsButton = nullptr;
        SvgToolButton* displayAsciiButton = nullptr;

        TextInput* goToAddressInput = nullptr;

        Targets::TargetState targetState = Targets::TargetState::UNKNOWN;

        void onTargetStateChanged(Targets::TargetState newState);
        void setStackMemoryHighlightingEnabled(bool enabled);
        void setHoveredRowAndColumnHighlightingEnabled(bool enabled);
        void setFocusedMemoryHighlightingEnabled(bool enabled);
        void setAnnotationsEnabled(bool enabled);
        void setDisplayAsciiEnabled(bool enabled);
        void onGoToAddressInputChanged();
    };
}
