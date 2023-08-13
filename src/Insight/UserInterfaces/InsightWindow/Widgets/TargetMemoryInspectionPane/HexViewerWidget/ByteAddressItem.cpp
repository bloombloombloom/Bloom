#include "ByteAddressItem.hpp"

namespace Widgets
{
    ByteAddressItem::ByteAddressItem(const HexViewerSharedState& hexViewerState, QGraphicsItem* parent)
        : hexViewerState(hexViewerState)
        , QGraphicsItem(parent)
    {}

    void ByteAddressItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
        static auto fontColor = QColor(0x8F, 0x91, 0x92);
        static auto font = QFont("'Ubuntu', sans-serif");
        font.setPixelSize(12);

        painter->setRenderHints(
            QPainter::RenderHint::Antialiasing | QPainter::RenderHint::SmoothPixmapTransform,
            true
        );

        if (!this->isEnabled()) {
            painter->setOpacity(0.5);
        }

        painter->setFont(font);
        painter->setPen(fontColor);
        painter->drawText(
            this->boundingRect(),
            Qt::AlignLeft,
            this->hexViewerState.settings.addressLabelType == AddressType::RELATIVE
                ? "0x" + QString::number(
                    this->address - this->hexViewerState.memoryDescriptor.addressRange.startAddress,
                    16
                ).rightJustified(8, '0').toUpper()
                : "0x" + QString::number(this->address, 16).rightJustified(8, '0').toUpper()
        );
    }
}
