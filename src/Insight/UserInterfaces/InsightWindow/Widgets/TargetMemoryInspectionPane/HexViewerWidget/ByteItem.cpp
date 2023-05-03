#include "ByteItem.hpp"

#include <QFont>
#include <QColor>

namespace Bloom::Widgets
{
    ByteItem::ByteItem(Targets::TargetMemoryAddress address)
        : HexViewerItem(address)
    {
        if (!ByteItem::pixmapCachesGenerated) {
            ByteItem::generatePixmapCaches();
        }
    }

    void ByteItem::paint(
        QPainter* painter,
        const HexViewerSharedState* hexViewerState,
        const QGraphicsItem* graphicsItem
    ) const {
        const auto boundingRect = QRect(0, 0, ByteItem::WIDTH, ByteItem::HEIGHT);

        if (!graphicsItem->isEnabled() || (this->excluded && !this->selected)) {
            painter->setOpacity(0.6);
        }

        if (this->excluded || !hexViewerState->data.has_value()) {
            if (this->selected) {
                painter->drawPixmap(boundingRect, ByteItem::selectedMissingDataPixmap.value());
                return;
            }

            painter->drawPixmap(boundingRect, ByteItem::missingDataPixmap.value());
            return;
        }

        const auto byteIndex = this->startAddress - hexViewerState->memoryDescriptor.addressRange.startAddress;
        const auto value = (*hexViewerState->data)[byteIndex];

        const auto hoveredPrimary = hexViewerState->hoveredByteItem == this;

        if (hexViewerState->settings.displayAsciiValues) {
            if (this->selected) {
                painter->drawPixmap(
                    boundingRect,
                    ByteItem::selectedAsciiPixmapsByValue[value]
                );
                return;
            }

            if (this->changed) {
                painter->drawPixmap(
                    boundingRect,
                    ByteItem::changedMemoryAsciiPixmapsByValue[value]
                );
                return;
            }

            if (this->stackMemory && hexViewerState->settings.groupStackMemory) {
                painter->drawPixmap(
                    boundingRect,
                    ByteItem::stackMemoryAsciiPixmapsByValue[value]
                );
                return;
            }

            if (this->grouped && hexViewerState->settings.highlightFocusedMemory) {
                painter->drawPixmap(
                    boundingRect,
                    ByteItem::groupedAsciiPixmapsByValue[value]
                );
                return;
            }

            if (hoveredPrimary) {
                painter->drawPixmap(
                    boundingRect,
                    ByteItem::hoveredPrimaryAsciiPixmapsByValue[value]
                );
                return;
            }

            painter->drawPixmap(boundingRect, this->standardAsciiPixmapsByValue[value]);
            return;
        }

        if (this->selected) {
            painter->drawPixmap(
                boundingRect,
                ByteItem::selectedPixmapsByValue[value]
            );
            return;
        }

        if (this->changed) {
            painter->drawPixmap(
                boundingRect,
                ByteItem::changedMemoryPixmapsByValue[value]
            );
            return;
        }

        if (this->stackMemory && hexViewerState->settings.groupStackMemory) {
            painter->drawPixmap(
                boundingRect,
                ByteItem::stackMemoryPixmapsByValue[value]
            );
            return;
        }

        if (this->grouped && hexViewerState->settings.highlightFocusedMemory) {
            painter->drawPixmap(
                boundingRect,
                ByteItem::groupedPixmapsByValue[value]
            );
            return;
        }

        if (hoveredPrimary) {
            painter->drawPixmap(
                boundingRect,
                ByteItem::hoveredPrimaryPixmapsByValue[value]
            );
            return;
        }

        painter->drawPixmap(
            boundingRect,
            this->standardPixmapsByValue[value]
        );
    }

    void ByteItem::generatePixmapCaches() {
        const auto lock = std::unique_lock(ByteItem::pixmapCacheMutex);

        if (ByteItem::pixmapCachesGenerated) {
            return;
        }

        static constexpr auto standardBackgroundColor = QColor(0x32, 0x33, 0x30, 255);
        static constexpr auto highlightedBackgroundColor = QColor(0x3C, 0x59, 0x5C, 255);
        static constexpr auto selectedBackgroundColor = QColor(0x3C, 0x59, 0x5C, 255);
        static constexpr auto groupedBackgroundColor = QColor(0x44, 0x44, 0x41, 255);
        static constexpr auto stackMemoryBackgroundColor = QColor(0x44, 0x44, 0x41, 200);
        static constexpr auto stackMemoryBarColor = QColor(0x67, 0x57, 0x20, 255);
        static constexpr auto changedMemoryBackgroundColor = QColor(0x5C, 0x49, 0x5D, 200);
        static constexpr auto changedMemoryFadedBackgroundColor = QColor(0x5C, 0x49, 0x5D, 125);

        static const auto hoveredStackMemoryBackgroundColor = QColor(
            stackMemoryBackgroundColor.red(),
            stackMemoryBackgroundColor.green(),
            stackMemoryBackgroundColor.blue(),
            255
        );

        static constexpr auto hoveredBackgroundColor = QColor(0x8E, 0x8B, 0x83, 70);

        static constexpr auto standardFontColor = QColor(0xAF, 0xB1, 0xB3);
        static constexpr auto fadedFontColor = QColor(0xAF, 0xB1, 0xB3, 100);
        static constexpr auto asciiFontColor = QColor(0xA7, 0x77, 0x26);
        static constexpr auto changedMemoryAsciiFontColor = QColor(0xB7, 0x7F, 0x21);

        const auto byteItemRect = QRect(0, 0, ByteItem::WIDTH, ByteItem::HEIGHT);
        const auto byteItemSize = byteItemRect.size();

        auto standardTemplatePixmap = QPixmap(byteItemSize);
        standardTemplatePixmap.fill(standardBackgroundColor);

        auto highlightedTemplatePixmap = QPixmap(byteItemSize);
        highlightedTemplatePixmap.fill(highlightedBackgroundColor);

        auto selectedTemplatePixmap = QPixmap(byteItemSize);
        selectedTemplatePixmap.fill(selectedBackgroundColor);

        auto groupedTemplatePixmap = QPixmap(byteItemSize);
        groupedTemplatePixmap.fill(groupedBackgroundColor);

        auto stackMemoryTemplatePixmap = QPixmap(byteItemSize);
        stackMemoryTemplatePixmap.fill(stackMemoryBackgroundColor);

        {
            auto painter = QPainter(&stackMemoryTemplatePixmap);
            painter.setBrush(stackMemoryBarColor);
            painter.setPen(Qt::PenStyle::NoPen);
            painter.drawRect(0, byteItemSize.height() - 3, byteItemSize.width(), 3);
        }

        auto changedMemoryTemplatePixmap = QPixmap(byteItemSize);
        changedMemoryTemplatePixmap.fill(changedMemoryBackgroundColor);

        auto changedMemoryFadedTemplatePixmap = QPixmap(byteItemSize);
        changedMemoryFadedTemplatePixmap.fill(changedMemoryFadedBackgroundColor);

        auto hoveredStackMemoryTemplatePixmap = QPixmap(byteItemSize);
        hoveredStackMemoryTemplatePixmap.fill(hoveredStackMemoryBackgroundColor);

        auto hoveredPrimaryTemplatePixmap = QPixmap(byteItemSize);
        hoveredPrimaryTemplatePixmap.fill(hoveredBackgroundColor);

        static auto font = QFont("'Ubuntu', sans-serif", 8);

        for (std::uint16_t value = 0x00; value <= 0xFF; ++value) {
            const auto hexValue = QString::number(value, 16).rightJustified(2, '0').toUpper();
            const auto asciiValue = value >= 32 && value <= 126
                ? std::optional("'" + QString(QChar(value)) + "'")
                : std::nullopt;

            {
                auto standardPixmap = standardTemplatePixmap;
                auto painter = QPainter(&standardPixmap);
                painter.setFont(font);
                painter.setPen(standardFontColor);
                painter.drawText(byteItemRect, Qt::AlignCenter, hexValue);

                ByteItem::standardPixmapsByValue.emplace_back(std::move(standardPixmap));
            }

            {
                auto selectedPixmap = selectedTemplatePixmap;
                auto painter = QPainter(&selectedPixmap);
                painter.setFont(font);
                painter.setPen(standardFontColor);
                painter.drawText(byteItemRect, Qt::AlignCenter, hexValue);

                ByteItem::selectedPixmapsByValue.emplace_back(std::move(selectedPixmap));
            }

            {
                auto groupedPixmap = groupedTemplatePixmap;
                auto painter = QPainter(&groupedPixmap);
                painter.setFont(font);
                painter.setPen(standardFontColor);
                painter.drawText(byteItemRect, Qt::AlignCenter, hexValue);

                ByteItem::groupedPixmapsByValue.emplace_back(std::move(groupedPixmap));
            }

            {
                auto stackMemoryPixmap = stackMemoryTemplatePixmap;
                auto painter = QPainter(&stackMemoryPixmap);
                painter.setFont(font);
                painter.setPen(standardFontColor);
                painter.drawText(byteItemRect, Qt::AlignCenter, hexValue);

                ByteItem::stackMemoryPixmapsByValue.emplace_back(std::move(stackMemoryPixmap));
            }

            {
                auto changedMemoryPixmap = changedMemoryTemplatePixmap;
                auto painter = QPainter(&changedMemoryPixmap);
                painter.setFont(font);
                painter.setPen(standardFontColor);
                painter.drawText(byteItemRect, Qt::AlignCenter, hexValue);

                ByteItem::changedMemoryPixmapsByValue.emplace_back(std::move(changedMemoryPixmap));
            }

            {
                auto hoveredPrimaryPixmap = hoveredPrimaryTemplatePixmap;
                auto painter = QPainter(&hoveredPrimaryPixmap);
                painter.setFont(font);
                painter.setPen(standardFontColor);
                painter.drawText(byteItemRect, Qt::AlignCenter, hexValue);

                ByteItem::hoveredPrimaryPixmapsByValue.emplace_back(std::move(hoveredPrimaryPixmap));
            }

            {
                auto standardAsciiPixmap = standardTemplatePixmap;
                auto painter = QPainter(&standardAsciiPixmap);
                painter.setFont(font);
                painter.setPen(asciiValue.has_value() ? asciiFontColor : fadedFontColor);
                painter.drawText(byteItemRect, Qt::AlignCenter, asciiValue.value_or(hexValue));

                ByteItem::standardAsciiPixmapsByValue.emplace_back(std::move(standardAsciiPixmap));
            }

            {
                auto selectedAsciiPixmap = selectedTemplatePixmap;
                auto painter = QPainter(&selectedAsciiPixmap);
                painter.setFont(font);
                painter.setPen(asciiValue.has_value() ? asciiFontColor : fadedFontColor);
                painter.drawText(byteItemRect, Qt::AlignCenter, asciiValue.value_or(hexValue));

                ByteItem::selectedAsciiPixmapsByValue.emplace_back(std::move(selectedAsciiPixmap));
            }

            {
                auto groupedAsciiPixmap = groupedTemplatePixmap;
                auto painter = QPainter(&groupedAsciiPixmap);
                painter.setFont(font);
                painter.setPen(asciiValue.has_value() ? asciiFontColor : fadedFontColor);
                painter.drawText(byteItemRect, Qt::AlignCenter, asciiValue.value_or(hexValue));

                ByteItem::groupedAsciiPixmapsByValue.emplace_back(std::move(groupedAsciiPixmap));
            }

            {
                auto stackMemoryAsciiPixmap = stackMemoryTemplatePixmap;
                auto painter = QPainter(&stackMemoryAsciiPixmap);
                painter.setFont(font);
                painter.setPen(asciiValue.has_value() ? asciiFontColor : fadedFontColor);
                painter.drawText(byteItemRect, Qt::AlignCenter, asciiValue.value_or(hexValue));

                ByteItem::stackMemoryAsciiPixmapsByValue.emplace_back(std::move(stackMemoryAsciiPixmap));
            }

            {
                auto changedMemoryAsciiPixmap = asciiValue.has_value()
                    ? changedMemoryTemplatePixmap
                    : changedMemoryFadedTemplatePixmap;

                auto painter = QPainter(&changedMemoryAsciiPixmap);
                painter.setFont(font);
                painter.setPen(asciiValue.has_value() ? changedMemoryAsciiFontColor : fadedFontColor);
                painter.drawText(byteItemRect, Qt::AlignCenter, asciiValue.value_or(hexValue));

                ByteItem::changedMemoryAsciiPixmapsByValue.emplace_back(std::move(changedMemoryAsciiPixmap));
            }

            {
                auto hoveredPrimaryAsciiPixmap = hoveredPrimaryTemplatePixmap;
                auto painter = QPainter(&hoveredPrimaryAsciiPixmap);
                painter.setFont(font);
                painter.setPen(asciiValue.has_value() ? asciiFontColor : fadedFontColor);
                painter.drawText(byteItemRect, Qt::AlignCenter, asciiValue.value_or(hexValue));

                ByteItem::hoveredPrimaryAsciiPixmapsByValue.emplace_back(std::move(hoveredPrimaryAsciiPixmap));
            }
        }

        {
            ByteItem::missingDataPixmap = standardTemplatePixmap;
            auto painter = QPainter(&ByteItem::missingDataPixmap.value());
            painter.setFont(font);
            painter.setPen(standardFontColor);
            painter.drawText(byteItemRect, Qt::AlignCenter, "??");
        }

        {
            ByteItem::selectedMissingDataPixmap = selectedTemplatePixmap;
            auto painter = QPainter(&ByteItem::selectedMissingDataPixmap.value());
            painter.setFont(font);
            painter.setPen(standardFontColor);
            painter.drawText(byteItemRect, Qt::AlignCenter, "??");
        }

        ByteItem::pixmapCachesGenerated = true;
    }
}
