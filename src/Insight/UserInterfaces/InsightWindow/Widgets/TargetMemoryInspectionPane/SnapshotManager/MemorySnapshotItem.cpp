#include "MemorySnapshotItem.hpp"

#include "src/Services/DateTimeService.hpp"

namespace Bloom::Widgets
{
    MemorySnapshotItem::MemorySnapshotItem(const MemorySnapshot& memorySnapshot)
        : memorySnapshot(memorySnapshot)
    {
        this->size = QSize(0, MemorySnapshotItem::HEIGHT);

        this->nameText = memorySnapshot.name;
        this->programCounterText = "0x" + QString::number(this->memorySnapshot.programCounter, 16).toUpper();
        this->createdDateText = memorySnapshot.createdDate.toString(
            memorySnapshot.createdDate.date() == Services::DateTimeService::currentDate()
                ? "hh:mm"
                : "dd/MM/yyyy hh:mm"
        );
    }

    void MemorySnapshotItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
        static constexpr auto margins = QMargins(5, 5, 5, 0);

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

        const auto fontMetrics = painter->fontMetrics();

        const auto programCounterTextSize = fontMetrics.size(Qt::TextSingleLine, this->programCounterText);
        const auto createdDateTextSize = fontMetrics.size(Qt::TextSingleLine, this->createdDateText);

        constexpr auto nameTextRightMargin = 10;
        const auto availableNameTextWidth = this->size.width() - margins.left() - margins.right()
            - programCounterTextSize.width() - nameTextRightMargin;

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

        if (!this->selected) {
            painter->setPen(secondaryFontColor);
        }

        const auto programCounterTextRect = QRect(
            this->size.width() - margins.right() - programCounterTextSize.width(),
            margins.top(),
            programCounterTextSize.width(),
            programCounterTextSize.height()
        );

        painter->drawText(programCounterTextRect, Qt::AlignLeft, this->programCounterText);

        const auto createdDateTextRect = QRect(
            margins.left(),
            nameTextRect.bottom() + 5,
            createdDateTextSize.width(),
            createdDateTextSize.height()
        );

        painter->drawText(createdDateTextRect, Qt::AlignLeft, this->createdDateText);

        static constexpr auto borderColor = QColor(0x2E, 0x2E, 0x2E);

        painter->setPen(borderColor);
        painter->drawLine(0, this->size.height() - 1, this->size.width(), this->size.height() - 1);
    }
}
