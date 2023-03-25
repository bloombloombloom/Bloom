#pragma once

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QScrollBar>
#include <optional>
#include <set>
#include <unordered_map>
#include <memory>
#include <vector>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QKeyEvent>
#include <QGraphicsRectItem>
#include <QPointF>
#include <QAction>
#include <QTimer>

#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetState.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/Label.hpp"

#include "GraphicsItem.hpp"
#include "TopLevelGroupItem.hpp"
#include "GroupItem.hpp"
#include "ByteItem.hpp"
#include "ByteAddressContainer.hpp"

#include "HexViewerSharedState.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemoryRegion.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/FocusedMemoryRegion.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/ExcludedMemoryRegion.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/AddressType.hpp"

namespace Bloom::Widgets
{
    class ItemGraphicsScene: public QGraphicsScene
    {
        Q_OBJECT

    public:
        ItemGraphicsScene(
            const Targets::TargetMemoryDescriptor& targetMemoryDescriptor,
            const std::optional<Targets::TargetMemoryBuffer>& data,
            const std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
            const std::vector<ExcludedMemoryRegion>& excludedMemoryRegions,
            HexViewerWidgetSettings& settings,
            QGraphicsView* parent
        );

        void init();
        void updateStackPointer(Targets::TargetStackPointer stackPointer);
        void selectByteItems(const std::set<Targets::TargetMemoryAddress>& addresses);
        void rebuildItemHierarchy();
        void adjustSize();
        void setEnabled(bool enabled);
        void refreshValues();
        QPointF getByteItemPositionByAddress(Targets::TargetMemoryAddress address);
        void allocateGraphicsItems();

    signals:
        void ready();
        void hoveredAddress(const std::optional<Targets::TargetMemoryAddress>& address);
        void selectionChanged(Targets::TargetMemorySize selectionCount);

    protected:
        bool event(QEvent* event) override;
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
        void mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
        void mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
        void mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
        void keyPressEvent(QKeyEvent* keyEvent) override;
        void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

    private:
        static constexpr auto GRID_SIZE = 100;

        bool enabled = true;

        HexViewerSharedState state;

        const std::vector<FocusedMemoryRegion>& focusedMemoryRegions;
        const std::vector<ExcludedMemoryRegion>& excludedMemoryRegions;

        std::unique_ptr<TopLevelGroupItem> topLevelGroup = nullptr;

        std::vector<HexViewerItem*> flattenedItems;
        std::vector<decltype(ItemGraphicsScene::flattenedItems)::iterator> gridPoints;
        std::vector<const ByteItem*> firstByteItemByLine;

        std::vector<GraphicsItem*> graphicsItems;

        const QMargins margins = QMargins(10, 10, 10, 10);

        Targets::TargetState targetState = Targets::TargetState::UNKNOWN;

        QGraphicsView* parent = nullptr;

        ByteAddressContainer* byteAddressContainer = nullptr;

        std::unordered_map<Targets::TargetMemoryAddress, ByteItem*> selectedByteItemsByAddress;

        QGraphicsRectItem* rubberBandRectItem = nullptr;
        std::optional<QPointF> rubberBandInitPoint = std::nullopt;

        QGraphicsRectItem* hoverRectX = new QGraphicsRectItem(QRect(0, 0, 0, ByteItem::HEIGHT));
        QGraphicsRectItem* hoverRectY = new QGraphicsRectItem(QRect(0, 0, ByteItem::WIDTH, 0));

        // Context menu actions
        QAction* selectAllByteItemsAction = new QAction("Select All", this);
        QAction* deselectByteItemsAction = new QAction("Deselect All", this);
        QAction* copyAbsoluteAddressAction = new QAction("Copy Absolute Addresses", this);
        QAction* copyRelativeAddressAction = new QAction("Copy Relative Addresses", this);
        QAction* copyHexValuesAction = new QAction("Copy Hexadecimal Values", this);
        QAction* copyDecimalValuesAction = new QAction("Copy Decimal Values", this);

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
            return std::max(this->parent->viewport()->width(), 200) - 2;
        }

        void refreshItemPositionIndices();
        void onTargetStateChanged(Targets::TargetState newState);
        void onByteItemEnter(ByteItem& byteItem);
        void onByteItemLeave();
        void clearSelectionRectItem();
        void selectByteItem(ByteItem& byteItem);
        void deselectByteItem(ByteItem& byteItem);
        void toggleByteItemSelection(ByteItem& byteItem);
        void clearByteItemSelection();
        void selectAllByteItems();
        void setAddressType(AddressType type);
        void copyAddressesToClipboard(AddressType type);
        void copyHexValuesToClipboard();
        void copyDecimalValuesToClipboard();
    };
}
