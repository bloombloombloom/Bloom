#include "ByteAddressContainer.hpp"

namespace Widgets
{
    ByteAddressContainer::ByteAddressContainer(const HexViewerSharedState& hexViewerState)
        : hexViewerState(hexViewerState)
    {}

    void ByteAddressContainer::adjustAddressLabels(const std::vector<const ByteItem*>& firstByteItemByLine) {
        static constexpr int leftMargin = 10;

        const auto addressItemCount = this->addressItems.size();
        decltype(this->addressItems)::size_type rowIndex = 0;

        for (const auto& byteItem : firstByteItemByLine) {
            auto addressItem = addressItemCount > 0 && rowIndex <= addressItemCount - 1
                ? this->addressItems[rowIndex]
                : *(this->addressItems.insert(
                    this->addressItems.end(),
                    new ByteAddressItem(this->hexViewerState, this)
                )
            );

            addressItem->setPos(
                leftMargin,
                byteItem->position().y() + 4 // +4 to have the address item and byte item align vertically, from center
            );

            addressItem->address = byteItem->startAddress;
            addressItem->setVisible(true);
            ++rowIndex;
        }

        // Hide any address items we no longer need
        const auto usedAddressItemCount = rowIndex;
        if (addressItemCount > 0 && usedAddressItemCount < addressItemCount) {
            for (auto i = (addressItemCount - 1); i >= usedAddressItemCount; --i) {
                this->addressItems[i]->setVisible(false);
            }
        }

        this->update();
    }

    void ByteAddressContainer::invalidateChildItemCaches() {
        for (auto& addressItem : this->addressItems) {
            addressItem->update();
        }
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
