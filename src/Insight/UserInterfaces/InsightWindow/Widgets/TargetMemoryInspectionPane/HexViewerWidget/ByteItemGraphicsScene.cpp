#include "ByteItemGraphicsScene.hpp"

#include <QVBoxLayout>
#include <QTableWidget>
#include <QScrollBar>
#include <QPainter>
#include <cmath>

#include "src/Logger/Logger.hpp"

using namespace Bloom::Widgets;
using namespace Bloom::Exceptions;

using Bloom::Targets::TargetMemoryDescriptor;

ByteItemGraphicsScene::ByteItemGraphicsScene(
    const TargetMemoryDescriptor& targetMemoryDescriptor,
    InsightWorker& insightWorker,
    const HexViewerWidgetSettings& settings,
    QLabel* hoveredAddressLabel,
    QWidget* parent
): QGraphicsScene(parent),
targetMemoryDescriptor(targetMemoryDescriptor),
insightWorker(insightWorker),
settings(settings),
hoveredAddressLabel(hoveredAddressLabel),
parent(parent) {
    this->setObjectName("byte-widget-container");

    this->byteAddressContainer = new ByteAddressContainer();
    this->addItem(this->byteAddressContainer);

    /*
     * Construct ByteWidget objects
     *
     * No need to position them here - the subsequent call to resizeEvent() will do that.
     */
    const auto memorySize = this->targetMemoryDescriptor.size();
    const auto startAddress = this->targetMemoryDescriptor.addressRange.startAddress;
    Logger::error("Constructing bytes begin");
    for (std::uint32_t i = 0; i < memorySize; i++) {
        const auto address = startAddress + i;

        auto* byteWidget = new ByteItem(i, address, this->hoveredByteWidget, settings);
        this->byteItemsByAddress.insert(std::pair(
            address,
            byteWidget
        ));

        this->addItem(byteWidget);
    }
    Logger::error("Constructing bytes end");
    this->adjustByteWidgets();

    QObject::connect(
        &insightWorker,
        &InsightWorker::targetStateUpdated,
        this,
        &ByteItemGraphicsScene::onTargetStateChanged
    );
}

void ByteItemGraphicsScene::updateValues(const Targets::TargetMemoryBuffer& buffer) {
    for (auto& [address, byteWidget] : this->byteItemsByAddress) {
        byteWidget->setValue(buffer.at(byteWidget->byteIndex));
        byteWidget->update();
    }
}

void ByteItemGraphicsScene::adjustByteWidgets() {
    const auto margins = QMargins(10, 10, 10, 10);
    const auto width = std::max(600, static_cast<int>(this->parent->width()));

    constexpr auto byteWidgetWidth = ByteItem::WIDTH + ByteItem::RIGHT_MARGIN;
    constexpr auto byteWidgetHeight = ByteItem::HEIGHT + ByteItem::BOTTOM_MARGIN;
    const auto rowCapacity = static_cast<std::size_t>(
        std::floor((width - margins.left() - margins.right() - ByteAddressContainer::WIDTH) / byteWidgetWidth)
    );
    const auto rowCount = static_cast<int>(
        std::ceil(static_cast<double>(this->byteItemsByAddress.size()) / static_cast<double>(rowCapacity))
    );

    this->setSceneRect(
        0,
        0,
        width,
        std::max(((rowCount * byteWidgetHeight) + margins.top() + margins.bottom()), this->parent->height())
    );

    // Don't bother recalculating the byte item positions if the number of rows have not changed.
    if (rowCount == this->byteItemsByRowIndex.size()) {
        return;
    }

    std::map<std::size_t, std::vector<ByteItem*>> byteWidgetsByRowIndex;
    std::map<std::size_t, std::vector<ByteItem*>> byteWidgetsByColumnIndex;

    for (auto& [address, byteWidget] : this->byteItemsByAddress) {
        const auto rowIndex = static_cast<std::size_t>(
            std::ceil(static_cast<double>(byteWidget->byteIndex + 1) / static_cast<double>(rowCapacity)) - 1
        );
        const auto columnIndex = static_cast<std::size_t>(
            static_cast<double>(byteWidget->byteIndex)
                - (std::floor(byteWidget->byteIndex / rowCapacity) * static_cast<double>(rowCapacity))
        );

        byteWidget->setPos(
            static_cast<int>(columnIndex * byteWidgetWidth + margins.left() + ByteAddressContainer::WIDTH),
            static_cast<int>(rowIndex * byteWidgetHeight + static_cast<std::size_t>(margins.top()))
        );

        byteWidget->currentRowIndex = static_cast<std::size_t>(rowIndex);
        byteWidget->currentColumnIndex = static_cast<std::size_t>(columnIndex);

        byteWidgetsByRowIndex[byteWidget->currentRowIndex].emplace_back(byteWidget);
        byteWidgetsByColumnIndex[byteWidget->currentColumnIndex].emplace_back(byteWidget);

        byteWidget->update();
    }

    this->byteItemsByRowIndex = std::move(byteWidgetsByRowIndex);
    this->byteItemsByColumnIndex = std::move(byteWidgetsByColumnIndex);

    this->byteAddressContainer->adjustAddressLabels(this->byteItemsByRowIndex);
}

void ByteItemGraphicsScene::setEnabled(bool enabled) {
    if (this->enabled != enabled) {
        this->enabled = enabled;

        for (auto& [byteAddress, byteItem] : this->byteItemsByAddress) {
            byteItem->setEnabled(this->enabled);
        }

        this->byteAddressContainer->setEnabled(enabled);
        this->update();
    }
}

void ByteItemGraphicsScene::onTargetStateChanged(Targets::TargetState newState) {
    using Targets::TargetState;
    this->targetState = newState;
}

void ByteItemGraphicsScene::onByteWidgetEnter(ByteItem* widget) {
    if (this->hoveredByteWidget.has_value()) {
        if (this->hoveredByteWidget.value() == widget) {
            // This byte item is already marked as hovered
            return;

        } else {
            this->onByteWidgetLeave();
        }
    }

    this->hoveredByteWidget = widget;

    this->hoveredAddressLabel->setText(
        "Relative Address (Absolute Address): " + widget->relativeAddressHex + " (" + widget->addressHex + ")"
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
    const auto byteItem = this->hoveredByteWidget.value();
    this->hoveredByteWidget = std::nullopt;

    this->hoveredAddressLabel->setText("Relative Address (Absolute Address):");

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

bool ByteItemGraphicsScene::event(QEvent* event) {
    if (event->type() == QEvent::Type::GraphicsSceneLeave && this->hoveredByteWidget.has_value()) {
        this->onByteWidgetLeave();
    }

    return QGraphicsScene::event(event);
}

void ByteItemGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) {
    auto hoveredItems = this->items(mouseEvent->scenePos());
    if (!hoveredItems.empty()) {
        auto hoveredByteWidget = dynamic_cast<ByteItem*>(hoveredItems.at(0));

        if (hoveredByteWidget != nullptr) {
            this->onByteWidgetEnter(hoveredByteWidget);
        }

    } else if (this->hoveredByteWidget.has_value()) {
        this->onByteWidgetLeave();
    }
}
