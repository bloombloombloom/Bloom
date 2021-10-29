#include "ByteAddressContainer.hpp"

#include <QVBoxLayout>
#include <QTableWidget>
#include <QScrollBar>
#include <QPainter>
#include <cmath>

#include "src/Logger/Logger.hpp"

using namespace Bloom::Widgets;

ByteAddressContainer::ByteAddressContainer() {
}

void ByteAddressContainer::adjustAddressLabels(
    const std::map<std::size_t, std::vector<ByteItem*>>& byteItemsByRowIndex
) {
    static const auto margins = QMargins(0, 10, 0, 0);

    const auto rowCount = byteItemsByRowIndex.size();
    const auto addressLabelCount = this->addressItemsByRowIndex.size();
    const auto layoutItemMaxIndex = static_cast<int>(addressLabelCount - 1);
    for (const auto& mappingPair : byteItemsByRowIndex) {
        const auto rowIndex = static_cast<std::size_t>(mappingPair.first);
        const auto& byteWidgets = mappingPair.second;

        if (byteWidgets.empty()) {
            continue;
        }

        ByteAddressItem* addressLabel;
        if (static_cast<int>(rowIndex) > layoutItemMaxIndex) {
            addressLabel = new ByteAddressItem(this);
            this->addressItemsByRowIndex.insert(std::pair(rowIndex, addressLabel));

        } else {
            addressLabel = this->addressItemsByRowIndex.at(rowIndex);
        }

        addressLabel->setAddressHex(byteWidgets.front()->relativeAddressHex);
        addressLabel->setPos(
            0,
            margins.top() + static_cast<double>(rowIndex * (ByteAddressItem::HEIGHT + ByteItem::BOTTOM_MARGIN))
        );
    }

    if (rowCount > 0 && rowCount > byteItemsByRowIndex.size()) {
        const auto rowCount = static_cast<int>(byteItemsByRowIndex.size());
        for (std::size_t i = rowCount - 1; i >= rowCount; i--) {
            delete this->addressItemsByRowIndex.at(i);
            this->addressItemsByRowIndex.erase(i);
        }
    }

    Logger::error("Done with rows: " + std::to_string(this->addressItemsByRowIndex.size()));

    this->update();
}

void ByteAddressContainer::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    static const auto backgroundColor = QColor(0x35, 0x36, 0x33);
    static const auto borderColor = QColor(0x41, 0x42, 0x3F);

    painter->setPen(Qt::PenStyle::NoPen);
    painter->setBrush(backgroundColor);
    painter->drawRect(
        0,
        0,
        ByteAddressContainer::WIDTH,
        static_cast<int>(this->boundingRect().height())
    );

    painter->setPen(borderColor);
    painter->drawLine(
        ByteAddressContainer::WIDTH - 1,
        0,
        ByteAddressContainer::WIDTH - 1,
        static_cast<int>(this->boundingRect().height())
    );
}
