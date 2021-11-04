#include "ByteAddressItem.hpp"

#include <QPainter>
#include <QStyle>

using namespace Bloom::Widgets;

void ByteAddressItem::setAddressHex(const QString& addressHex) {
    this->addressHex = addressHex;
}

void ByteAddressItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    painter->setRenderHints(QPainter::RenderHint::Antialiasing | QPainter::RenderHint::SmoothPixmapTransform, true);

    static const auto widgetRect = this->boundingRect();
    auto fontColor = QColor(0x8F, 0x91, 0x92);
    static auto font = painter->font();
    font.setPixelSize(12);

    if (!this->isEnabled()) {
        fontColor.setAlpha(100);
    }

    painter->setFont(font);
    painter->setPen(fontColor);
    painter->drawText(widgetRect, Qt::AlignCenter, this->addressHex);
}
