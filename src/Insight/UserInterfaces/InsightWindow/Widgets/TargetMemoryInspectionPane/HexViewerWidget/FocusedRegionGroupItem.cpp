#include "FocusedRegionGroupItem.hpp"

#include <cassert>
#include <QColor>

namespace Bloom::Widgets
{
    FocusedRegionGroupItem::FocusedRegionGroupItem(
        const FocusedMemoryRegion& focusedRegion,
        std::unordered_map<Targets::TargetMemoryAddress, ByteItem>& byteItemsByAddress,
        HexViewerItem* parent
    )
        : GroupItem(focusedRegion.addressRange.startAddress, parent)
        , focusedMemoryRegion(focusedRegion)
    {
        const auto& startAddress = this->focusedMemoryRegion.addressRange.startAddress;
        const auto& endAddress = this->focusedMemoryRegion.addressRange.endAddress;

        // Sanity check
        assert(byteItemsByAddress.contains(startAddress) && byteItemsByAddress.contains(endAddress));

        for (auto address = startAddress; address <= endAddress; ++address) {
            auto& byteItem = byteItemsByAddress.at(address);
            byteItem.parent = this;
            byteItem.grouped = true;

            this->items.push_back(&byteItem);
        }
    }

    void FocusedRegionGroupItem::refreshValue(const HexViewerSharedState& hexViewerState) {
        if (!hexViewerState.data.has_value()) {
            this->valueLabel = std::nullopt;
            return;
        }

        const auto regionStartAddress = this->focusedMemoryRegion.addressRange.startAddress;
        const auto regionEndAddress = this->focusedMemoryRegion.addressRange.endAddress;
        const auto startIndex = regionStartAddress - hexViewerState.memoryDescriptor.addressRange.startAddress;
        auto value = Targets::TargetMemoryBuffer(
            hexViewerState.data->begin() + startIndex,
            hexViewerState.data->begin() + startIndex + (regionEndAddress - regionStartAddress + 1)
        );

        if (this->focusedMemoryRegion.endianness == Targets::TargetMemoryEndianness::LITTLE) {
            std::reverse(value.begin(), value.end());
        }

        switch (this->focusedMemoryRegion.dataType) {
            case MemoryRegionDataType::UNSIGNED_INTEGER: {
                std::uint64_t integerValue = 0;
                for (const auto& byte : value) {
                    integerValue = (integerValue << 8) | byte;
                }

                this->valueLabel = QString::number(integerValue);
                break;
            }
            case MemoryRegionDataType::SIGNED_INTEGER: {
                const auto valueSize = value.size();

                if (valueSize == 1) {
                    this->valueLabel = QString::number(static_cast<int8_t>(value[0]));
                    break;
                }

                if (valueSize == 2) {
                    this->valueLabel = QString::number(static_cast<int16_t>((value[0] << 8) | value[1]));
                    break;
                }

                if (valueSize <= 4) {
                    std::int32_t integerValue = 0;
                    for (const auto& byte : value) {
                        integerValue = (integerValue << 8) | byte;
                    }

                    this->valueLabel = QString::number(integerValue);
                    break;
                }

                std::int64_t integerValue = 0;
                for (const auto& byte : value) {
                    integerValue = (integerValue << 8) | byte;
                }

                this->valueLabel = QString::number(integerValue);
                break;
            }
            case MemoryRegionDataType::ASCII_STRING: {
                // Replace non-ASCII chars with '?'
                auto asciiData = value;

                std::replace_if(
                    asciiData.begin(),
                    asciiData.end(),
                    [] (unsigned char value) {
                        /*
                         * We only care about non-control characters (except for the white space character) in
                         * the standard ASCII range.
                         */
                        return value < 32 || value > 126;
                    },
                    '?'
                );

                this->valueLabel = "'" + QString::fromLatin1(
                    reinterpret_cast<const char*>(asciiData.data()),
                    static_cast<qsizetype>(asciiData.size())
                ) + "'";
                break;
            }
            default: {
                this->valueLabel = std::nullopt;
            }
        }
    }

    void FocusedRegionGroupItem::paint(
        QPainter* painter,
        const HexViewerSharedState* hexViewerState,
        const QGraphicsItem* graphicsItem
    ) const {
        if (!hexViewerState->settings.displayAnnotations) {
            return;
        }

        painter->setRenderHints(QPainter::RenderHint::Antialiasing, false);

        if (!graphicsItem->isEnabled()) {
            painter->setOpacity(0.5);
        }

        this->paintRegionNameAnnotation(painter, hexViewerState, graphicsItem);

        if (this->focusedMemoryRegion.dataType != MemoryRegionDataType::UNKNOWN) {
            // Paint the value annotation
            this->paintValueAnnotation(painter, hexViewerState, graphicsItem);
        }
    }

    QMargins FocusedRegionGroupItem::groupMargins(
        const HexViewerSharedState* hexViewerState,
        const int maximumWidth
    ) const {
        if (hexViewerState->settings.displayAnnotations) {
            constexpr auto averageSymbolWidth = 6;
            const auto nameLabelWidth = static_cast<int>(this->focusedMemoryRegion.name.size() * averageSymbolWidth);
            const auto valueLabelWidth = static_cast<int>(
                this->valueLabel.has_value() ? this->valueLabel->size() * averageSymbolWidth : 0
            );

            const auto minimumWidth = std::min(std::max(nameLabelWidth, valueLabelWidth), maximumWidth);

            const auto byteItemSize = (this->focusedMemoryRegion.addressRange.endAddress
                - this->focusedMemoryRegion.addressRange.startAddress + 1);
            const auto estimatedWidth = static_cast<int>(
                byteItemSize * (ByteItem::WIDTH + (ByteItem::RIGHT_MARGIN / 2))
            );

            const auto annotationMargin = static_cast<int>(
                estimatedWidth < minimumWidth ? minimumWidth - estimatedWidth : 0
            );

            return QMargins(
                annotationMargin / 2,
                this->focusedMemoryRegion.dataType != MemoryRegionDataType::UNKNOWN
                    ? FocusedRegionGroupItem::ANNOTATION_HEIGHT
                    : 0,
                annotationMargin / 2,
                FocusedRegionGroupItem::ANNOTATION_HEIGHT
            );
        }

        return QMargins(0, 0, 0, 0);
    }

    void FocusedRegionGroupItem::paintRegionNameAnnotation(
        QPainter* painter,
        const HexViewerSharedState* hexViewerState,
        const QGraphicsItem* graphicsItem
    ) const {
        auto labelText = this->focusedMemoryRegion.name;

        static const auto lineColor = QColor(0x4F, 0x4F, 0x4F);
        static const auto labelFontColor = QColor(0x68, 0x68, 0x68);

        static auto labelFont = QFont("'Ubuntu', sans-serif");
        labelFont.setPixelSize(12);

        painter->setFont(labelFont);

        const auto groupWidth = this->groupSize.width();
        const auto fontMetrics = painter->fontMetrics();
        auto labelSize = fontMetrics.size(Qt::TextSingleLine, labelText);
        if (labelSize.width() > groupWidth) {
            labelSize.setWidth(groupWidth);
            labelText = fontMetrics.elidedText(
                labelText,
                Qt::TextElideMode::ElideRight,
                groupWidth
            );
        }

        const auto heightOffset = this->groupSize.height() - FocusedRegionGroupItem::ANNOTATION_HEIGHT + 4;

        const auto verticalLineYStart = static_cast<int>(heightOffset);
        const auto verticalLineYEnd = static_cast<int>(heightOffset + 5);

        const auto labelRect = QRect(
            (groupWidth - labelSize.width()) / 2,
            verticalLineYEnd + 10,
            labelSize.width(),
            labelSize.height()
        );

        painter->setPen(lineColor);

        if (this->focusedMemoryRegion.addressRange.startAddress != this->focusedMemoryRegion.addressRange.endAddress) {
            const auto lineStartX = this->items.front()->relativePosition.x() + (ByteItem::WIDTH / 2);
            const auto lineEndX = this->multiLine
                ? groupWidth - + (ByteItem::WIDTH / 2)
                : this->items.back()->relativePosition.x() + (ByteItem::WIDTH / 2);

            painter->drawLine(QLine(
                lineStartX,
                verticalLineYStart,
                lineStartX,
                verticalLineYEnd
            ));

            painter->drawLine(QLine(
                lineEndX,
                verticalLineYStart,
                lineEndX,
                verticalLineYEnd
            ));

            painter->drawLine(QLine(
                lineStartX,
                verticalLineYEnd,
                lineEndX,
                verticalLineYEnd
            ));
        }

        painter->drawLine(QLine(
            groupWidth / 2,
            verticalLineYEnd,
            groupWidth / 2,
            verticalLineYEnd + 4
        ));

        painter->setPen(labelFontColor);
        painter->drawText(labelRect, Qt::AlignCenter, labelText);
    }

    void FocusedRegionGroupItem::paintValueAnnotation(
        QPainter* painter,
        const HexViewerSharedState* hexViewerState,
        const QGraphicsItem* graphicsItem
    ) const {
        using Targets::TargetMemoryEndianness;

        auto labelText = this->valueLabel.value_or("??");

        static const auto lineColor = QColor(0x4F, 0x4F, 0x4F);
        static const auto labelFontColor = QColor(0x94, 0x6F, 0x30);

        static auto labelFont = QFont("'Ubuntu', sans-serif");
        labelFont.setPixelSize(12);
        labelFont.setItalic(true);

        painter->setFont(labelFont);

        const auto groupWidth = this->groupSize.width();
        const auto fontMetrics = painter->fontMetrics();
        auto labelSize = fontMetrics.size(Qt::TextSingleLine, labelText);
        if (labelSize.width() > groupWidth) {
            labelSize.setWidth(groupWidth);
            labelText = fontMetrics.elidedText(
                labelText,
                Qt::TextElideMode::ElideRight,
                groupWidth
            );
        }

        const auto heightOffset = FocusedRegionGroupItem::ANNOTATION_HEIGHT - 4;

        const auto verticalLineYStart = static_cast<int>(heightOffset - 5);
        const auto verticalLineYEnd = static_cast<int>(heightOffset);

        const auto labelRect = QRect(
            (groupWidth - labelSize.width()) / 2,
            verticalLineYStart - 10 - labelSize.height(),
            labelSize.width(),
            labelSize.height()
        );

        painter->setPen(lineColor);

        if (this->focusedMemoryRegion.addressRange.startAddress != this->focusedMemoryRegion.addressRange.endAddress) {
            const auto lineStartX = this->items.front()->relativePosition.x() + (ByteItem::WIDTH / 2);
            const auto lineEndX = this->multiLine
                ? groupWidth - + (ByteItem::WIDTH / 2)
                : this->items.back()->relativePosition.x() + (ByteItem::WIDTH / 2);

            painter->drawLine(QLine(
                lineStartX,
                verticalLineYStart,
                lineStartX,
                verticalLineYEnd
            ));

            painter->drawLine(QLine(
                lineEndX,
                verticalLineYStart,
                lineEndX,
                verticalLineYEnd
            ));

            painter->drawLine(QLine(
                lineStartX,
                verticalLineYStart,
                lineEndX,
                verticalLineYStart
            ));

            /*
             * Draw a circle just above the first byte item of the region, to indicate the first byte used to generate
             * the value (which will depend on the configured endianness of the region).
             */
            painter->setBrush(lineColor);

            constexpr auto radius = 2;
            painter->drawEllipse(
                QPoint(
                    this->focusedMemoryRegion.endianness == TargetMemoryEndianness::BIG ? lineStartX : lineEndX,
                    !this->multiLine || this->focusedMemoryRegion.endianness == TargetMemoryEndianness::BIG
                        ? verticalLineYStart
                        : this->groupSize.height() - FocusedRegionGroupItem::ANNOTATION_HEIGHT + 4 + 5
                ),
                radius,
                radius
            );
        }

        painter->drawLine(QLine(
            groupWidth / 2,
            verticalLineYStart - 4,
            groupWidth / 2,
            verticalLineYStart
        ));

        painter->setPen(labelFontColor);
        painter->drawText(labelRect, Qt::AlignCenter, labelText);
    }
}
