#include "ByteItemGraphicsScene.hpp"

#include <cmath>

namespace Bloom::Widgets
{
    using Bloom::Targets::TargetMemoryDescriptor;

    ByteItemGraphicsScene::ByteItemGraphicsScene(
        const TargetMemoryDescriptor& targetMemoryDescriptor,
        std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
        std::vector<ExcludedMemoryRegion>& excludedMemoryRegions,
        InsightWorker& insightWorker,
        const HexViewerWidgetSettings& settings,
        Label* hoveredAddressLabel,
        QGraphicsView* parent
    )
        : QGraphicsScene(parent)
        , targetMemoryDescriptor(targetMemoryDescriptor)
        , focusedMemoryRegions(focusedMemoryRegions)
        , excludedMemoryRegions(excludedMemoryRegions)
        , insightWorker(insightWorker)
        , settings(settings)
        , hoveredAddressLabel(hoveredAddressLabel)
        , parent(parent)
    {
        this->setObjectName("byte-widget-container");

        this->byteAddressContainer = new ByteAddressContainer();
        this->addItem(this->byteAddressContainer);

        // Construct ByteWidget objects
        const auto memorySize = this->targetMemoryDescriptor.size();
        const auto startAddress = this->targetMemoryDescriptor.addressRange.startAddress;
        for (std::uint32_t i = 0; i < memorySize; i++) {
            const auto address = startAddress + i;

            auto* byteWidget = new ByteItem(
                i,
                address,
                this->currentStackPointer,
                &(this->hoveredByteWidget),
                &(this->hoveredAnnotationItem),
                this->highlightedByteItems,
                settings
            );

            this->byteItemsByAddress.insert(std::pair(
                address,
                byteWidget
            ));

            this->addItem(byteWidget);
        }

        QObject::connect(
            &insightWorker,
            &InsightWorker::targetStateUpdated,
            this,
            &ByteItemGraphicsScene::onTargetStateChanged
        );

        this->refreshRegions();
        this->adjustSize();
    }

    void ByteItemGraphicsScene::updateValues(const Targets::TargetMemoryBuffer& buffer) {
        for (auto& [address, byteWidget] : this->byteItemsByAddress) {
            byteWidget->setValue(buffer.at(byteWidget->byteIndex));
        }

        this->updateAnnotationValues(buffer);
        this->lastValueBuffer = buffer;
    }

    void ByteItemGraphicsScene::updateStackPointer(std::uint32_t stackPointer) {
        this->currentStackPointer = stackPointer;
        this->invalidateChildItemCaches();
    }

    void ByteItemGraphicsScene::setHighlightedAddresses(const std::set<std::uint32_t>& highlightedAddresses) {
        this->highlightedByteItems.clear();

        for (auto& [address, byteItem] : this->byteItemsByAddress) {
            if (highlightedAddresses.contains(address)) {
                byteItem->highlighted = true;
                this->highlightedByteItems.insert(byteItem);

            } else {
                byteItem->highlighted = false;
            }

            byteItem->update();
        }
    }

    void ByteItemGraphicsScene::refreshRegions() {
        for (auto& [byteAddress, byteWidget] : this->byteItemsByAddress) {
            byteWidget->focusedMemoryRegion = nullptr;
            byteWidget->excludedMemoryRegion = nullptr;

            for (const auto& focusedRegion : this->focusedMemoryRegions) {
                if (byteAddress >= focusedRegion.addressRange.startAddress
                    && byteAddress <= focusedRegion.addressRange.endAddress
                ) {
                    byteWidget->focusedMemoryRegion = &focusedRegion;
                    break;
                }
            }

            for (const auto& excludedRegion : this->excludedMemoryRegions) {
                if (byteAddress >= excludedRegion.addressRange.startAddress
                    && byteAddress <= excludedRegion.addressRange.endAddress
                ) {
                    byteWidget->excludedMemoryRegion = &excludedRegion;
                    break;
                }
            }

            byteWidget->update();
        }

        // Refresh annotation items
        this->hoveredAnnotationItem = nullptr;
        for (auto* annotationItem : this->annotationItems) {
            this->removeItem(annotationItem);
            delete annotationItem;
        }

        this->annotationItems.clear();
        this->valueAnnotationItems.clear();

        for (const auto& focusedRegion : this->focusedMemoryRegions) {
            auto* annotationItem = new AnnotationItem(focusedRegion, AnnotationItemPosition::BOTTOM);
            annotationItem->setEnabled(this->enabled);
            this->addItem(annotationItem);
            this->annotationItems.emplace_back(annotationItem);

            if (focusedRegion.dataType != MemoryRegionDataType::UNKNOWN) {
                auto* valueAnnotationItem = new ValueAnnotationItem(focusedRegion);
                valueAnnotationItem->setEnabled(this->enabled);
                this->addItem(valueAnnotationItem);
                this->annotationItems.emplace_back(valueAnnotationItem);
                this->valueAnnotationItems.emplace_back(valueAnnotationItem);
            }
        }

        if (this->targetState == Targets::TargetState::STOPPED && this->enabled && !this->lastValueBuffer.empty()) {
            this->updateAnnotationValues(this->lastValueBuffer);
        }

        this->adjustSize(true);
    }

    void ByteItemGraphicsScene::adjustSize(bool forced) {
        const auto width = this->getSceneWidth();

        const auto columnCount = static_cast<std::size_t>(
            std::floor(
                (width - this->margins.left() - this->margins.right() - ByteAddressContainer::WIDTH
                + ByteItem::RIGHT_MARGIN) / (ByteItem::WIDTH + ByteItem::RIGHT_MARGIN)
            )
        );
        const auto rowCount = static_cast<int>(
            std::ceil(static_cast<double>(this->byteItemsByAddress.size()) / static_cast<double>(columnCount))
        );

        // Don't bother recalculating the byte & annotation positions if the number of rows & columns haven't changed.
        if (this->byteItemsByAddress.empty()
            || (
                !forced
                && rowCount == this->byteItemsByRowIndex.size()
                && columnCount == this->byteItemsByColumnIndex.size()
            )
        ) {
            this->setSceneRect(
                0,
                0,
                width,
                std::max(static_cast<int>(this->sceneRect().height()), this->parent->viewport()->height())
            );

            return;
        }

        if (!this->byteItemsByAddress.empty()) {
            this->adjustByteItemPositions();

            if (this->settings.displayAnnotations) {
                this->adjustAnnotationItemPositions();
            }

            const auto* lastByteItem = (--this->byteItemsByAddress.end())->second;
            const auto sceneHeight = static_cast<int>(
                lastByteItem->pos().y() + ByteItem::HEIGHT + AnnotationItem::BOTTOM_HEIGHT + this->margins.bottom()
            );

            this->setSceneRect(
                0,
                0,
                width,
                std::max(sceneHeight, this->parent->height())
            );
        }
    }

    void ByteItemGraphicsScene::setEnabled(bool enabled) {
        if (this->enabled != enabled) {
            this->enabled = enabled;

            for (auto& [byteAddress, byteItem] : this->byteItemsByAddress) {
                byteItem->setEnabled(this->enabled);
            }

            for (auto* annotationItem : this->annotationItems) {
                annotationItem->setEnabled(this->enabled);
            }

            this->byteAddressContainer->setEnabled(enabled);
            this->byteAddressContainer->update();
            this->update();
        }
    }

    void ByteItemGraphicsScene::invalidateChildItemCaches() {
        for (auto& [address, byteWidget] : this->byteItemsByAddress) {
            byteWidget->update();
        }

        for (auto* annotationItem : this->annotationItems) {
            annotationItem->update();
        }
    }

    QPointF ByteItemGraphicsScene::getByteItemPositionByAddress(std::uint32_t address) {
        if (this->byteItemsByAddress.contains(address)) {
            return this->byteItemsByAddress.at(address)->pos();
        }

        return QPointF();
    }

    bool ByteItemGraphicsScene::event(QEvent* event) {
        if (event->type() == QEvent::Type::GraphicsSceneLeave && this->hoveredByteWidget != nullptr) {
            this->onByteWidgetLeave();
        }

        return QGraphicsScene::event(event);
    }

    void ByteItemGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) {
        auto hoveredItems = this->items(mouseEvent->scenePos());
        ByteItem* hoveredByteItem = nullptr;
        AnnotationItem* hoveredAnnotationItem = nullptr;

        if (!hoveredItems.empty()) {
            hoveredByteItem = dynamic_cast<ByteItem*>(hoveredItems.at(0));
            hoveredAnnotationItem = dynamic_cast<AnnotationItem*>(hoveredItems.at(0));
        }

        if (hoveredByteItem != nullptr) {
            this->onByteWidgetEnter(hoveredByteItem);
            return;
        }

        if (this->hoveredByteWidget != nullptr) {
            this->onByteWidgetLeave();
        }

        if (hoveredAnnotationItem != nullptr) {
            this->onAnnotationItemEnter(hoveredAnnotationItem);
            return;
        }

        if (this->hoveredAnnotationItem != nullptr) {
            this->onAnnotationItemLeave();
        }
    }

    void ByteItemGraphicsScene::updateAnnotationValues(const Targets::TargetMemoryBuffer& buffer) {
        const auto memoryStartAddress = this->targetMemoryDescriptor.addressRange.startAddress;
        for (auto* valueAnnotationItem : this->valueAnnotationItems) {
            if (valueAnnotationItem->size > buffer.size()) {
                continue;
            }

            const auto relativeStartAddress = valueAnnotationItem->startAddress - memoryStartAddress;
            const auto relativeEndAddress = valueAnnotationItem->endAddress - memoryStartAddress;

            valueAnnotationItem->setValue(Targets::TargetMemoryBuffer(
                buffer.begin() + relativeStartAddress,
                buffer.begin() + relativeEndAddress + 1
            ));
        }
    }

    void ByteItemGraphicsScene::adjustByteItemPositions() {
        const auto columnCount = static_cast<std::size_t>(
            std::floor(
                (this->getSceneWidth() - this->margins.left() - this->margins.right() - ByteAddressContainer::WIDTH
                + ByteItem::RIGHT_MARGIN) / (ByteItem::WIDTH + ByteItem::RIGHT_MARGIN)
            )
        );

        std::map<std::size_t, std::vector<ByteItem*>> byteWidgetsByRowIndex;
        std::map<std::size_t, std::vector<ByteItem*>> byteWidgetsByColumnIndex;

        auto rowIndicesWithTopAnnotations = std::set<std::size_t>();
        auto rowIndicesWithBottomAnnotations = std::set<std::size_t>();
        const auto& memoryAddressRange = this->targetMemoryDescriptor.addressRange;

        for (auto* annotationItem : this->annotationItems) {
            const auto firstByteRowIndex = static_cast<std::size_t>(
                std::ceil(static_cast<double>((annotationItem->startAddress - memoryAddressRange.startAddress) + 1)
                    / static_cast<double>(columnCount)) - 1
            );

            const auto lastByteRowIndex = static_cast<std::size_t>(
                std::ceil(static_cast<double>((annotationItem->endAddress - memoryAddressRange.startAddress) + 1)
                    / static_cast<double>(columnCount)) - 1
            );

            // We only display annotations that span a single row.
            if (this->settings.displayAnnotations && firstByteRowIndex == lastByteRowIndex) {
                annotationItem->show();

                if (annotationItem->position == AnnotationItemPosition::TOP) {
                    rowIndicesWithTopAnnotations.insert(firstByteRowIndex);

                } else if (annotationItem->position == AnnotationItemPosition::BOTTOM) {
                    rowIndicesWithBottomAnnotations.insert(firstByteRowIndex);
                }

            } else {
                annotationItem->hide();
            }
        }

        constexpr auto annotationTopHeight = AnnotationItem::TOP_HEIGHT;
        constexpr auto annotationBottomHeight = AnnotationItem::BOTTOM_HEIGHT;

        std::size_t lastRowIndex = 0;
        int rowYPosition = margins.top();
        auto currentRowAnnotatedTop = false;

        for (auto& [address, byteWidget] : this->byteItemsByAddress) {
            const auto rowIndex = static_cast<std::size_t>(
                std::ceil(static_cast<double>(byteWidget->byteIndex + 1) / static_cast<double>(columnCount)) - 1
            );
            const auto columnIndex = static_cast<std::size_t>(
                static_cast<double>(byteWidget->byteIndex)
                    - (std::floor(byteWidget->byteIndex / columnCount) * static_cast<double>(columnCount))
            );

            if (rowIndex != lastRowIndex) {
                rowYPosition += ByteItem::HEIGHT + ByteItem::BOTTOM_MARGIN;
                currentRowAnnotatedTop = false;

                if (rowIndicesWithBottomAnnotations.contains(lastRowIndex)) {
                    rowYPosition += annotationBottomHeight;
                }
            }

            if (!currentRowAnnotatedTop && rowIndicesWithTopAnnotations.contains(rowIndex)) {
                rowYPosition += annotationTopHeight;
                currentRowAnnotatedTop = true;
            }

            byteWidget->setPos(
                static_cast<int>(
                    columnIndex * (ByteItem::WIDTH + ByteItem::RIGHT_MARGIN) + this->margins.left()
                        + ByteAddressContainer::WIDTH
                ),
                rowYPosition
            );

            byteWidget->currentRowIndex = rowIndex;
            byteWidget->currentColumnIndex = columnIndex;

            byteWidgetsByRowIndex[byteWidget->currentRowIndex].emplace_back(byteWidget);
            byteWidgetsByColumnIndex[byteWidget->currentColumnIndex].emplace_back(byteWidget);

            lastRowIndex = rowIndex;
        }

        this->byteItemsByRowIndex = std::move(byteWidgetsByRowIndex);
        this->byteItemsByColumnIndex = std::move(byteWidgetsByColumnIndex);

        this->byteAddressContainer->adjustAddressLabels(this->byteItemsByRowIndex);
    }

    void ByteItemGraphicsScene::adjustAnnotationItemPositions() {
        if (this->byteItemsByAddress.empty() || !this->settings.displayAnnotations) {
            return;
        }

        for (auto* annotationItem : this->annotationItems) {
            if (!this->byteItemsByAddress.contains(annotationItem->startAddress)) {
                annotationItem->hide();
                continue;
            }

            const auto firstByteItemPosition = this->byteItemsByAddress.at(annotationItem->startAddress)->pos();

            if (annotationItem->position == AnnotationItemPosition::TOP) {
                annotationItem->setPos(
                    firstByteItemPosition.x(),
                    firstByteItemPosition.y() - AnnotationItem::TOP_HEIGHT - 1
                );

            } else if (annotationItem->position == AnnotationItemPosition::BOTTOM) {
                annotationItem->setPos(
                    firstByteItemPosition.x(),
                    firstByteItemPosition.y() + ByteItem::HEIGHT
                );
            }
        }
    }

    void ByteItemGraphicsScene::onTargetStateChanged(Targets::TargetState newState) {
        using Targets::TargetState;
        this->targetState = newState;
    }

    void ByteItemGraphicsScene::onByteWidgetEnter(ByteItem* widget) {
        if (this->hoveredByteWidget != nullptr) {
            if (this->hoveredByteWidget == widget) {
                // This byte item is already marked as hovered
                return;
            }

            this->onByteWidgetLeave();
        }

        this->hoveredByteWidget = widget;

        this->hoveredAddressLabel->setText(
            "Relative Address / Absolute Address: " + widget->relativeAddressHex + " / " + widget->addressHex
        );

        if (this->settings.highlightHoveredRowAndCol && !this->byteItemsByRowIndex.empty()) {
            for (auto& byteWidget : this->byteItemsByColumnIndex.at(widget->currentColumnIndex)) {
                byteWidget->update();
            }

            for (auto& byteWidget : this->byteItemsByRowIndex.at(widget->currentRowIndex)) {
                byteWidget->update();
            }

        } else {
            widget->update();
        }
    }

    void ByteItemGraphicsScene::onByteWidgetLeave() {
        auto* byteItem = this->hoveredByteWidget;
        this->hoveredByteWidget = nullptr;

        this->hoveredAddressLabel->setText("Relative Address / Absolute Address:");

        if (this->settings.highlightHoveredRowAndCol && !this->byteItemsByRowIndex.empty()) {
            for (auto& byteWidget : this->byteItemsByColumnIndex.at(byteItem->currentColumnIndex)) {
                byteWidget->update();
            }

            for (auto& byteWidget : this->byteItemsByRowIndex.at(byteItem->currentRowIndex)) {
                byteWidget->update();
            }

        } else {
            byteItem->update();
        }
    }

    void ByteItemGraphicsScene::onAnnotationItemEnter(AnnotationItem* annotationItem) {
        if (this->hoveredAnnotationItem != nullptr) {
            if (this->hoveredAnnotationItem == annotationItem) {
                return;
            }

            this->onAnnotationItemLeave();
        }

        this->hoveredAnnotationItem = annotationItem;

        for (
            auto byteItemAddress = annotationItem->startAddress;
            byteItemAddress <= annotationItem->endAddress;
            byteItemAddress++
        ) {
            this->byteItemsByAddress.at(byteItemAddress)->update();
        }
    }

    void ByteItemGraphicsScene::onAnnotationItemLeave() {
        auto* annotationItem = this->hoveredAnnotationItem;
        this->hoveredAnnotationItem = nullptr;

        for (
            auto byteItemAddress = annotationItem->startAddress;
            byteItemAddress <= annotationItem->endAddress;
            byteItemAddress++
        ) {
            this->byteItemsByAddress.at(byteItemAddress)->update();
        }
    }
}
