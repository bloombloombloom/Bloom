#include "ByteItem.hpp"

#include <QFont>
#include <QColor>

namespace Bloom::Widgets
{
    ByteItem::ByteItem(Targets::TargetMemoryAddress address)
        : HexViewerItem(address)
    {
        if (ByteItem::standardPixmapsByValue.empty()) {
            ByteItem::generatePixmapCaches();
        }
    }

    void ByteItem::paint(
        QPainter* painter,
        const HexViewerSharedState* hexViewerState,
        const QGraphicsItem* graphicsItem
    ) const {
        const auto boundingRect = QRect(0, 0, ByteItem::WIDTH, ByteItem::HEIGHT);

        if (!graphicsItem->isEnabled()) {
            painter->setOpacity(0.6);
        }

        if (this->excluded || !hexViewerState->data.has_value()) {
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
        static const auto standardBackgroundColor = QColor(0x32, 0x33, 0x30, 255);
        static const auto highlightedBackgroundColor = QColor(0x3C, 0x59, 0x5C, 255);
        static const auto selectedBackgroundColor = QColor(0x3C, 0x59, 0x5C, 255);
        static const auto groupedBackgroundColor = QColor(0x44, 0x44, 0x41, 255);
        static const auto stackMemoryBackgroundColor = QColor(0x67, 0x57, 0x20, 210);

        static const auto hoveredStackMemoryBackgroundColor = QColor(
            stackMemoryBackgroundColor.red(),
            stackMemoryBackgroundColor.green(),
            stackMemoryBackgroundColor.blue(),
            255
        );

        static const auto hoveredBackgroundColor = QColor(0x8E, 0x8B, 0x83, 70);
        static const auto hoveredSecondaryBackgroundColor = QColor(0x8E, 0x8B, 0x83, 30);

        static const auto standardFontColor = QColor(0xAF, 0xB1, 0xB3);
        static const auto fadedFontColor = QColor(0xAF, 0xB1, 0xB3, 100);
        static const auto asciiFontColor = QColor(0xA7, 0x77, 0x26);

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

        auto hoveredStackMemoryTemplatePixmap = QPixmap(byteItemSize);
        hoveredStackMemoryTemplatePixmap.fill(hoveredStackMemoryBackgroundColor);

        auto hoveredPrimaryTemplatePixmap = QPixmap(byteItemSize);
        hoveredPrimaryTemplatePixmap.fill(hoveredBackgroundColor);

        auto hoveredSecondaryTemplatePixmap = QPixmap(byteItemSize);
        hoveredSecondaryTemplatePixmap.fill(hoveredSecondaryBackgroundColor);

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
                auto hoveredPrimaryPixmap = hoveredPrimaryTemplatePixmap;
                auto painter = QPainter(&hoveredPrimaryPixmap);
                painter.setFont(font);
                painter.setPen(standardFontColor);
                painter.drawText(byteItemRect, Qt::AlignCenter, hexValue);

                ByteItem::hoveredPrimaryPixmapsByValue.emplace_back(std::move(hoveredPrimaryPixmap));
            }

            {
                auto hoveredSecondaryPixmap = hoveredSecondaryTemplatePixmap;
                auto painter = QPainter(&hoveredSecondaryPixmap);
                painter.setFont(font);
                painter.setPen(standardFontColor);
                painter.drawText(byteItemRect, Qt::AlignCenter, hexValue);

                ByteItem::hoveredSecondaryPixmapsByValue.emplace_back(std::move(hoveredSecondaryPixmap));
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
                auto hoveredPrimaryAsciiPixmap = hoveredPrimaryTemplatePixmap;
                auto painter = QPainter(&hoveredPrimaryAsciiPixmap);
                painter.setFont(font);
                painter.setPen(asciiValue.has_value() ? asciiFontColor : fadedFontColor);
                painter.drawText(byteItemRect, Qt::AlignCenter, asciiValue.value_or(hexValue));

                ByteItem::hoveredPrimaryAsciiPixmapsByValue.emplace_back(std::move(hoveredPrimaryAsciiPixmap));
            }

            {
                auto hoveredSecondaryAsciiPixmap = hoveredSecondaryTemplatePixmap;
                auto painter = QPainter(&hoveredSecondaryAsciiPixmap);
                painter.setFont(font);
                painter.setPen(asciiValue.has_value() ? asciiFontColor : fadedFontColor);
                painter.drawText(byteItemRect, Qt::AlignCenter, asciiValue.value_or(hexValue));

                ByteItem::hoveredSecondaryAsciiPixmapsByValue.emplace_back(std::move(hoveredSecondaryAsciiPixmap));
            }
        }

        {
            ByteItem::missingDataPixmap = standardTemplatePixmap;
            auto painter = QPainter(&ByteItem::missingDataPixmap.value());
            painter.setFont(font);
            painter.setPen(standardFontColor);
            painter.drawText(byteItemRect, Qt::AlignCenter, "??");
        }
    }
}
