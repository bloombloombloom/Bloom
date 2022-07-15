#pragma once

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QScrollBar>
#include <QWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <map>
#include <algorithm>
#include <vector>
#include <QSize>
#include <QString>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <optional>

#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetState.hpp"

#include "src/Insight/InsightWorker/InsightWorker.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/Label.hpp"

#include "ByteItem.hpp"
#include "ByteAddressContainer.hpp"
#include "AnnotationItem.hpp"
#include "ValueAnnotationItem.hpp"
#include "HexViewerWidgetSettings.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemoryRegion.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/FocusedMemoryRegion.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/ExcludedMemoryRegion.hpp"

namespace Bloom::Widgets
{
    class ByteItemGraphicsScene: public QGraphicsScene
    {
        Q_OBJECT

    public:
        ByteItemGraphicsScene(
            const Targets::TargetMemoryDescriptor& targetMemoryDescriptor,
            std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
            std::vector<ExcludedMemoryRegion>& excludedMemoryRegions,
            InsightWorker& insightWorker,
            const HexViewerWidgetSettings& settings,
            Label* hoveredAddressLabel,
            QGraphicsView* parent
        );

        void updateValues(const Targets::TargetMemoryBuffer& buffer);
        void updateStackPointer(std::uint32_t stackPointer);
        void setHighlightedAddresses(const std::set<std::uint32_t>& highlightedAddresses);
        void refreshRegions();
        void adjustSize(bool forced = false);
        void setEnabled(bool enabled);
        void invalidateChildItemCaches();
        QPointF getByteItemPositionByAddress(std::uint32_t address);

    signals:
        void byteWidgetsAdjusted();

    protected:
        bool event(QEvent* event) override;
        void mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

    private:
        const Targets::TargetMemoryDescriptor& targetMemoryDescriptor;
        std::vector<FocusedMemoryRegion>& focusedMemoryRegions;
        std::vector<ExcludedMemoryRegion>& excludedMemoryRegions;

        bool enabled = true;

        ByteItem* hoveredByteWidget = nullptr;
        AnnotationItem* hoveredAnnotationItem = nullptr;

        std::optional<std::uint32_t> currentStackPointer;

        Targets::TargetMemoryBuffer lastValueBuffer;

        std::map<std::uint32_t, ByteItem*> byteItemsByAddress;
        std::vector<AnnotationItem*> annotationItems;
        std::vector<ValueAnnotationItem*> valueAnnotationItems;
        std::map<std::size_t, std::vector<ByteItem*>> byteItemsByRowIndex;
        std::map<std::size_t, std::vector<ByteItem*>> byteItemsByColumnIndex;

        Targets::TargetState targetState = Targets::TargetState::UNKNOWN;
        InsightWorker& insightWorker;

        const QMargins margins = QMargins(10, 10, 10, 10);
        const HexViewerWidgetSettings& settings;

        QGraphicsView* parent = nullptr;
        Label* hoveredAddressLabel = nullptr;

        ByteAddressContainer* byteAddressContainer = nullptr;

        std::set<std::uint32_t> highlightedAddresses;

        int getSceneWidth() {
            /*
             * Minus 2 for the QSS margin on the vertical scrollbar (which isn't accounted for during viewport
             * size calculation).
             *
             * See https://bugreports.qt.io/browse/QTBUG-99189 for more on this.
             */
            return std::max(this->parent->viewport()->width(), 400) - 2;
        }

        void updateAnnotationValues(const Targets::TargetMemoryBuffer& buffer);
        void adjustByteItemPositions();
        void adjustAnnotationItemPositions();
        void onTargetStateChanged(Targets::TargetState newState);
        void onByteWidgetEnter(Bloom::Widgets::ByteItem* widget);
        void onByteWidgetLeave();
        void onAnnotationItemEnter(Bloom::Widgets::AnnotationItem* annotationItem);
        void onAnnotationItemLeave();
    };
}
