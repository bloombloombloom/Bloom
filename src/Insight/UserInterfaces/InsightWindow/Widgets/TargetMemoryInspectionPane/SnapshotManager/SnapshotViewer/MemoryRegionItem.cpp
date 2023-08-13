#include "MemoryRegionItem.hpp"

#include "src/Services/DateTimeService.hpp"

namespace Widgets
{
    MemoryRegionItem::MemoryRegionItem(const MemoryRegion& memoryRegion)
        : memoryRegion(memoryRegion)
    {
        this->size = QSize(0, MemoryRegionItem::HEIGHT);

        this->nameText = memoryRegion.name;
        this->addressRangeText = "0x" + QString::number(this->memoryRegion.addressRange.startAddress, 16).toUpper()
            + QString(" -> ") + "0x" + QString::number(this->memoryRegion.addressRange.endAddress, 16).toUpper();
        this->regionTypeText = this->memoryRegion.type == MemoryRegionType::EXCLUDED ? "Excluded" : "Focused";
        this->createdDateText = memoryRegion.createdDate.toString(
            memoryRegion.createdDate.date() == Services::DateTimeService::currentDate()
                ? "hh:mm"
                : "dd/MM/yyyy hh:mm"
        );

        this->setToolTip(this->nameText);
    }

    void MemoryRegionItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
        static constexpr auto margins = QMargins(10, 5, 10, 0);

        painter->setOpacity(0.7);
        static auto font = QFont("'Ubuntu', sans-serif");
        font.setPixelSize(14);
        static auto secondaryFont = QFont("'Ubuntu', sans-serif");
        secondaryFont.setPixelSize(13);

        static constexpr auto fontColor = QColor(0xAF, 0xB1, 0xB3);
        static constexpr auto secondaryFontColor = QColor(0x8A, 0x8A, 0x8D);

        painter->setFont(font);
        painter->setPen(fontColor);

        const auto fontMetrics = painter->fontMetrics();

        const auto addressRangeTextSize = fontMetrics.size(Qt::TextSingleLine, this->addressRangeText);
        const auto regionTypeTextSize = fontMetrics.size(Qt::TextSingleLine, this->regionTypeText);
        const auto createdDateTextSize = fontMetrics.size(Qt::TextSingleLine, this->createdDateText);

        constexpr auto nameTextRightMargin = 10;
        const auto availableNameTextWidth = this->size.width() - margins.left() - margins.right()
            - regionTypeTextSize.width() - nameTextRightMargin;

        const auto nameText = fontMetrics.elidedText(
            this->nameText,
            Qt::TextElideMode::ElideRight,
            availableNameTextWidth
        );

        const auto nameTextSize = fontMetrics.size(Qt::TextSingleLine, nameText);
        const auto nameTextRect = QRect(
            margins.left(),
            margins.top(),
            nameTextSize.width(),
            nameTextSize.height()
        );

        painter->drawText(nameTextRect, Qt::AlignLeft, nameText);

        painter->setFont(secondaryFont);
        painter->setPen(secondaryFontColor);

        const auto addressRangeTextRect = QRect(
            margins.left(),
            nameTextRect.bottom() + 5,
            addressRangeTextSize.width(),
            addressRangeTextSize.height()
        );

        painter->drawText(addressRangeTextRect, Qt::AlignLeft, this->addressRangeText);

        const auto regionTypeTextRect = QRect(
            this->size.width() - margins.right() - regionTypeTextSize.width(),
            margins.top(),
            regionTypeTextSize.width(),
            regionTypeTextSize.height()
        );

        painter->drawText(regionTypeTextRect, Qt::AlignRight, this->regionTypeText);

        const auto createdDateTextRect = QRect(
            this->size.width() - margins.right() - createdDateTextSize.width(),
            nameTextRect.bottom() + 5,
            createdDateTextSize.width(),
            createdDateTextSize.height()
        );

        painter->drawText(createdDateTextRect, Qt::AlignRight, this->createdDateText);

        static constexpr auto borderColor = QColor(0x41, 0x42, 0x3F);
        painter->setPen(borderColor);
        painter->drawLine(0, this->size.height() - 1, this->size.width(), this->size.height() - 1);
    }
}
