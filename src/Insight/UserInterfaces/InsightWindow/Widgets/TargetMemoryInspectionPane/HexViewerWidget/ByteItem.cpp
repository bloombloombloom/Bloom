#include "ByteItem.hpp"

#include <QPainter>

namespace Bloom::Widgets
{
    ByteItem::ByteItem(
        std::size_t byteIndex,
        std::uint32_t address,
        std::optional<std::uint32_t>& currentStackPointer,
        ByteItem** hoveredByteItem,
        AnnotationItem** hoveredAnnotationItem,
        std::set<ByteItem*>& highlightedByteItems,
        const HexViewerWidgetSettings& settings
    )
        : QGraphicsItem(nullptr)
        , byteIndex(byteIndex)
        , address(address)
        , currentStackPointer(currentStackPointer)
        , hoveredByteItem(hoveredByteItem)
        , hoveredAnnotationItem(hoveredAnnotationItem)
        , highlightedByteItems(highlightedByteItems)
        , settings(settings)
    {
        this->setCacheMode(
            QGraphicsItem::CacheMode::ItemCoordinateCache,
            QSize(ByteItem::WIDTH, ByteItem::HEIGHT)
        );
        this->setAcceptHoverEvents(true);

        this->addressHex = "0x" + QString::number(this->address, 16).rightJustified(
            8,
            '0'
        ).toUpper();
        this->relativeAddressHex = "0x" + QString::number(this->byteIndex, 16).rightJustified(
            8,
            '0'
        ).toUpper();

        this->setSelected(false);
    }

    void ByteItem::setValue(unsigned char value) {
        this->value = value;
        this->hexValue = QString::number(this->value, 16).rightJustified(2, '0').toUpper();
        this->asciiValue = (this->value >= 32 && this->value <= 126)
            ? std::optional("'" + QString(QChar(this->value)) + "'") : std::nullopt;

        this->valueInitialised = this->excludedMemoryRegion == nullptr;
        this->update();
    }

    void ByteItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
        painter->setRenderHints(
            QPainter::RenderHint::Antialiasing | QPainter::RenderHint::SmoothPixmapTransform,
            true
        );
        painter->setPen(Qt::PenStyle::NoPen);

        static const auto widgetRect = this->boundingRect();
        static const auto font = QFont("'Ubuntu', sans-serif", 8);

        const auto* backgroundColor = this->getBackgroundColor();
        const auto* textColor = this->getTextColor();

        if (backgroundColor != nullptr) {
            painter->setBrush(*backgroundColor);
            painter->drawRect(widgetRect);
        }

        painter->setFont(font);
        painter->setPen(*textColor);

        if (this->valueInitialised && this->excludedMemoryRegion == nullptr) {
            painter->drawText(
                widgetRect,
                Qt::AlignCenter,
                this->settings.displayAsciiValues && this->asciiValue.has_value()
                    ? this->asciiValue.value()
                    : this->hexValue
            );

        } else {
            static const auto placeholderString = QString("??");
            painter->drawText(widgetRect, Qt::AlignCenter, placeholderString);
        }
    }

    const QColor* ByteItem::getBackgroundColor() {
        /*
         * Due to the sheer number of byte items, painting them can be quite expensive. This function needs to be fast.
         *
         * The background colors vary in alpha value, depending on certain states. We create a static object for each
         * color variant, so that we don't have to make copies or call QColor::setAlpha() for each ByteItem.
         */
        static const auto highlightedBackgroundColor = QColor(0x3C, 0x59, 0x5C, 255);
        static const auto selectedBackgroundColor = QColor(0x3C, 0x59, 0x5C, 255);
        static const auto focusedRegionBackgroundColor = QColor(0x44, 0x44, 0x41, 255);
        static const auto stackMemoryBackgroundColor = QColor(0x67, 0x57, 0x20, 210);

        static const auto disabledHighlightedBackgroundColor = QColor(
            highlightedBackgroundColor.red(),
            highlightedBackgroundColor.green(),
            highlightedBackgroundColor.blue(),
            100
        );

        static const auto disabledSelectedBackgroundColor = QColor(
            selectedBackgroundColor.red(),
            selectedBackgroundColor.green(),
            selectedBackgroundColor.blue(),
            100
        );

        static const auto disabledFocusedRegionBackgroundColor = QColor(
            focusedRegionBackgroundColor.red(),
            focusedRegionBackgroundColor.green(),
            focusedRegionBackgroundColor.blue(),
            100
        );

        static const auto disabledStackMemoryBackgroundColor = QColor(
            stackMemoryBackgroundColor.red(),
            stackMemoryBackgroundColor.green(),
            stackMemoryBackgroundColor.blue(),
            100
        );

        static const auto hoveredStackMemoryBackgroundColor = QColor(
            stackMemoryBackgroundColor.red(),
            stackMemoryBackgroundColor.green(),
            stackMemoryBackgroundColor.blue(),
            255
        );

        static const auto hoveredBackgroundColor = QColor(0x8E, 0x8B, 0x83, 70);
        static const auto hoveredNeighbourBackgroundColor = QColor(0x8E, 0x8B, 0x83, 30);

        if (this->isEnabled()) {
            if (this->highlighted) {
                return &(highlightedBackgroundColor);
            }

            if (this->selected) {
                return &(selectedBackgroundColor);
            }

            const auto* hoveredByteItem = *(this->hoveredByteItem);
            const auto hovered = hoveredByteItem == this;
            const auto hoveredNeighbour =
                !hovered
                && hoveredByteItem != nullptr
                && this->settings.highlightHoveredRowAndCol
                && (
                    hoveredByteItem->currentColumnIndex == this->currentColumnIndex
                    || hoveredByteItem->currentRowIndex == this->currentRowIndex
                );

            if (
                this->settings.highlightStackMemory
                && this->currentStackPointer.has_value()
                && this->address > this->currentStackPointer
            ) {
                return hovered ? &(hoveredStackMemoryBackgroundColor) : &(stackMemoryBackgroundColor);
            }

            if (this->settings.highlightFocusedMemory && this->focusedMemoryRegion != nullptr) {
                return &(focusedRegionBackgroundColor);
            }

            if (hoveredNeighbour) {
                return &(hoveredNeighbourBackgroundColor);
            }

            if (hovered) {
                return &(hoveredBackgroundColor);
            }

        } else {
            if (this->highlighted) {
                return &(disabledHighlightedBackgroundColor);
            }

            if (this->selected) {
                return &(disabledSelectedBackgroundColor);
            }

            if (
                this->settings.highlightStackMemory
                && this->currentStackPointer.has_value()
                && this->address > this->currentStackPointer
            ) {
                return &(disabledStackMemoryBackgroundColor);
            }

            if (this->settings.highlightFocusedMemory && this->focusedMemoryRegion != nullptr) {
                return &(disabledFocusedRegionBackgroundColor);
            }
        }

        return nullptr;
    }

    const QColor* ByteItem::getTextColor() {
        static const auto standardTextColor = QColor(0xAF, 0xB1, 0xB3);
        static const auto asciiModeTextColor = QColor(0xA7, 0x77, 0x26);

        static const auto fadedStandardTextColor = QColor(
            standardTextColor.red(),
            standardTextColor.green(),
            standardTextColor.blue(),
            100
        );

        static const auto fadedAsciiModeTextColor = QColor(
            asciiModeTextColor.red(),
            asciiModeTextColor.green(),
            asciiModeTextColor.blue(),
            100
        );

        const auto displayAsAscii = this->settings.displayAsciiValues && this->asciiValue.has_value();

        if (this->isEnabled() && this->valueInitialised) {
            if (!displayAsAscii) {
                if (
                    this->excludedMemoryRegion != nullptr
                    || this->settings.displayAsciiValues
                    || (!this->highlightedByteItems.empty() && !this->highlighted)
                ) {
                    return &(fadedStandardTextColor);
                }

                return &(standardTextColor);
            }

            return &(asciiModeTextColor);
        }

        return displayAsAscii ? &(fadedAsciiModeTextColor) : &(fadedStandardTextColor);
    }
}
