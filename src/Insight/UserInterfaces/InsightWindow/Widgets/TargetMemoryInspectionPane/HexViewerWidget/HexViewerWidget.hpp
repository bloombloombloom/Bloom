#pragma once

#include <QWidget>
#include <QLabel>
#include <QGraphicsView>
#include <QVBoxLayout>
#include <set>
#include <map>
#include <QSize>
#include <QString>
#include <QEvent>
#include <optional>

#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetState.hpp"

#include "src/Insight/InsightWorker/InsightWorker.hpp"

#include "HexViewerWidgetSettings.hpp"
#include "ByteItemContainerGraphicsView.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/SvgToolButton.hpp"

namespace Bloom::Widgets
{
    class HexViewerWidget: public QWidget
    {
        Q_OBJECT

    public:
        QToolButton* refreshButton = nullptr;

        HexViewerWidget(
            const Targets::TargetMemoryDescriptor& targetMemoryDescriptor,
            InsightWorker& insightWorker,
            QWidget* parent
        );

        void updateValues(const Targets::TargetMemoryBuffer& buffer);

        void setStackPointer(std::uint32_t stackPointer);

    protected:
        void resizeEvent(QResizeEvent* event) override;

    private:
        const Targets::TargetMemoryDescriptor& targetMemoryDescriptor;
        InsightWorker& insightWorker;

        HexViewerWidgetSettings settings = HexViewerWidgetSettings();

        QWidget* container = nullptr;
        QWidget* toolBar = nullptr;
        QWidget* bottomBar = nullptr;

        ByteItemContainerGraphicsView* byteItemGraphicsView = nullptr;
        ByteItemGraphicsScene* byteItemGraphicsScene = nullptr;
        QLabel* hoveredAddressLabel = nullptr;

        SvgToolButton* highlightStackMemoryButton = nullptr;
        SvgToolButton* highlightFocusedMemoryButton = nullptr;
        SvgToolButton* highlightHoveredRowAndColumnButton = nullptr;

        Targets::TargetState targetState = Targets::TargetState::UNKNOWN;

    private slots:
        void onTargetStateChanged(Targets::TargetState newState);
        void onByteWidgetsAdjusted();
        void setStackMemoryHighlightingEnabled(bool enabled);
        void setHoveredRowAndColumnHighlightingEnabled(bool enabled);
        void setFocusedMemoryHighlightingEnabled(bool enabled);
    };
}
