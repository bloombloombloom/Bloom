#include "ByteWidgetContainer.hpp"

#include <QVBoxLayout>
#include <QTableWidget>
#include <QScrollBar>
#include <QPainter>
#include <cmath>

using namespace Bloom::Widgets;
using namespace Bloom::Exceptions;

using Bloom::Targets::TargetMemoryDescriptor;

ByteWidgetContainer::ByteWidgetContainer(
    const TargetMemoryDescriptor& targetMemoryDescriptor,
    InsightWorker& insightWorker,
    QWidget* parent
): QWidget(parent), targetMemoryDescriptor(targetMemoryDescriptor), insightWorker(insightWorker), parent(parent) {
    this->setObjectName("byte-widget-container");
    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Ignored);

    this->setContentsMargins(
        10,
        10,
        10,
        10
    );

    /*
     * Construct ByteWidget objects
     *
     * No need to position them here - the subsequent call to resizeEvent() will do that.
     */
    const auto memorySize = this->targetMemoryDescriptor.size();
    const auto startAddress = this->targetMemoryDescriptor.addressRange.startAddress;
    for (std::size_t i = 0; i < memorySize; i++) {
        const auto address = static_cast<std::uint32_t>(startAddress + i);

        auto byteWidget = new ByteWidget(i, address, this->hoveredByteWidget, this);
        this->byteWidgetsByAddress.insert(std::pair(
            address,
            byteWidget
        ));

        this->connect(byteWidget, &ByteWidget::enter, this, &ByteWidgetContainer::onByteWidgetEnter);
        this->connect(byteWidget, &ByteWidget::leave, this, &ByteWidgetContainer::onByteWidgetLeave);
    }

    this->connect(
        &insightWorker,
        &InsightWorker::targetStateUpdated,
        this,
        &ByteWidgetContainer::onTargetStateChanged
    );

    this->show();
}

void ByteWidgetContainer::updateValues(const Targets::TargetMemoryBuffer& buffer) {
    for (auto& [address, byteWidget] : this->byteWidgetsByAddress) {
        byteWidget->setValue(buffer.at(byteWidget->byteIndex));
        byteWidget->update();
    }
}

void ByteWidgetContainer::resizeEvent(QResizeEvent* event) {
    this->adjustByteWidgets();
}

void ByteWidgetContainer::adjustByteWidgets() {
    std::map<std::size_t, std::vector<ByteWidget*>> byteWidgetsByRowIndex;
    std::map<std::size_t, std::vector<ByteWidget*>> byteWidgetsByColumnIndex;

    const auto margins = this->contentsMargins();
    const auto width = this->width();

    constexpr auto byteWidgetWidth = ByteWidget::WIDTH + ByteWidget::RIGHT_MARGIN;
    constexpr auto byteWidgetHeight = ByteWidget::HEIGHT + ByteWidget::BOTTOM_MARGIN;
    const auto rowCapacity = static_cast<std::size_t>(
        std::floor((width - margins.left() - margins.right()) / byteWidgetWidth)
    );
    const auto rowCount = static_cast<int>(
        std::ceil(static_cast<double>(this->byteWidgetsByAddress.size()) / static_cast<double>(rowCapacity))
    );

    for (auto& [address, byteWidget] : this->byteWidgetsByAddress) {
        const auto rowIndex = static_cast<std::size_t>(
            std::ceil(static_cast<double>(byteWidget->byteIndex + 1) / static_cast<double>(rowCapacity)) - 1
        );
        const auto columnIndex = static_cast<std::size_t>(
            static_cast<double>(byteWidget->byteIndex)
            - (std::floor(byteWidget->byteIndex / rowCapacity) * static_cast<double>(rowCapacity))
        );

        byteWidget->setGeometry(QRect(
            static_cast<int>(columnIndex * byteWidgetWidth + static_cast<std::size_t>(margins.left())),
            static_cast<int>(rowIndex * byteWidgetHeight + static_cast<std::size_t>(margins.top())),
            ByteWidget::WIDTH,
            ByteWidget::HEIGHT
        ));

        byteWidget->currentRowIndex = static_cast<std::size_t>(rowIndex);
        byteWidget->currentColumnIndex = static_cast<std::size_t>(columnIndex);

        byteWidgetsByRowIndex[byteWidget->currentRowIndex].emplace_back(byteWidget);
        byteWidgetsByColumnIndex[byteWidget->currentColumnIndex].emplace_back(byteWidget);

        byteWidget->update();
    }

    const auto minHeight = (rowCount * byteWidgetHeight) + margins.top() + margins.bottom();
    this->setMinimumHeight(minHeight);
    this->parent->setMinimumHeight(minHeight);

    this->byteWidgetsByRowIndex.swap(byteWidgetsByRowIndex);
    this->byteWidgetsByColumnIndex.swap(byteWidgetsByColumnIndex);

    emit this->byteWidgetsAdjusted();
}

void ByteWidgetContainer::onTargetStateChanged(Targets::TargetState newState) {
    using Targets::TargetState;
    this->targetState = newState;
}

void ByteWidgetContainer::onByteWidgetEnter(ByteWidget* widget) {
    this->hoveredByteWidget = widget;

    if (!this->byteWidgetsByRowIndex.empty()) {
        for (auto& byteWidget : this->byteWidgetsByColumnIndex.at(widget->currentColumnIndex)) {
            byteWidget->update();
        }

        for (auto& byteWidget : this->byteWidgetsByRowIndex.at(widget->currentRowIndex)) {
            byteWidget->update();
        }
    }
}

void ByteWidgetContainer::onByteWidgetLeave(ByteWidget* widget) {
    this->hoveredByteWidget = std::nullopt;

    if (!this->byteWidgetsByRowIndex.empty()) {
        for (auto& byteWidget : this->byteWidgetsByColumnIndex.at(widget->currentColumnIndex)) {
            byteWidget->update();
        }

        for (auto& byteWidget : this->byteWidgetsByRowIndex.at(widget->currentRowIndex)) {
            byteWidget->update();
        }
    }
}
