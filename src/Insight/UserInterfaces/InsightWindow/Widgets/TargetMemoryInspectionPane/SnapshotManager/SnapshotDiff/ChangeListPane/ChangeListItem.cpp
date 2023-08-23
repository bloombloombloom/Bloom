#include "ChangeListItem.hpp"

#include <QString>
#include <QLocale>

namespace Widgets
{
    ChangeListItem::ChangeListItem(const Targets::TargetMemoryAddressRange& addressRange)
        : addressRange(addressRange)
    {
        this->size = QSize(0, ChangeListItem::HEIGHT);
    }

    void ChangeListItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
        static constexpr auto margins = QMargins(7, 5, 7, 0);

        static auto font = QFont("'Ubuntu', sans-serif");
        font.setPixelSize(14);
        static auto secondaryFont = QFont("'Ubuntu', sans-serif");
        secondaryFont.setPixelSize(13);

        static constexpr auto fontColor = QColor(0xAF, 0xB1, 0xB3);
        static constexpr auto secondaryFontColor = QColor(0x8A, 0x8A, 0x8D);

        if (this->selected) {
            static constexpr auto selectedBackgroundColor = QColor(0x3C, 0x59, 0x5C);

            painter->setBrush(selectedBackgroundColor);
            painter->setPen(Qt::PenStyle::NoPen);
            painter->drawRect(QRect(QPoint(0, 0), this->size));
        }

        painter->setFont(font);
        painter->setPen(fontColor);

        auto fontMetrics = painter->fontMetrics();

        const auto byteCount = this->addressRange.endAddress - this->addressRange.startAddress + 1;
        const auto byteCountText = QLocale(QLocale::English).toString(byteCount)
            + (byteCount == 1 ? " byte" : " bytes");
        const auto byteCountTextSize = fontMetrics.size(Qt::TextSingleLine, byteCountText);

        const auto byteCountTextRect = QRect(
            margins.left(),
            margins.top(),
            byteCountTextSize.width(),
            byteCountTextSize.height()
        );

        painter->drawText(byteCountTextRect, Qt::AlignLeft, byteCountText);

        painter->setFont(secondaryFont);
        fontMetrics = painter->fontMetrics();

        if (!this->selected) {
            painter->setPen(secondaryFontColor);
        }

        auto addressRangeText = "0x" + QString::number(
            this->addressRange.startAddress,
            16
        ).rightJustified(8, '0').toUpper();

        if (byteCount > 1) {
            addressRangeText += " -> 0x" + QString::number(
                this->addressRange.endAddress,
                16
            ).rightJustified(8, '0').toUpper();
        }

        const auto availableAddressRangeTextWidth = this->size.width() - margins.left() - margins.right();

        addressRangeText = fontMetrics.elidedText(
            addressRangeText,
            Qt::TextElideMode::ElideRight,
            availableAddressRangeTextWidth
        );

        const auto addressRangeTextSize = fontMetrics.size(Qt::TextSingleLine, addressRangeText);
        const auto addressRangeTextRect = QRect(
            margins.left(),
            byteCountTextRect.bottom() + 5,
            addressRangeTextSize.width(),
            addressRangeTextSize.height()
        );

        painter->drawText(addressRangeTextRect, Qt::AlignLeft, addressRangeText);

        static constexpr auto borderColor = QColor(0x41, 0x42, 0x3F);

        painter->setPen(borderColor);
        painter->drawLine(0, this->size.height() - 1, this->size.width(), this->size.height() - 1);
    }
}
