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

#include "HexViewerItemIndex.hpp"
#include "HexViewerItemRenderer.hpp"
#include "TopLevelGroupItem.hpp"
#include "GroupItem.hpp"
#include "ByteItem.hpp"
#include "ByteAddressContainer.hpp"
#include "ContextMenuAction.hpp"

#include "HexViewerSharedState.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemoryRegion.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/FocusedMemoryRegion.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/ExcludedMemoryRegion.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/AddressType.hpp"

namespace Widgets
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
        void highlightByteItems(const std::set<Targets::TargetMemoryAddress>& addresses);
        void rebuildItemHierarchy();
        void adjustSize();
        void setEnabled(bool enabled);
        void refreshValues();
        QPointF getByteItemPositionByAddress(Targets::TargetMemoryAddress address);
        void addExternalContextMenuAction(ContextMenuAction* action);

    signals:
        void ready();
        void hoveredAddress(const std::optional<Targets::TargetMemoryAddress>& address);
        void selectionChanged(
            const std::unordered_map<Targets::TargetMemoryAddress, ByteItem*>& selectedByteItemsByAddress
        );

    protected:
        bool enabled = true;

        HexViewerSharedState state;

        const std::vector<FocusedMemoryRegion>& focusedMemoryRegions;
        const std::vector<ExcludedMemoryRegion>& excludedMemoryRegions;

        std::unique_ptr<TopLevelGroupItem> topLevelGroup = nullptr;
        std::unique_ptr<HexViewerItemIndex> itemIndex = nullptr;

        HexViewerItemRenderer* renderer = nullptr;

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
        QAction* copyAbsoluteAddressAction = new QAction("Absolute Addresses", this);
        QAction* copyRelativeAddressAction = new QAction("Relative Addresses", this);
        QAction* copyHexValuesAction = new QAction("Values as Hex Stream", this);
        QAction* copyBinaryBitStringValues = new QAction("...as Binary Bit String", this);
        QAction* copyBinaryBitStringWithDelimitersValues = new QAction(
            "...as Prefixed Binary Bit Strings with Line Delimiters",
            this
        );
        QAction* copyHexValuesWithDelimitersAction = new QAction(
            "...as Prefixed Hex Strings with Line Delimiters",
            this
        );
        QAction* copyDecimalValuesAction = new QAction("...as Decimals with Line Delimiters", this);
        QAction* copyValueJsonMapAction = new QAction("...as Address to Value JSON Map", this);
        QAction* copyAsciiValuesAction = new QAction("...as ASCII String", this);

        // Address label container context menu actions
        QAction* displayRelativeAddressAction = new QAction("Relative", this);
        QAction* displayAbsoluteAddressAction = new QAction("Absolute", this);

        std::vector<ContextMenuAction*> externalContextMenuActions;

        int getSceneWidth() {
            /*
             * Minus 2 for the QSS margin on the vertical scrollbar (which isn't accounted for during viewport
             * size calculation).
             *
             * See https://bugreports.qt.io/browse/QTBUG-99189 for more on this.
             */
            return std::max(this->parent->viewport()->width(), 200) - 2;
        }

        virtual void initRenderer();
        virtual QMargins margins();
        virtual QPointF addressContainerPosition();
        bool event(QEvent* event) override;
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
        void mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
        void mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
        void mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
        void keyPressEvent(QKeyEvent* keyEvent) override;
        void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
        int getScrollbarValue();
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
        std::map<Targets::TargetMemoryAddress, ByteItem*> sortedByteItemsByAddress();
        void copyAddressesToClipboard(AddressType type);
        void copyHexValuesToClipboard(bool withDelimiters);
        void copyDecimalValuesToClipboard();
        void copyBinaryBitStringToClipboard(bool withDelimiters);
        void copyValueMappingToClipboard();
        void copyAsciiValueToClipboard();
    };
}
