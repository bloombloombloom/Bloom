#include "ByteItem.hpp"

#include <QPainter>
#include <QStyle>

#include "src/Logger/Logger.hpp"

using namespace Bloom::Widgets;

ByteItem::ByteItem(
    std::size_t byteIndex,
    std::uint32_t address,
    std::optional<ByteItem*>& hoveredByteItem,
    const HexViewerWidgetSettings& settings
):
QGraphicsItem(nullptr),
byteIndex(byteIndex),
address(address),
hoveredByteItem(hoveredByteItem),
settings(settings)
{
    this->setCacheMode(
        QGraphicsItem::CacheMode::ItemCoordinateCache,
        QSize(ByteItem::WIDTH, ByteItem::HEIGHT)
    );
    this->setAcceptHoverEvents(true);

    this->addressHex = "0x" + QString::number(this->address, 16).rightJustified(8, '0').toUpper();
    this->relativeAddressHex = "0x" + QString::number(this->byteIndex, 16).rightJustified(8, '0').toUpper();

    this->setSelected(false);
}

void ByteItem::setValue(unsigned char value) {
    this->valueChanged = this->valueInitialised && this->value != value;

    this->value = value;
    this->hexValue = QString::number(this->value, 16).rightJustified(2, '0').toUpper();
    this->asciiValue = (this->value >= 32 && this->value <= 126)
        ? std::optional(QString(QChar(this->value))) : std::nullopt;

    this->valueInitialised = true;
}

void ByteItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    painter->setRenderHints(QPainter::RenderHint::Antialiasing | QPainter::RenderHint::SmoothPixmapTransform, true);
    painter->setPen(Qt::PenStyle::NoPen);

    static const auto widgetRect = this->boundingRect();
    static const auto standardTextColor = QColor(0xAF, 0xB1, 0xB3);
    static const auto valueChangedTextColor = QColor(0x54, 0x7F, 0xBA);
    static auto font = QFont("'Ubuntu', sans-serif");

    static const auto stackMemoryBackgroundColor = QColor(0x5E, 0x50, 0x27, 255);
    static const auto hoveredBackgroundColor = QColor(0x8E, 0x8B, 0x83, 70);
    static const auto hoveredNeighbourBackgroundColor = QColor(0x8E, 0x8B, 0x83, 30);

    font.setPixelSize(11);
    painter->setFont(font);

    if (this->settings.highlightStackMemory && this->settings.stackPointerAddress.has_value()
        && this->address > this->settings.stackPointerAddress
    ) {
        // This byte is within the stack memory
        painter->setBrush(stackMemoryBackgroundColor);
        painter->drawRect(widgetRect);
    }

    const auto* hoveredByteItem = this->hoveredByteItem.value_or(nullptr);
    if (hoveredByteItem != nullptr) {
        if (hoveredByteItem == this) {
            painter->setBrush(hoveredBackgroundColor);

        } else if (this->settings.highlightHoveredRowAndCol
            && (
                hoveredByteItem->currentColumnIndex == this->currentColumnIndex
                || hoveredByteItem->currentRowIndex == this->currentRowIndex
            )
        ) {
            painter->setBrush(hoveredNeighbourBackgroundColor);
        }

        painter->drawRect(widgetRect);
    }

    auto textColor = this->valueChanged ? valueChangedTextColor : standardTextColor;

    if (this->valueInitialised) {
        if (!this->isEnabled()) {
            textColor.setAlpha(100);
        }

        painter->setPen(textColor);
        painter->drawText(widgetRect, Qt::AlignCenter, this->hexValue);

    } else {
        textColor.setAlpha(100);
        painter->setPen(textColor);

        static const auto placeholderString = QString("??");
        painter->drawText(widgetRect, Qt::AlignCenter, placeholderString);
    }
}
