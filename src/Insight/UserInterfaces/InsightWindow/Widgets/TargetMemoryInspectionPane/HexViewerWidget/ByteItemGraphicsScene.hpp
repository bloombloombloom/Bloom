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
#include <QGraphicsSceneContextMenuEvent>
#include <QKeyEvent>
#include <optional>
#include <QGraphicsRectItem>
#include <QPointF>
#include <QAction>

#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetState.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/Label.hpp"

#include "ByteItem.hpp"
#include "ByteAddressContainer.hpp"
#include "AnnotationItem.hpp"
#include "ValueAnnotationItem.hpp"
#include "HexViewerWidgetSettings.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemoryRegion.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/FocusedMemoryRegion.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/ExcludedMemoryRegion.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/AddressType.hpp"

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
            HexViewerWidgetSettings& settings,
            Label* hoveredAddressLabel,
            QGraphicsView* parent
        );

        void init();
        void updateValues(const Targets::TargetMemoryBuffer& buffer);
        void updateStackPointer(Targets::TargetStackPointer stackPointer);
        void setHighlightedAddresses(const std::set<Targets::TargetMemoryAddress>& highlightedAddresses);
        void refreshRegions();
        void adjustSize(bool forced = false);
        void setEnabled(bool enabled);
        void invalidateChildItemCaches();
        QPointF getByteItemPositionByAddress(Targets::TargetMemoryAddress address);

    signals:
        void ready();
        void byteWidgetsAdjusted();

    protected:
        bool event(QEvent* event) override;
        void mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
        void mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
        void mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
        void keyPressEvent(QKeyEvent* keyEvent) override;
        void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

    private:
        const Targets::TargetMemoryDescriptor& targetMemoryDescriptor;
        std::vector<FocusedMemoryRegion>& focusedMemoryRegions;
        std::vector<ExcludedMemoryRegion>& excludedMemoryRegions;

        bool enabled = true;

        ByteItem* hoveredByteWidget = nullptr;
        AnnotationItem* hoveredAnnotationItem = nullptr;

        std::optional<Targets::TargetStackPointer> currentStackPointer;

        Targets::TargetMemoryBuffer lastValueBuffer;

        std::map<Targets::TargetMemoryAddress, ByteItem*> byteItemsByAddress;
        std::vector<AnnotationItem*> annotationItems;
        std::vector<ValueAnnotationItem*> valueAnnotationItems;
        std::map<std::size_t, std::vector<ByteItem*>> byteItemsByRowIndex;
        std::map<std::size_t, std::vector<ByteItem*>> byteItemsByColumnIndex;

        Targets::TargetState targetState = Targets::TargetState::UNKNOWN;

        const QMargins margins = QMargins(10, 10, 10, 10);
        HexViewerWidgetSettings& settings;

        QGraphicsView* parent = nullptr;
        Label* hoveredAddressLabel = nullptr;

        ByteAddressContainer* byteAddressContainer = nullptr;

        std::set<ByteItem*> highlightedByteItems;
        std::map<Targets::TargetMemoryAddress, ByteItem*> selectedByteItemsByAddress;

        QGraphicsRectItem* rubberBandRectItem = nullptr;
        std::optional<QPointF> rubberBandInitPoint = std::nullopt;


        // Address label container context menu actions
        QAction* displayRelativeAddressAction = new QAction("Relative", this);
        QAction* displayAbsoluteAddressAction = new QAction("Absolute", this);

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
        void clearSelectionRectItem();
        void selectByteItem(ByteItem* byteItem);
        void deselectByteItem(ByteItem* byteItem);
        void toggleByteItemSelection(ByteItem* byteItem);
        void clearByteItemSelection();
        void selectAllByteItems();
        void setAddressType(AddressType type);
    };
}
