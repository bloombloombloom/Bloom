#include "ByteAddressItem.hpp"

#include <QPainter>

namespace Bloom::Widgets
{
    ByteAddressItem::ByteAddressItem(
        std::size_t rowIndex,
        const std::map<std::size_t, std::vector<ByteItem*>>& byteItemsByRowIndex,
        QGraphicsItem* parent
    )
        : rowIndex(rowIndex)
        , byteItemsByRowIndex(byteItemsByRowIndex)
        , QGraphicsItem(parent)
    {
        this->setCacheMode(
            QGraphicsItem::CacheMode::ItemCoordinateCache,
            QSize(ByteAddressItem::WIDTH, ByteAddressItem::HEIGHT)
        );
    }

    void ByteAddressItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
        painter->setRenderHints(
            QPainter::RenderHint::Antialiasing | QPainter::RenderHint::SmoothPixmapTransform,
            true
        );

        static const auto widgetRect = this->boundingRect();
        static auto fontColor = QColor(0x8F, 0x91, 0x92);
        static auto font = QFont("'Ubuntu', sans-serif");
        font.setPixelSize(12);
        fontColor.setAlpha(!this->isEnabled() ? 100 : 255);

        painter->setFont(font);
        painter->setPen(fontColor);
        painter->drawText(
            widgetRect,
            Qt::AlignLeft,
            this->byteItemsByRowIndex.at(this->rowIndex)[0]->relativeAddressHex
        );
    }
}
