#include "AnnotationItem.hpp"

#include <QPainter>

#include "ByteItem.hpp"

using namespace Bloom::Widgets;

AnnotationItem::AnnotationItem(std::uint32_t startAddress, std::size_t size, QString labelText, AnnotationItemPosition position)
: QGraphicsItem(nullptr),
startAddress(startAddress),
size(size),
endAddress(static_cast<std::uint32_t>(startAddress + size - 1)),
labelText(std::move(labelText)),
position(position),
width(static_cast<int>((ByteItem::WIDTH + ByteItem::RIGHT_MARGIN) * size - ByteItem::RIGHT_MARGIN)),
height(position == AnnotationItemPosition::TOP ? AnnotationItem::TOP_HEIGHT : AnnotationItem::BOTTOM_HEIGHT) {
    this->setAcceptHoverEvents(true);
    this->setToolTip(this->labelText);
}

AnnotationItem::AnnotationItem(
    const Targets::TargetMemoryAddressRange& addressRange,
    const QString& labelText,
    AnnotationItemPosition position
): AnnotationItem(
    addressRange.startAddress,
    addressRange.endAddress - addressRange.startAddress + 1,
    labelText,
    position
) {}

AnnotationItem::AnnotationItem(const FocusedMemoryRegion& focusedMemoryRegion, AnnotationItemPosition position)
: AnnotationItem(
    focusedMemoryRegion.getAbsoluteAddressRange(),
    focusedMemoryRegion.name,
    position
) {}

void AnnotationItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    auto lineColor = this->getLineColor();
    auto labelFontColor = this->getLabelFontColor();

    const auto isEnabled = this->isEnabled();

    lineColor.setAlpha(isEnabled ? 255 : 100);
    labelFontColor.setAlpha(isEnabled ? 255 : 100);

    const auto fontMetrics = painter->fontMetrics();
    auto labelSize = fontMetrics.size(Qt::TextSingleLine, this->labelText);
    if (labelSize.width() > this->width) {
        labelSize.setWidth(this->width);
        this->labelText = fontMetrics.elidedText(this->labelText, Qt::TextElideMode::ElideRight, this->width);
    }

    constexpr auto verticalLineLength = 5;
    const auto verticalLineYStart = this->position == AnnotationItemPosition::BOTTOM ? 0 : AnnotationItem::TOP_HEIGHT;
    const auto verticalLineYEnd = this->position == AnnotationItemPosition::BOTTOM ?
        verticalLineLength : AnnotationItem::TOP_HEIGHT - verticalLineLength;

    const auto labelRect = QRect(
        (this->width - labelSize.width()) / 2,
        verticalLineYEnd - (this->position == AnnotationItemPosition::BOTTOM ? -6: labelSize.height() + 6),
        labelSize.width(),
        labelSize.height()
    );

    painter->setPen(lineColor);
    painter->drawLine(QLine(
        ByteItem::WIDTH / 2,
        verticalLineYStart,
        ByteItem::WIDTH / 2,
        verticalLineYEnd
    ));

    painter->drawLine(QLine(
        this->width - (ByteItem::WIDTH / 2),
        verticalLineYStart,
        this->width - (ByteItem::WIDTH / 2),
        verticalLineYEnd
    ));

    painter->drawLine(QLine(
        ByteItem::WIDTH / 2,
        verticalLineYEnd,
        (ByteItem::WIDTH / 2) + (this->width - ByteItem::WIDTH),
        verticalLineYEnd
    ));

    painter->drawLine(QLine(
        this->width / 2,
        verticalLineYEnd,
        this->width / 2,
        (this->position == AnnotationItemPosition::BOTTOM ? verticalLineYEnd + 4 : verticalLineYEnd - 4)
    ));

    painter->setPen(labelFontColor);
    painter->drawText(labelRect, Qt::AlignCenter, this->labelText);
}
