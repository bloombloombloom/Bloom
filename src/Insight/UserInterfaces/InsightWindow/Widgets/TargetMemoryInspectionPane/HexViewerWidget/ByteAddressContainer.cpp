#include "ByteAddressContainer.hpp"

namespace Bloom::Widgets
{
    ByteAddressContainer::ByteAddressContainer(const HexViewerWidgetSettings& settings)
        : settings(settings)
    {}

    void ByteAddressContainer::adjustAddressLabels(
        const std::map<std::size_t, std::vector<ByteItem*>>& byteItemsByRowIndex
    ) {
        static constexpr int leftMargin = 10;
        const auto newRowCount = byteItemsByRowIndex.size();
        const auto layoutItemMaxIndex = static_cast<int>(this->addressItemsByRowIndex.size() - 1);

        for (const auto& mappingPair : byteItemsByRowIndex) {
            const auto rowIndex = static_cast<std::size_t>(mappingPair.first);
            const auto& byteItems = mappingPair.second;

            ByteAddressItem* addressLabel = nullptr;
            if (static_cast<int>(rowIndex) > layoutItemMaxIndex) {
                addressLabel = new ByteAddressItem(rowIndex, byteItemsByRowIndex, this->settings.addressLabelType, this);
                this->addressItemsByRowIndex.emplace(rowIndex, addressLabel);

            } else {
                addressLabel = this->addressItemsByRowIndex.at(rowIndex);
                addressLabel->update();
                addressLabel->setVisible(true);
            }

            const auto& firstByteItem = byteItems.front();
            addressLabel->setPos(
                leftMargin,
                firstByteItem->pos().y() + 3 // +3 to have the address item and byte item align vertically, from center
            );
        }

        // Hide any address items we no longer need
        const auto addressItemCount = this->addressItemsByRowIndex.size();

        if (newRowCount > 0 && newRowCount < addressItemCount) {
            for (auto i = (addressItemCount - 1); i >= newRowCount; i--) {
                this->addressItemsByRowIndex.at(i)->setVisible(false);
            }
        }

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
}
