#include "ItemGraphicsScene.hpp"

#include <cmath>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <map>
#include <algorithm>

#include "src/Insight/InsightWorker/InsightWorker.hpp"
#include "src/Insight/InsightSignals.hpp"

#include "src/Insight/InsightWorker/Tasks/ConstructHexViewerTopLevelGroupItem.hpp"

namespace Widgets
{
    ItemGraphicsScene::ItemGraphicsScene(
        const Targets::TargetMemoryDescriptor& targetMemoryDescriptor,
        const std::optional<Targets::TargetMemoryBuffer>& data,
        const std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
        const std::vector<ExcludedMemoryRegion>& excludedMemoryRegions,
        HexViewerWidgetSettings& settings,
        QGraphicsView* parent
    )
        : QGraphicsScene(parent)
        , state(
            HexViewerSharedState(
                targetMemoryDescriptor,
                data,
                settings
            )
        )
        , focusedMemoryRegions(focusedMemoryRegions)
        , excludedMemoryRegions(excludedMemoryRegions)
        , parent(parent)
    {
        this->setObjectName("byte-widget-container");

        this->byteAddressContainer = new ByteAddressContainer(this->state);
        this->addItem(this->byteAddressContainer);

        this->displayRelativeAddressAction->setCheckable(true);
        this->displayAbsoluteAddressAction->setCheckable(true);

        this->setAddressType(this->state.settings.addressLabelType);
        this->setItemIndexMethod(QGraphicsScene::ItemIndexMethod::NoIndex);

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
            [this] {
                this->copyHexValuesToClipboard(false);
            }
        );

        QObject::connect(
            this->copyHexValuesWithDelimitersAction,
            &QAction::triggered,
            this,
            [this] {
                this->copyHexValuesToClipboard(true);
            }
        );

        QObject::connect(
            this->copyDecimalValuesAction,
            &QAction::triggered,
            this,
            &ItemGraphicsScene::copyDecimalValuesToClipboard
        );

        QObject::connect(
            this->copyBinaryBitStringValues,
            &QAction::triggered,
            this,
            [this] {
                this->copyBinaryBitStringToClipboard(false);
            }
        );

        QObject::connect(
            this->copyBinaryBitStringWithDelimitersValues,
            &QAction::triggered,
            this,
            [this] {
                this->copyBinaryBitStringToClipboard(true);
            }
        );

        QObject::connect(
            this->copyValueJsonMapAction,
            &QAction::triggered,
            this,
            &ItemGraphicsScene::copyValueMappingToClipboard
        );

        QObject::connect(
            this->copyAsciiValuesAction,
            &QAction::triggered,
            this,
            &ItemGraphicsScene::copyAsciiValueToClipboard
        );

        this->setSceneRect(0, 0, this->getSceneWidth(), 0);

        static const auto hoverRectBackgroundColor = QColor(0x8E, 0x8B, 0x83, 45);

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
    }

    void ItemGraphicsScene::init() {
        this->byteAddressContainer->setPos(this->addressContainerPosition());

        const auto constructHexViewerTopLevelGroupItem = QSharedPointer<ConstructHexViewerTopLevelGroupItem>(
            new ConstructHexViewerTopLevelGroupItem(
                this->focusedMemoryRegions,
                this->excludedMemoryRegions,
                this->state
            ),
            &QObject::deleteLater
        );

        QObject::connect(
            constructHexViewerTopLevelGroupItem.get(),
            &ConstructHexViewerTopLevelGroupItem::topLevelGroupItem,
            this,
            [this] (TopLevelGroupItem* item) {
                const auto margins = this->margins();

                this->topLevelGroup.reset(item);
                this->topLevelGroup->setPosition(
                    QPoint(ByteAddressContainer::WIDTH + margins.left(), margins.top())
                );

                this->itemIndex = std::make_unique<HexViewerItemIndex>(this->topLevelGroup.get(), this);
                this->initRenderer();

                emit this->ready();
            }
        );

        InsightWorker::queueTask(constructHexViewerTopLevelGroupItem);
    }

    void ItemGraphicsScene::updateStackPointer(std::uint32_t stackPointer) {
        this->state.currentStackPointer = stackPointer;
        this->rebuildItemHierarchy();
    }

    void ItemGraphicsScene::selectByteItems(const std::set<std::uint32_t>& addresses) {
        this->selectedByteItemsByAddress.clear();

        for (auto& [address, byteItem] : this->topLevelGroup->byteItemsByAddress) {
            if (addresses.contains(address)) {
                byteItem.selected = true;
                this->selectedByteItemsByAddress.insert(std::pair(byteItem.startAddress, &byteItem));

            } else if (byteItem.selected) {
                byteItem.selected = false;
            }
        }

        this->update();
        emit this->selectionChanged(this->selectedByteItemsByAddress);
    }

    void ItemGraphicsScene::highlightByteItems(const std::set<std::uint32_t>& addresses) {
        for (auto& [address, byteItem] : this->topLevelGroup->byteItemsByAddress) {
            byteItem.highlighted = addresses.contains(address);
        }

        this->state.highlightingEnabled = !addresses.empty();
        this->update();
    }

    void ItemGraphicsScene::rebuildItemHierarchy() {
        this->topLevelGroup->rebuildItemHierarchy();
        this->itemIndex->refreshFlattenedItems();
        this->adjustSize();
    }

    void ItemGraphicsScene::adjustSize() {
        const auto margins = this->margins();
        const auto width = this->getSceneWidth();
        const auto availableWidth = width - ByteAddressContainer::WIDTH - margins.left() - margins.right();

        auto hoverRectX = this->hoverRectX->rect();
        hoverRectX.setWidth(width);
        this->hoverRectX->setRect(hoverRectX);

        auto hoverRectY = this->hoverRectY->rect();
        hoverRectY.setHeight(this->views().first()->viewport()->height() + (ByteItem::HEIGHT * 2));
        this->hoverRectY->setRect(hoverRectY);

        this->topLevelGroup->adjustItemPositions(availableWidth);
        this->itemIndex->refreshIndex();

        const auto sceneSize = QSize(
            width,
            std::max(
                static_cast<int>(this->topLevelGroup->size().height())
                    + margins.top() + margins.bottom(),
                this->parent->height()
            )
        );
        this->setSceneRect(
            0,
            0,
            sceneSize.width(),
            sceneSize.height()
        );

        if (this->renderer != nullptr) {
            this->renderer->size = sceneSize;
        }

        this->byteAddressContainer->adjustAddressLabels(this->itemIndex->byteItemLines);
        this->update();
    }

    void ItemGraphicsScene::setEnabled(bool enabled) {
        if (this->enabled == enabled) {
            return;
        }

        this->enabled = enabled;

        if (this->renderer != nullptr) {
            this->renderer->setEnabled(this->enabled);
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

    void ItemGraphicsScene::addExternalContextMenuAction(ContextMenuAction* action) {
        QObject::connect(action, &QAction::triggered, this, [this, action] () {
            emit action->invoked(this->selectedByteItemsByAddress);
        });

        this->externalContextMenuActions.push_back(action);
    }

    void ItemGraphicsScene::initRenderer() {
        this->renderer = new HexViewerItemRenderer(
            this->state,
            *(this->itemIndex.get()),
            this->views().first()
        );
        this->renderer->setPos(0, 0);
        this->addItem(this->renderer);
    }

    QMargins ItemGraphicsScene::margins() {
        return QMargins(10, 10, 10, 10);
    }

    QPointF ItemGraphicsScene::addressContainerPosition() {
        return QPointF(0, 0);
    }

    bool ItemGraphicsScene::event(QEvent* event) {
        if (event->type() == QEvent::Type::GraphicsSceneLeave && this->state.hoveredByteItem != nullptr) {
            this->onByteItemLeave();
        }

        return QGraphicsScene::event(event);
    }

    void ItemGraphicsScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* mouseEvent) {
        this->mousePressEvent(mouseEvent);
    }

    void ItemGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) {
        static const auto rubberBandRectBackgroundColor = QColor(0x3C, 0x59, 0x5C, 0x82);
        static const auto rubberBandRectBorderColor = QColor(0x3C, 0x59, 0x5C, 255);

        const auto button = mouseEvent->button();
        const auto mousePosition = mouseEvent->buttonDownScenePos(button);

        if (this->state.highlightingEnabled) {
            this->state.highlightingEnabled = false;
            this->update();
        }

        if (mousePosition.x() <= this->byteAddressContainer->boundingRect().width()) {
            return;
        }

        this->update();

        if (button == Qt::MouseButton::RightButton) {
            ByteItem* clickedByteItem = this->itemIndex->byteItemAt(mousePosition);

            if (clickedByteItem == nullptr || clickedByteItem->selected) {
                return;
            }
        }

        const auto modifiers = mouseEvent->modifiers();
        if ((modifiers & Qt::ShiftModifier) == 0 && button == Qt::MouseButton::LeftButton) {
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

        auto* clickedByteItem = this->itemIndex->byteItemAt(mousePosition);
        if (clickedByteItem != nullptr) {
            if ((modifiers & Qt::ShiftModifier) != 0) {
                for (
                    auto i = static_cast<std::int64_t>(clickedByteItem->startAddress);
                    i >= this->state.memoryDescriptor.addressRange.startAddress;
                    --i
                ) {
                    auto& byteItem = this->topLevelGroup->byteItemsByAddress.at(
                        static_cast<Targets::TargetMemoryAddress>(i)
                    );

                    if (byteItem.selected) {
                        break;
                    }

                    this->toggleByteItemSelection(byteItem);
                }

                emit this->selectionChanged(this->selectedByteItemsByAddress);
                return;
            }

            this->toggleByteItemSelection(*clickedByteItem);
            emit this->selectionChanged(this->selectedByteItemsByAddress);
        }
    }

    void ItemGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) {
        const auto mousePosition = mouseEvent->scenePos();

        if (this->rubberBandRectItem != nullptr && this->rubberBandInitPoint.has_value()) {
            this->update();
            const auto oldRect = this->rubberBandRectItem->rect();

            this->rubberBandRectItem->setRect(
                qMin(mousePosition.x(), this->rubberBandInitPoint->x()),
                qMin(mousePosition.y(), this->rubberBandInitPoint->y()),
                qMax(static_cast<int>(qAbs(mousePosition.x() - this->rubberBandInitPoint->x())), 1),
                qMax(static_cast<int>(qAbs(mousePosition.y() - this->rubberBandInitPoint->y())), 1)
            );

            if ((mouseEvent->modifiers() & Qt::ControlModifier) == 0) {
                this->clearByteItemSelection();

            } else {
                const auto oldItems = this->itemIndex->intersectingByteItems(oldRect);
                for (auto* byteItem : oldItems) {
                    if (byteItem->selected) {
                        this->deselectByteItem(*byteItem);
                    }
                }
            }

            const auto items = this->itemIndex->intersectingByteItems(this->rubberBandRectItem->rect());
            for (auto& byteItem : items) {
                this->selectByteItem(*byteItem);
            }
            emit this->selectionChanged(this->selectedByteItemsByAddress);
        }

        auto* hoveredByteItem = this->itemIndex->byteItemAt(mousePosition);
        if (hoveredByteItem != nullptr) {
            this->onByteItemEnter(*hoveredByteItem);
            return;
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

        if (key == Qt::Key_Escape && !this->selectedByteItemsByAddress.empty()) {
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
            menu->setLayoutDirection(Qt::LayoutDirection::LeftToRight);
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
        menu->setLayoutDirection(Qt::LayoutDirection::LeftToRight);
        menu->addAction(this->selectAllByteItemsAction);
        menu->addAction(this->deselectByteItemsAction);
        menu->addSeparator();

        auto* copyMenu = new QMenu("Copy Selection", menu);
        copyMenu->addAction(this->copyAbsoluteAddressAction);
        copyMenu->addAction(this->copyRelativeAddressAction);
        copyMenu->addSeparator();
        copyMenu->addAction(this->copyHexValuesAction);
        copyMenu->addAction(this->copyHexValuesWithDelimitersAction);
        copyMenu->addAction(this->copyDecimalValuesAction);
        copyMenu->addAction(this->copyAsciiValuesAction);
        copyMenu->addAction(this->copyBinaryBitStringValues);
        copyMenu->addAction(this->copyBinaryBitStringWithDelimitersValues);
        copyMenu->addAction(this->copyValueJsonMapAction);

        copyMenu->setEnabled(itemsSelected);
        this->deselectByteItemsAction->setEnabled(itemsSelected);

        menu->addMenu(copyMenu);

        if (!this->externalContextMenuActions.empty()) {
            menu->addSeparator();

            for (auto& externalAction : this->externalContextMenuActions) {
                menu->addAction(externalAction);
                externalAction->setEnabled(
                    itemsSelected
                    && (
                        !externalAction->isEnabledCallback.has_value()
                        || externalAction->isEnabledCallback.value()(this->selectedByteItemsByAddress)
                    )

                );
            }
        }

        menu->exec(event->screenPos());
    }

    int ItemGraphicsScene::getScrollbarValue() {
        return this->views().first()->verticalScrollBar()->value();
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
        this->update();

        if (this->state.settings.highlightHoveredRowAndCol) {
            const auto byteItemScenePos = byteItem.position();
            this->hoverRectX->setPos(0, byteItemScenePos.y());
            this->hoverRectY->setPos(
                byteItemScenePos.x(),
                std::max(this->getScrollbarValue() - ByteItem::HEIGHT, 0)
            );

            this->hoverRectX->setVisible(true);
            this->hoverRectY->setVisible(true);
            this->hoverRectX->update();
            this->hoverRectY->update();
        }

        emit this->hoveredAddress(byteItem.startAddress);
    }

    void ItemGraphicsScene::onByteItemLeave() {
        if (this->state.hoveredByteItem != nullptr) {
            this->state.hoveredByteItem = nullptr;
        }

        this->hoverRectX->setVisible(false);
        this->hoverRectY->setVisible(false);
        this->hoverRectX->update();
        this->hoverRectY->update();
        this->update();

        emit this->hoveredAddress(std::nullopt);
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
        emit this->selectionChanged(this->selectedByteItemsByAddress);
    }

    void ItemGraphicsScene::selectAllByteItems() {
        for (auto& [address, byteItem] : this->topLevelGroup->byteItemsByAddress) {
            byteItem.selected = true;
            this->selectedByteItemsByAddress.insert(std::pair(byteItem.startAddress, &byteItem));
        }

        this->update();
        emit this->selectionChanged(this->selectedByteItemsByAddress);
    }

    void ItemGraphicsScene::setAddressType(AddressType type) {
        this->state.settings.addressLabelType = type;

        this->displayRelativeAddressAction->setChecked(this->state.settings.addressLabelType == AddressType::RELATIVE);
        this->displayAbsoluteAddressAction->setChecked(this->state.settings.addressLabelType == AddressType::ABSOLUTE);

        this->byteAddressContainer->invalidateChildItemCaches();
    }

    std::map<Targets::TargetMemoryAddress, ByteItem*> ItemGraphicsScene::sortedByteItemsByAddress() {
        auto sortedByteItemsByAddress = std::map<Targets::TargetMemoryAddress, ByteItem*>();
        std::transform(
            this->selectedByteItemsByAddress.begin(),
            this->selectedByteItemsByAddress.end(),
            std::inserter(sortedByteItemsByAddress, sortedByteItemsByAddress.end()),
            [] (const decltype(this->selectedByteItemsByAddress)::value_type& pair) {
                return pair;
            }
        );

        return sortedByteItemsByAddress;
    }

    void ItemGraphicsScene::copyAddressesToClipboard(AddressType type) {
        if (this->selectedByteItemsByAddress.empty()) {
            return;
        }

        auto data = QString();
        const auto memoryStartAddress = this->state.memoryDescriptor.addressRange.startAddress;

        for (const auto& [address, byteItem] : this->sortedByteItemsByAddress()) {
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

    void ItemGraphicsScene::copyHexValuesToClipboard(bool withDelimiters) {
        if (this->selectedByteItemsByAddress.empty()) {
            return;
        }

        auto data = QString();

        for (const auto& [address, byteItem] : this->sortedByteItemsByAddress()) {
            const unsigned char byteValue = byteItem->excluded
                ? 0x00
                : (*this->state.data)[byteItem->startAddress - this->state.memoryDescriptor.addressRange.startAddress];

            data.append(
                withDelimiters
                    ? "0x" + QString::number(byteValue, 16).rightJustified(2, '0').toUpper() + "\n"
                    : QString::number(byteValue, 16).rightJustified(2, '0').toUpper()
            );
        }

        QApplication::clipboard()->setText(std::move(data));
    }

    void ItemGraphicsScene::copyDecimalValuesToClipboard() {
        if (this->selectedByteItemsByAddress.empty() || !this->state.data.has_value()) {
            return;
        }

        auto data = QString();

        for (const auto& [address, byteItem] : this->sortedByteItemsByAddress()) {
            const unsigned char byteValue = byteItem->excluded
                ? 0x00
                : (*this->state.data)[byteItem->startAddress - this->state.memoryDescriptor.addressRange.startAddress];
            data.append(QString::number(byteValue, 10) + "\n");
        }

        QApplication::clipboard()->setText(std::move(data));
    }

    void ItemGraphicsScene::copyBinaryBitStringToClipboard(bool withDelimiters) {
        if (this->selectedByteItemsByAddress.empty()) {
            return;
        }

        auto data = QString();

        for (const auto& [address, byteItem] : this->sortedByteItemsByAddress()) {
            const unsigned char byteValue = byteItem->excluded
                ? 0x00
                : (*this->state.data)[byteItem->startAddress - this->state.memoryDescriptor.addressRange.startAddress];

            data.append(
                withDelimiters
                    ? "0b" + QString::number(byteValue, 2).rightJustified(8, '0') + "\n"
                    : QString::number(byteValue, 2).rightJustified(8, '0') + " "
            );
        }

        QApplication::clipboard()->setText(std::move(data));
    }

    void ItemGraphicsScene::copyValueMappingToClipboard() {
        if (this->selectedByteItemsByAddress.empty() || !this->state.data.has_value()) {
            return;
        }

        auto data = QJsonObject();

        for (const auto& [address, byteItem] : this->sortedByteItemsByAddress()) {
            const unsigned char byteValue = byteItem->excluded
                ? 0x00
                : (*this->state.data)[byteItem->startAddress - this->state.memoryDescriptor.addressRange.startAddress];

            data.insert(
                "0x" + QString::number(address, 16).rightJustified(8, '0').toUpper(),
                "0x" + QString::number(byteValue, 16).rightJustified(2, '0').toUpper()
            );
        }

        QApplication::clipboard()->setText(QJsonDocument(data).toJson(QJsonDocument::JsonFormat::Indented));
    }

    void ItemGraphicsScene::copyAsciiValueToClipboard() {
        if (this->selectedByteItemsByAddress.empty() || !this->state.data.has_value()) {
            return;
        }

        auto data = QString();

        for (const auto& [address, byteItem] : this->sortedByteItemsByAddress()) {
            const unsigned char byteValue =
                (*this->state.data)[byteItem->startAddress - this->state.memoryDescriptor.addressRange.startAddress];

            if (byteItem->excluded || byteValue < 32 || byteValue > 126) {
                continue;
            }

            data.append(QChar(byteValue));
        }

        QApplication::clipboard()->setText(std::move(data));
    }
}
