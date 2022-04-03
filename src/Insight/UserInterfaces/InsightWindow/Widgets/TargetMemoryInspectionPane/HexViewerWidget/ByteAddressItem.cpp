#include "ByteAddressItem.hpp"

#include <QPainter>

namespace Bloom::Widgets
{
    void ByteAddressItem::setAddressHex(const QString& addressHex) {
        this->setCacheMode(
            QGraphicsItem::CacheMode::ItemCoordinateCache,
            QSize(ByteAddressItem::WIDTH, ByteAddressItem::HEIGHT)
        );
        this->addressHex = addressHex;
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
        painter->drawText(widgetRect, Qt::AlignLeft, this->addressHex);
    }
}
