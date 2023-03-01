#include "ItemGraphicsScene.hpp"

#include <cmath>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QByteArray>

#include "src/Insight/InsightWorker/InsightWorker.hpp"
#include "src/Insight/InsightSignals.hpp"

#include "src/Insight/InsightWorker/Tasks/ConstructHexViewerTopLevelGroupItem.hpp"

namespace Bloom::Widgets
{
    ItemGraphicsScene::ItemGraphicsScene(
        const Targets::TargetMemoryDescriptor& targetMemoryDescriptor,
        const std::optional<Targets::TargetMemoryBuffer>& data,
        std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
        std::vector<ExcludedMemoryRegion>& excludedMemoryRegions,
        HexViewerWidgetSettings& settings,
        Label* hoveredAddressLabel,
        QGraphicsView* parent
    )
        : state(
            HexViewerSharedState(
                targetMemoryDescriptor,
                data,
                settings
            )
        )
        , focusedMemoryRegions(focusedMemoryRegions)
        , excludedMemoryRegions(excludedMemoryRegions)
        , hoveredAddressLabel(hoveredAddressLabel)
        , parent(parent)
        , QGraphicsScene(parent)
    {
        this->setObjectName("byte-widget-container");

        this->byteAddressContainer = new ByteAddressContainer(this->state);
        this->addItem(this->byteAddressContainer);

        this->displayRelativeAddressAction->setCheckable(true);
        this->displayAbsoluteAddressAction->setCheckable(true);

        this->setAddressType(this->state.settings.addressLabelType);

        QObject::connect(
            InsightSignals::instance(),
            &InsightSignals::targetStateUpdated,
            this,
            &ItemGraphicsScene::onTargetStateChanged
        );

        QObject::connect(
            this->displayRelativeAddressAction,
            &QAction::triggered,
            this,
            [this] {
                this->setAddressType(AddressType::RELATIVE);
            }
        );

        QObject::connect(
            this->displayAbsoluteAddressAction,
            &QAction::triggered,
            this,
            [this] {
                this->setAddressType(AddressType::ABSOLUTE);
            }
        );

        QObject::connect(
            this->selectAllByteItemsAction,
            &QAction::triggered,
            this,
            &ItemGraphicsScene::selectAllByteItems
        );

        QObject::connect(
            this->deselectByteItemsAction,
            &QAction::triggered,
            this,
            &ItemGraphicsScene::clearByteItemSelection
        );

        QObject::connect(
            this->copyAbsoluteAddressAction,
            &QAction::triggered,
            this,
            [this] {
                this->copyAddressesToClipboard(AddressType::ABSOLUTE);
            }
        );

        QObject::connect(
            this->copyRelativeAddressAction,
            &QAction::triggered,
            this,
            [this] {
                this->copyAddressesToClipboard(AddressType::RELATIVE);
            }
        );

        QObject::connect(
            this->copyHexValuesAction,
            &QAction::triggered,
            this,
            &ItemGraphicsScene::copyHexValuesToClipboard
        );

        QObject::connect(
            this->copyDecimalValuesAction,
            &QAction::triggered,
            this,
            &ItemGraphicsScene::copyDecimalValuesToClipboard
        );

        this->setSceneRect(0, 0, this->getSceneWidth(), 0);

        static const auto hoverRectBackgroundColor = QColor(0x8E, 0x8B, 0x83, 30);

        this->hoverRectX->setBrush(hoverRectBackgroundColor);
        this->hoverRectY->setBrush(hoverRectBackgroundColor);
        this->hoverRectX->setPen(Qt::NoPen);
        this->hoverRectY->setPen(Qt::NoPen);

        this->hoverRectX->setVisible(false);
        this->hoverRectY->setVisible(false);

        this->hoverRectX->setZValue(100);
        this->hoverRectY->setZValue(100);

        this->addItem(this->hoverRectX);
        this->addItem(this->hoverRectY);
        this->setItemIndexMethod(QGraphicsScene::NoIndex);
    }

    void ItemGraphicsScene::init() {
        auto* constructHexViewerTopLevelGroupItem = new ConstructHexViewerTopLevelGroupItem(this->focusedMemoryRegions, this->state);

        QObject::connect(
            constructHexViewerTopLevelGroupItem,
            &ConstructHexViewerTopLevelGroupItem::topLevelGroupItem,
            this,
            [this] (TopLevelGroupItem* item) {
                this->topLevelGroup.reset(item);
                this->topLevelGroup->setPosition(QPoint(ByteAddressContainer::WIDTH + this->margins.left(), this->margins.top()));
                this->flattenedItems = this->topLevelGroup->flattenedItems();
                emit this->ready();
            }
        );

        InsightWorker::queueTask(constructHexViewerTopLevelGroupItem);

        auto* vScrollBar = this->views().first()->verticalScrollBar();
        vScrollBar->setSingleStep((ByteItem::HEIGHT + (ByteItem::BOTTOM_MARGIN / 2)));
    }

    void ItemGraphicsScene::updateStackPointer(std::uint32_t stackPointer) {
        this->state.currentStackPointer = stackPointer;
        this->update();
    }

    void ItemGraphicsScene::selectByteItems(const std::set<std::uint32_t>& addresses) {
        for (auto& [address, byteItem] : this->topLevelGroup->byteItemsByAddress) {
            if (addresses.contains(address)) {
                byteItem.selected = true;

            } else if (byteItem.selected) {
                byteItem.selected = false;
            }
        }

        this->update();
    }

    void ItemGraphicsScene::refreshRegions() {
        this->topLevelGroup->rebuildItemHierarchy();
        this->flattenedItems = this->topLevelGroup->flattenedItems();
        this->adjustSize();
        return;
    }

    void ItemGraphicsScene::adjustSize() {
        const auto width = this->getSceneWidth();
        const auto availableWidth = width - ByteAddressContainer::WIDTH - this->margins.left() - this->margins.right();

        auto hoverRectX = this->hoverRectX->rect();
        hoverRectX.setWidth(width);
        this->hoverRectX->setRect(hoverRectX);

        auto hoverRectY = this->hoverRectY->rect();
        hoverRectY.setHeight(this->views().first()->viewport()->height() + (ByteItem::HEIGHT * 2));
        this->hoverRectY->setRect(hoverRectY);

        this->topLevelGroup->adjustItemPositions(availableWidth);

        this->setSceneRect(
            0,
            0,
            width,
            std::max(
                static_cast<int>(this->topLevelGroup->size().height())
                    + this->margins.top() + this->margins.bottom(),
                this->parent->height()
            )
        );

        this->refreshItemPositionIndices();

        this->byteAddressContainer->adjustAddressLabels(this->firstByteItemByLine);

        const auto* view = this->views().first();
        const auto itemsRequired = static_cast<std::uint32_t>(
            (availableWidth / (ByteItem::WIDTH + (ByteItem::RIGHT_MARGIN / 2)))
                * (
                    (view->viewport()->height() + (4 * ItemGraphicsScene::GRID_SIZE))
                    / (ByteItem::HEIGHT + (ByteItem::BOTTOM_MARGIN / 2))
                )
        );

        while (this->graphicsItems.size() < itemsRequired) {
            auto* item = new GraphicsItem(&(this->state));
            item->setEnabled(this->enabled);
            this->graphicsItems.push_back(item);
            this->addItem(item);
        }

        this->allocateGraphicsItems();
        this->update();
    }

    void ItemGraphicsScene::setEnabled(bool enabled) {
        if (this->enabled == enabled) {
            return;
        }

        this->enabled = enabled;

        for (auto& graphicsItem : this->graphicsItems) {
            graphicsItem->setEnabled(this->enabled);
        }

        this->byteAddressContainer->setEnabled(enabled);
        this->byteAddressContainer->update();
        this->update();
    }

    void ItemGraphicsScene::refreshValues() {
        this->topLevelGroup->refreshValues();
        this->update();
    }

    QPointF ItemGraphicsScene::getByteItemPositionByAddress(std::uint32_t address) {
        const auto byteItemIt = this->topLevelGroup->byteItemsByAddress.find(address);
        if (byteItemIt != this->topLevelGroup->byteItemsByAddress.end()) {
            return byteItemIt->second.position();
        }

        return QPointF();
    }

    void ItemGraphicsScene::allocateGraphicsItems() {
        const auto* view = this->views().first();
        const auto verticalScrollBarValue = view->verticalScrollBar()->value();

        constexpr auto bufferPointSize = 2;
        const auto gridPointIndex = static_cast<decltype(this->gridPoints)::size_type>(std::max(
            static_cast<int>(
                std::floor(
                    static_cast<float>(verticalScrollBarValue) / static_cast<float>(ItemGraphicsScene::GRID_SIZE)
                )
            ) - 1 - bufferPointSize,
            0
        ));

        // Sanity check
        assert(this->gridPoints.size() > gridPointIndex);

        const auto& allocatableGraphicsItems = this->graphicsItems;
        auto allocatableGraphicsItemsCount = allocatableGraphicsItems.size();

        const auto allocateRangeStartItemIt = this->gridPoints[gridPointIndex];
        const auto allocateRangeEndItemIt = allocateRangeStartItemIt + std::min(
            std::distance(allocateRangeStartItemIt, this->flattenedItems.end() - 1),
            static_cast<long>(allocatableGraphicsItemsCount)
        );

        const auto& firstItem = *allocateRangeStartItemIt;
        const auto& lastItem = *allocateRangeEndItemIt;

        /*
         * Ensure that a graphics item for each parent, grandparent, etc. is allocated for the first item in the
         * allocatable range.
         */
        auto* parentItem = firstItem->parent;

        while (
            parentItem != nullptr
            && parentItem != this->topLevelGroup.get()
            && allocatableGraphicsItemsCount > 0
        ) {
            allocatableGraphicsItems[allocatableGraphicsItemsCount - 1]->setHexViewerItem(parentItem);
            --allocatableGraphicsItemsCount;
            parentItem = parentItem->parent;
        }

        for (auto itemIt = allocateRangeStartItemIt; itemIt != allocateRangeEndItemIt; ++itemIt) {
            if (allocatableGraphicsItemsCount < 1) {
                // No more graphics items available to allocate
                break;
            }

            allocatableGraphicsItems[allocatableGraphicsItemsCount - 1]->setHexViewerItem(*itemIt);
            --allocatableGraphicsItemsCount;
        }

        // If we still have some available graphics items, clear them
        while (allocatableGraphicsItemsCount > 0) {
            allocatableGraphicsItems[allocatableGraphicsItemsCount - 1]->setHexViewerItem(nullptr);
            --allocatableGraphicsItemsCount;
        }

        this->update();
    }

    bool ItemGraphicsScene::event(QEvent* event) {
        if (event->type() == QEvent::Type::GraphicsSceneLeave && this->state.hoveredByteItem != nullptr) {
            this->onByteItemLeave();
        }

        return QGraphicsScene::event(event);
    }

    void ItemGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) {
        static const auto rubberBandRectBackgroundColor = QColor(0x3C, 0x59, 0x5C, 0x82);
        static const auto rubberBandRectBorderColor = QColor(0x3C, 0x59, 0x5C, 255);

        if (mouseEvent->button() != Qt::MouseButton::LeftButton) {
            return;
        }

        const auto mousePosition = mouseEvent->buttonDownScenePos(Qt::MouseButton::LeftButton);

        if (mousePosition.x() <= this->byteAddressContainer->boundingRect().width()) {
            return;
        }

        this->update();
        const auto modifiers = mouseEvent->modifiers();
        if ((modifiers & Qt::ShiftModifier) == 0) {
            this->clearSelectionRectItem();

            this->rubberBandInitPoint = std::move(mousePosition);
            this->rubberBandRectItem = new QGraphicsRectItem(
                this->rubberBandInitPoint->x(),
                this->rubberBandInitPoint->y(),
                1,
                1
            );
            this->rubberBandRectItem->setBrush(rubberBandRectBackgroundColor);
            this->rubberBandRectItem->setPen(rubberBandRectBorderColor);
            this->addItem(this->rubberBandRectItem);
        }

        if ((modifiers & (Qt::ControlModifier | Qt::ShiftModifier)) == 0) {
            this->clearByteItemSelection();
        }

        for (const auto& item : this->items(mousePosition)) {
            auto* clickedGraphicsItem = dynamic_cast<GraphicsItem*>(item);

            if (clickedGraphicsItem == nullptr) {
                continue;
            }

            auto* byteItem = dynamic_cast<ByteItem*>(clickedGraphicsItem->hexViewerItem);

            if (byteItem == nullptr) {
                continue;
            }

            if ((modifiers & Qt::ShiftModifier) != 0) {
                for (
                    auto i = byteItem->startAddress;
                    i >= this->state.memoryDescriptor.addressRange.startAddress;
                    --i
                ) {
                    auto& byteItem = this->topLevelGroup->byteItemsByAddress.at(i);

                    if (byteItem.selected) {
                        break;
                    }

                    this->toggleByteItemSelection(byteItem);
                }

                return;
            }

            this->toggleByteItemSelection(*byteItem);
            break;
        }
    }

    void ItemGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) {
        const auto mousePosition = mouseEvent->scenePos();
        auto hoveredItems = this->items(mousePosition);

        if (this->rubberBandRectItem != nullptr && this->rubberBandInitPoint.has_value()) {
            const auto oldRect = this->rubberBandRectItem->rect();

            this->rubberBandRectItem->setRect(
                qMin(mousePosition.x(), this->rubberBandInitPoint->x()),
                qMin(mousePosition.y(), this->rubberBandInitPoint->y()),
                qAbs(mousePosition.x() - this->rubberBandInitPoint->x()),
                qAbs(mousePosition.y() - this->rubberBandInitPoint->y())
            );

            if ((mouseEvent->modifiers() & Qt::ControlModifier) == 0) {
                this->clearByteItemSelection();

            } else {
                const auto oldItems = this->items(oldRect, Qt::IntersectsItemShape);
                for (auto* item : oldItems) {
                    auto* graphicsItem = dynamic_cast<GraphicsItem*>(item);

                    if (graphicsItem == nullptr) {
                        continue;
                    }

                    auto* byteItem = dynamic_cast<ByteItem*>(graphicsItem->hexViewerItem);

                    if (byteItem != nullptr && byteItem->selected) {
                        this->deselectByteItem(*byteItem);
                    }
                }
            }

            const auto items = this->items(this->rubberBandRectItem->rect(), Qt::IntersectsItemShape);
            for (auto* item : items) {
                auto* graphicsItem = dynamic_cast<GraphicsItem*>(item);

                if (graphicsItem == nullptr) {
                    continue;
                }

                auto* byteItem = dynamic_cast<ByteItem*>(graphicsItem->hexViewerItem);

                if (byteItem != nullptr && !byteItem->selected) {
                    this->selectByteItem(*byteItem);
                }
            }
        }

        for (const auto& item : hoveredItems) {
            auto* hoveredGraphicsItem = dynamic_cast<GraphicsItem*>(item);

            if (hoveredGraphicsItem == nullptr) {
                continue;
            }

            auto* hoveredByteItem = dynamic_cast<ByteItem*>(hoveredGraphicsItem->hexViewerItem);

            if (hoveredByteItem != nullptr) {
                this->onByteItemEnter(*hoveredByteItem);
                return;
            }
        }

        if (this->state.hoveredByteItem != nullptr) {
            this->onByteItemLeave();
        }
    }

    void ItemGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) {
        this->clearSelectionRectItem();
        this->update();
    }

    void ItemGraphicsScene::keyPressEvent(QKeyEvent* keyEvent) {
        const auto key = keyEvent->key();

        if (key == Qt::Key_Escape) {
            this->clearByteItemSelection();
            return;
        }

        const auto modifiers = keyEvent->modifiers();
        if ((modifiers & Qt::ControlModifier) != 0 && key == Qt::Key_A) {
            this->selectAllByteItems();
            return;
        }

        QGraphicsScene::keyPressEvent(keyEvent);
    }

    void ItemGraphicsScene::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
        if (event->scenePos().x() <= ByteAddressContainer::WIDTH) {
            auto* menu = new QMenu(this->parent);
            menu->setObjectName("byte-item-address-container-context-menu");

            auto* addressTypeMenu = new QMenu("Address Type", menu);
            addressTypeMenu->addAction(this->displayAbsoluteAddressAction);
            addressTypeMenu->addAction(this->displayRelativeAddressAction);
            menu->addMenu(addressTypeMenu);

            menu->exec(event->screenPos());
            return;
        }

        const auto itemsSelected = !this->selectedByteItemsByAddress.empty();

        auto* menu = new QMenu(this->parent);
        menu->addAction(this->selectAllByteItemsAction);
        menu->addAction(this->deselectByteItemsAction);
        menu->addSeparator();

        auto* copyMenu = new QMenu("Copy Selected", menu);
        copyMenu->addAction(this->copyAbsoluteAddressAction);
        copyMenu->addAction(this->copyRelativeAddressAction);
        copyMenu->addSeparator();
        copyMenu->addAction(this->copyHexValuesAction);
        copyMenu->addAction(this->copyDecimalValuesAction);

        copyMenu->setEnabled(itemsSelected);
        this->deselectByteItemsAction->setEnabled(itemsSelected);

        menu->addMenu(copyMenu);
        menu->exec(event->screenPos());
    }

    void ItemGraphicsScene::refreshItemPositionIndices() {
        const auto pointsRequired = static_cast<std::uint32_t>(
            this->sceneRect().height() / ItemGraphicsScene::GRID_SIZE
        );

        this->gridPoints.clear();
        this->gridPoints.reserve(pointsRequired);

        this->firstByteItemByLine.clear();

        auto currentPoint = 0;
        auto currentLineYPosition = 0;
        for (auto itemIt = this->flattenedItems.begin(); itemIt != this->flattenedItems.end(); ++itemIt) {
            auto& item = *itemIt;

            if (item->allocatedGraphicsItem != nullptr) {
                item->allocatedGraphicsItem->setPos(item->position());
            }

            const auto byteItem = dynamic_cast<const ByteItem*>(item);

            if (byteItem == nullptr) {
                continue;
            }

            const auto itemYStartPosition = byteItem->position().y();
            const auto itemYEndPosition = itemYStartPosition + byteItem->size().height();

            if (itemYStartPosition > currentLineYPosition) {
                this->firstByteItemByLine.push_back(byteItem);
                currentLineYPosition = itemYStartPosition;
            }

            if (itemYEndPosition >= currentPoint) {
                // This byte item is the first to exceed or intersect with the currentPoint
                this->gridPoints.push_back(itemIt);
                currentPoint += ItemGraphicsScene::GRID_SIZE;
            }
        }
    }

    void ItemGraphicsScene::onTargetStateChanged(Targets::TargetState newState) {
        this->targetState = newState;
    }

    void ItemGraphicsScene::onByteItemEnter(ByteItem& byteItem) {
        if (this->state.hoveredByteItem != nullptr) {
            if (this->state.hoveredByteItem == &byteItem) {
                // This byteItem is already marked as hovered
                return;
            }

            this->onByteItemLeave();
        }

        this->state.hoveredByteItem = &byteItem;

        const auto addressHex = "0x" + QString::number(
            byteItem.startAddress,
            16
        ).rightJustified(8, '0').toUpper();

        const auto relativeAddress = byteItem.startAddress - this->state.memoryDescriptor.addressRange.startAddress;
        const auto relativeAddressHex = "0x" + QString::number(
            relativeAddress,
            16
        ).rightJustified(8, '0').toUpper();

        this->hoveredAddressLabel->setText(
            "Relative Address / Absolute Address: " + relativeAddressHex + " / " + addressHex
        );

        if (this->state.settings.highlightHoveredRowAndCol) {
            const auto byteItemScenePos = byteItem.position();
            this->hoverRectX->setPos(0, byteItemScenePos.y());
            this->hoverRectY->setPos(
                byteItemScenePos.x(),
                std::max(this->views().first()->verticalScrollBar()->value() - ByteItem::HEIGHT, 0)
            );

            this->hoverRectX->setVisible(true);
            this->hoverRectY->setVisible(true);
        }

        this->update();
    }

    void ItemGraphicsScene::onByteItemLeave() {
        if (this->state.hoveredByteItem != nullptr) {
            this->state.hoveredByteItem = nullptr;
        }

        this->hoveredAddressLabel->setText("Relative Address / Absolute Address:");

        this->hoverRectX->setVisible(false);
        this->hoverRectY->setVisible(false);
        this->update();
    }

    void ItemGraphicsScene::clearSelectionRectItem() {
        if (this->rubberBandRectItem != nullptr) {
            this->removeItem(this->rubberBandRectItem);
            delete this->rubberBandRectItem;
            this->rubberBandRectItem = nullptr;
        }
    }

    void ItemGraphicsScene::selectByteItem(ByteItem& byteItem) {
        byteItem.selected = true;
        this->selectedByteItemsByAddress.insert(std::pair(byteItem.startAddress, &byteItem));
    }

    void ItemGraphicsScene::deselectByteItem(ByteItem& byteItem) {
        byteItem.selected = false;
        this->selectedByteItemsByAddress.erase(byteItem.startAddress);
    }

    void ItemGraphicsScene::toggleByteItemSelection(ByteItem& byteItem) {
        if (byteItem.selected) {
            this->deselectByteItem(byteItem);
            return;
        }

        this->selectByteItem(byteItem);
    }

    void ItemGraphicsScene::clearByteItemSelection() {
        for (auto& [address, byteItem] : this->selectedByteItemsByAddress) {
            byteItem->selected = false;
        }

        this->selectedByteItemsByAddress.clear();
        this->update();
    }

    void ItemGraphicsScene::selectAllByteItems() {
        for (auto& [address, byteItem] : this->topLevelGroup->byteItemsByAddress) {
            byteItem.selected = true;
            this->selectedByteItemsByAddress.insert(std::pair(byteItem.startAddress, &byteItem));
//            this->selectByteItem(byteItem);
        }

        this->update();
    }

    void ItemGraphicsScene::setAddressType(AddressType type) {
        this->state.settings.addressLabelType = type;

        this->displayRelativeAddressAction->setChecked(this->state.settings.addressLabelType == AddressType::RELATIVE);
        this->displayAbsoluteAddressAction->setChecked(this->state.settings.addressLabelType == AddressType::ABSOLUTE);

        this->byteAddressContainer->invalidateChildItemCaches();
    }

    void ItemGraphicsScene::copyAddressesToClipboard(AddressType type) {
        if (this->selectedByteItemsByAddress.empty()) {
            return;
        }

        auto data = QString();
        const auto memoryStartAddress = this->state.memoryDescriptor.addressRange.startAddress;

        for (const auto& [address, byteItem] : this->selectedByteItemsByAddress) {
            data.append(
                "0x" + QString::number(
                    type == AddressType::RELATIVE
                        ? byteItem->startAddress - memoryStartAddress
                        : byteItem->startAddress,
                    16
                ).rightJustified(8, '0').toUpper() + "\n"
            );
        }

        QApplication::clipboard()->setText(std::move(data));
    }

    void ItemGraphicsScene::copyHexValuesToClipboard() {
        if (this->selectedByteItemsByAddress.empty()) {
            return;
        }

        auto data = QString();

        for (const auto& [address, byteItem] : this->selectedByteItemsByAddress) {
            const auto byteIndex = byteItem->startAddress - this->state.memoryDescriptor.addressRange.startAddress;
            data.append("0x" + QString::number((*this->state.data)[byteIndex], 16).rightJustified(2, '0').toUpper() + "\n");
        }

        QApplication::clipboard()->setText(std::move(data));
    }

    void ItemGraphicsScene::copyDecimalValuesToClipboard() {
        if (this->selectedByteItemsByAddress.empty() || !this->state.data.has_value()) {
            return;
        }

        auto data = QString();

        for (const auto& [address, byteItem] : this->selectedByteItemsByAddress) {
            const auto byteIndex = byteItem->startAddress - this->state.memoryDescriptor.addressRange.startAddress;
            data.append(QString::number((*this->state.data)[byteIndex], 10) + "\n");
        }

        QApplication::clipboard()->setText(std::move(data));
    }
}
