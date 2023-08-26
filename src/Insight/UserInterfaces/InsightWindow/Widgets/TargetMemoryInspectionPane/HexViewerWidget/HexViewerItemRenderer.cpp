#include "HexViewerItemRenderer.hpp"

#include <QScrollBar>
#include <QColor>
#include <QPainterPath>

namespace Widgets
{
    HexViewerItemRenderer::HexViewerItemRenderer(
        const HexViewerSharedState& hexViewerState,
        const TopLevelGroupItem& topLevelGroupItem,
        const HexViewerItemIndex& itemIndex,
        const QGraphicsView* view
    )
        : hexViewerState(hexViewerState)
        , topLevelGroupItem(topLevelGroupItem)
        , itemIndex(itemIndex)
        , view(view)
        , viewport(view->viewport())
    {
        this->setAcceptHoverEvents(true);
        this->setCacheMode(QGraphicsItem::CacheMode::NoCache);

        if (!HexViewerItemRenderer::pixmapCachesGenerated) {
            HexViewerItemRenderer::generatePixmapCaches();
        }
    }

    void HexViewerItemRenderer::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
        const auto viewportYStart = this->view->verticalScrollBar()->value();
        const auto viewportYEnd = viewportYStart + this->viewport->size().height();

        const auto visibleItems = this->itemIndex.items(viewportYStart, viewportYEnd);

        painter->setRenderHints(QPainter::RenderHint::Antialiasing, false);

        // Paint the ancestors of the first visible item
        const auto& firstItem = *(visibleItems.begin());

        auto* parentItem = firstItem->parent;
        while (parentItem != nullptr) {
            this->paintItem(parentItem, painter);
            parentItem = parentItem->parent;
            painter->setOpacity(1);
        }

        for (auto& item : visibleItems) {
            this->paintItem(item, painter);
            painter->setOpacity(1);
        }

        if (this->hexViewerState.highlightingEnabled) {
            for (const auto& range : this->hexViewerState.highlightedPrimaryAddressRanges) {
                const auto& startItem = this->topLevelGroupItem.byteItemsByAddress.at(range.startAddress);
                const auto& endItem = this->topLevelGroupItem.byteItemsByAddress.at(range.endAddress);

                const auto startItemY = startItem.position().y();
                const auto endItemY = endItem.position().y();

                if (startItemY > viewportYEnd) {
                    break;
                }

                if (endItemY < viewportYStart) {
                    continue;
                }

                this->paintPrimaryHighlightBorder(&startItem, &endItem, painter);
            }
        }
    }

    void HexViewerItemRenderer::paintItem(const HexViewerItem* item, QPainter* painter) {
        const auto* byteItem = dynamic_cast<const ByteItem*>(item);

        if (byteItem != nullptr) {
            return this->paintByteItem(byteItem, painter);
        }

        const auto* focusedRegionItem = dynamic_cast<const FocusedRegionGroupItem*>(item);

        if (focusedRegionItem != nullptr) {
            return this->paintFocusedRegionGroupItem(focusedRegionItem, painter);
        }

        const auto* stackMemoryItem = dynamic_cast<const StackMemoryGroupItem*>(item);

        if (stackMemoryItem != nullptr) {
            return this->paintStackMemoryGroupItem(stackMemoryItem, painter);
        }
    }

    void HexViewerItemRenderer::paintByteItem(const ByteItem* item, QPainter* painter) {
        const auto position = item->position();
        const auto boundingRect = QRect(position.x(), position.y(), ByteItem::WIDTH, ByteItem::HEIGHT);

        painter->setOpacity(
            !this->isEnabled()
            || (item->excluded && !item->selected)
            || (this->hexViewerState.highlightingEnabled && !item->primaryHighlighted)
                ? 0.6
                : 1
        );

        if (item->excluded || !this->hexViewerState.data.has_value()) {
            if (item->selected) {
                painter->drawPixmap(boundingRect, HexViewerItemRenderer::selectedMissingDataPixmap.value());
                return;
            }

            if (this->hexViewerState.highlightingEnabled && item->primaryHighlighted) {
                painter->drawPixmap(boundingRect, HexViewerItemRenderer::primaryHighlightedMissingDataPixmap.value());
                return;
            }

            painter->drawPixmap(boundingRect, HexViewerItemRenderer::missingDataPixmap.value());
            return;
        }

        const auto byteIndex = item->startAddress - this->hexViewerState.memoryDescriptor.addressRange.startAddress;
        const auto value = (*(this->hexViewerState.data))[byteIndex];

        const auto hoveredPrimary = this->hexViewerState.hoveredByteItem == item;

        if (this->hexViewerState.settings.displayAsciiValues) {
            if (item->selected) {
                painter->drawPixmap(
                    boundingRect,
                    HexViewerItemRenderer::selectedAsciiPixmapsByValue[value]
                );
                return;
            }

            if (this->hexViewerState.highlightingEnabled && item->primaryHighlighted) {
                painter->drawPixmap(
                    boundingRect,
                    HexViewerItemRenderer::primaryHighlightedAsciiPixmapsByValue[value]
                );
                return;
            }

            if (item->changed) {
                painter->drawPixmap(
                    boundingRect,
                    HexViewerItemRenderer::changedMemoryAsciiPixmapsByValue[value]
                );
                return;
            }

            if (item->stackMemory && this->hexViewerState.settings.groupStackMemory) {
                painter->drawPixmap(
                    boundingRect,
                    HexViewerItemRenderer::stackMemoryAsciiPixmapsByValue[value]
                );
                return;
            }

            if (item->grouped && this->hexViewerState.settings.highlightFocusedMemory) {
                painter->drawPixmap(
                    boundingRect,
                    HexViewerItemRenderer::groupedAsciiPixmapsByValue[value]
                );
                return;
            }

            if (hoveredPrimary) {
                painter->drawPixmap(
                    boundingRect,
                    HexViewerItemRenderer::hoveredPrimaryAsciiPixmapsByValue[value]
                );
                return;
            }

            painter->drawPixmap(boundingRect, HexViewerItemRenderer::standardAsciiPixmapsByValue[value]);
            return;
        }

        if (item->selected) {
            painter->drawPixmap(
                boundingRect,
                HexViewerItemRenderer::selectedPixmapsByValue[value]
            );
            return;
        }

        if (this->hexViewerState.highlightingEnabled && item->primaryHighlighted) {
            painter->drawPixmap(
                boundingRect,
                HexViewerItemRenderer::primaryHighlightedPixmapsByValue[value]
            );
            return;
        }

        if (item->changed) {
            painter->drawPixmap(
                boundingRect,
                HexViewerItemRenderer::changedMemoryPixmapsByValue[value]
            );
            return;
        }

        if (item->stackMemory && this->hexViewerState.settings.groupStackMemory) {
            painter->drawPixmap(
                boundingRect,
                HexViewerItemRenderer::stackMemoryPixmapsByValue[value]
            );
            return;
        }

        if (item->grouped && this->hexViewerState.settings.highlightFocusedMemory) {
            painter->drawPixmap(
                boundingRect,
                HexViewerItemRenderer::groupedPixmapsByValue[value]
            );
            return;
        }

        if (hoveredPrimary) {
            painter->drawPixmap(
                boundingRect,
                HexViewerItemRenderer::hoveredPrimaryPixmapsByValue[value]
            );
            return;
        }

        painter->drawPixmap(
            boundingRect,
            HexViewerItemRenderer::standardPixmapsByValue[value]
        );
    }

    void HexViewerItemRenderer::paintPrimaryHighlightBorder(
        const ByteItem* startItem,
        const ByteItem* endItem,
        QPainter* painter
    ) {
        constexpr auto padding = 6;
        constexpr auto rectRadius = 4;

        const auto startItemPos = startItem->position();
        const auto endItemPos = endItem->position();
        const auto endItemSize = endItem->size();

        auto painterPath = QPainterPath();

        if (startItemPos.y() != endItemPos.y()) {
            // The highlighted range spans more than one line - draw the border around all lines containing the range
            const auto leftMostItem = this->itemIndex.leftMostByteItemWithinRange(startItemPos.y(), endItemPos.y());
            const auto leftMostItemPos = leftMostItem->position();

            const auto* rightMostItem = this->itemIndex.rightMostByteItemWithinRange(startItemPos.y(), endItemPos.y());
            const auto rightMostItemPos = rightMostItem->position();

            painterPath.addRoundedRect(
                leftMostItemPos.x() - padding,
                startItemPos.y() - padding,
                (rightMostItemPos.x() + rightMostItem->size().width()) - leftMostItemPos.x() + (padding * 2),
                (endItemPos.y() + endItemSize.height()) - startItemPos.y() + (padding * 2),
                rectRadius,
                rectRadius
            );

        } else {
            painterPath.addRoundedRect(
                startItemPos.x() - padding,
                startItemPos.y() - padding,
                (endItemPos.x() + endItemSize.width()) - startItemPos.x() + (padding * 2),
                (endItemPos.y() + endItemSize.height()) - startItemPos.y() + (padding * 2),
                rectRadius,
                rectRadius
            );
        }

        painter->setRenderHints(QPainter::RenderHint::Antialiasing | QPainter::RenderHint::SmoothPixmapTransform, true);
        painter->setPen(QPen(QColor(0x58, 0x58, 0x58), 2));
        painter->setBrush(Qt::BrushStyle::NoBrush);
        painter->drawPath(painterPath);
    }

    void HexViewerItemRenderer::paintFocusedRegionGroupItem(const FocusedRegionGroupItem* item, QPainter* painter) {
        if (!this->hexViewerState.settings.displayAnnotations) {
            return;
        }

        const auto position = item->position();

        // The region name label
        {
            auto labelText = item->focusedMemoryRegion.name;

            static constexpr auto lineColor = QColor(0x4F, 0x4F, 0x4F);
            static constexpr auto labelFontColor = QColor(0x68, 0x68, 0x68);

            static auto labelFont = QFont("'Ubuntu', sans-serif");
            labelFont.setPixelSize(12);

            painter->setFont(labelFont);

            const auto groupWidth = item->groupSize.width();
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

            const auto heightOffset = item->groupSize.height() - FocusedRegionGroupItem::ANNOTATION_HEIGHT + 4;

            const auto verticalLineYStart = position.y() + static_cast<int>(heightOffset);
            const auto verticalLineYEnd = position.y() + static_cast<int>(heightOffset + 5);

            const auto labelRect = QRect(
                position.x() + (groupWidth - labelSize.width()) / 2,
                verticalLineYEnd + 10,
                labelSize.width(),
                labelSize.height()
            );

            painter->setPen(lineColor);

            if (item->focusedMemoryRegion.addressRange.startAddress !=
                item->focusedMemoryRegion.addressRange.endAddress) {
                const auto lineStartX =
                    position.x() + item->items.front()->relativePosition.x() + (ByteItem::WIDTH / 2);
                const auto lineEndX = item->multiLine
                    ? position.x() + groupWidth - (ByteItem::WIDTH / 2)
                    : position.x() + item->items.back()->relativePosition.x() + (ByteItem::WIDTH / 2);

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
                position.x() + groupWidth / 2,
                verticalLineYEnd,
                position.x() + groupWidth / 2,
                verticalLineYEnd + 4
            ));

            painter->setPen(labelFontColor);
            painter->drawText(labelRect, Qt::AlignCenter, labelText);
        }

        // The value label
        if (item->focusedMemoryRegion.dataType != MemoryRegionDataType::UNKNOWN) {
            using Targets::TargetMemoryEndianness;

            auto labelText = item->valueLabel.value_or("??");

            static const auto lineColor = QColor(0x4F, 0x4F, 0x4F);
            static const auto labelFontColor = QColor(0x94, 0x6F, 0x30);

            static auto labelFont = QFont("'Ubuntu', sans-serif");
            labelFont.setPixelSize(12);
            labelFont.setItalic(true);

            painter->setFont(labelFont);

            const auto groupWidth = item->groupSize.width();
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

            const auto verticalLineYStart = position.y() + static_cast<int>(heightOffset - 5);
            const auto verticalLineYEnd = position.y() + static_cast<int>(heightOffset);

            const auto labelRect = QRect(
                position.x() + (groupWidth - labelSize.width()) / 2,
                verticalLineYStart - 10 - labelSize.height(),
                labelSize.width(),
                labelSize.height()
            );

            painter->setPen(lineColor);

            if (item->focusedMemoryRegion.addressRange.startAddress != item->focusedMemoryRegion.addressRange.endAddress) {
                const auto lineStartX = position.x() + item->items.front()->relativePosition.x() + (ByteItem::WIDTH / 2);
                const auto lineEndX = item->multiLine
                    ? position.x() + groupWidth - + (ByteItem::WIDTH / 2)
                    : position.x() + item->items.back()->relativePosition.x() + (ByteItem::WIDTH / 2);

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
                        item->focusedMemoryRegion.endianness == TargetMemoryEndianness::BIG ? lineStartX : lineEndX,
                        !item->multiLine || item->focusedMemoryRegion.endianness == TargetMemoryEndianness::BIG
                            ? verticalLineYStart
                            : position.y() + item->groupSize.height() - FocusedRegionGroupItem::ANNOTATION_HEIGHT + 4 + 5
                    ),
                    radius,
                    radius
                );
            }

            painter->drawLine(QLine(
                position.x() + groupWidth / 2,
                verticalLineYStart - 4,
                position.x() + groupWidth / 2,
                verticalLineYStart
            ));

            painter->setPen(labelFontColor);
            painter->drawText(labelRect, Qt::AlignCenter, labelText);
        }
    }

    void HexViewerItemRenderer::paintStackMemoryGroupItem(const StackMemoryGroupItem* item, QPainter* painter) {
        const auto position = item->position();

        const auto headingText = QString("Stack Memory");

        const auto stackSize = this->hexViewerState.memoryDescriptor.addressRange.endAddress - item->startAddress + 1;
        const auto& memoryCapacity = this->hexViewerState.memoryDescriptor.size();

        const auto stackSizeHeadingText = QString("Stack size:");
        const auto stackSizeValueText = QString::number(stackSize) + " byte(s) ("
            + QString::number(static_cast<float>(stackSize) / static_cast<float>(memoryCapacity / 100), 'f' , 1)
            + "% of memory capacity)";

        const auto stackPointerHeadingText = QString("Stack pointer:");
        const auto stackPointerValueText = "0x" + QString::number(
            item->stackPointer,
            16
        ).rightJustified(8, '0').toUpper();

        static constexpr auto lineColor = QColor(0x4F, 0x4F, 0x4F);
        static constexpr auto headingLabelFontColor = QColor(0x6F, 0x6F, 0x6F);
        static constexpr auto valueLabelFontColor = QColor(0x94, 0x6F, 0x30, 230);

        static auto headingLabelFont = QFont("'Ubuntu', sans-serif");
        headingLabelFont.setPixelSize(13);

        static auto valueFont = QFont("'Ubuntu', sans-serif");
        valueFont.setPixelSize(13);
        valueFont.setItalic(true);

        painter->setFont(headingLabelFont);

        const auto groupWidth = item->groupSize.width();
        const auto fontMetrics = painter->fontMetrics();

        auto headingLabelSize = fontMetrics.size(Qt::TextSingleLine, headingText);

        auto stackSizeHeadingLabelSize = fontMetrics.size(Qt::TextSingleLine, stackSizeHeadingText);
        auto stackSizeLabelSize = fontMetrics.size(Qt::TextSingleLine, stackSizeValueText);

        auto stackPointerHeadingLabelSize = fontMetrics.size(Qt::TextSingleLine, stackPointerHeadingText);
        auto stackPointerLabelSize = fontMetrics.size(Qt::TextSingleLine, stackPointerValueText);

        static constexpr auto labelLineHeight = 4;
        static constexpr auto labelBottomMargin = 10;
        static constexpr auto labelRightMargin = 3;

        const auto heightOffset = headingLabelSize.height() + stackSizeHeadingLabelSize.height()
            + stackPointerHeadingLabelSize.height() + (labelBottomMargin * 3) + 15;

        const auto verticalLineYStart = static_cast<int>(position.y() + heightOffset - 5);
        const auto verticalLineYEnd = static_cast<int>(position.y() + heightOffset);

        const auto lineStartX = position.x() + ByteItem::WIDTH / 2;
        const auto lineEndX = position.x() + groupWidth - (ByteItem::WIDTH / 2);

        const auto labelRect = QRect(
            position.x() + (groupWidth - headingLabelSize.width()) / 2,
            verticalLineYStart - stackPointerHeadingLabelSize.height() - stackSizeHeadingLabelSize.height()
                - headingLabelSize.height() - labelLineHeight - (labelBottomMargin * 2) - 3,
            headingLabelSize.width(),
            headingLabelSize.height()
        );

        const auto stackPointerHeadingLabelRect = QRect(
            labelRect.left() + (labelRect.width() / 2) - (
                (stackPointerHeadingLabelSize.width() + stackPointerLabelSize.width()) / 2
            ),
            labelRect.bottom() + labelBottomMargin,
            stackPointerHeadingLabelSize.width(),
            stackPointerHeadingLabelSize.height()
        );

        const auto stackPointerValueLabelRect = QRect(
            stackPointerHeadingLabelRect.right() + labelRightMargin,
            labelRect.bottom() + labelBottomMargin,
            stackPointerLabelSize.width(),
            stackPointerLabelSize.height()
        );

        const auto stackSizeHeadingLabelRect = QRect(
            labelRect.left() + (labelRect.width() / 2) - (
                (stackSizeHeadingLabelSize.width() + stackSizeLabelSize.width()) / 2
            ),
            stackPointerHeadingLabelRect.bottom() + labelBottomMargin,
            stackSizeHeadingLabelSize.width(),
            stackSizeHeadingLabelSize.height()
        );

        const auto stackSizeValueLabelRect = QRect(
            stackSizeHeadingLabelRect.right() + labelRightMargin,
            stackPointerHeadingLabelRect.bottom() + labelBottomMargin,
            stackSizeLabelSize.width(),
            stackSizeLabelSize.height()
        );

        painter->setPen(lineColor);

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

        painter->drawLine(QLine(
            position.x() + groupWidth / 2,
            verticalLineYStart - labelLineHeight,
            position.x() + groupWidth / 2,
            verticalLineYStart
        ));

        painter->setPen(headingLabelFontColor);

        painter->drawText(labelRect, Qt::AlignCenter, headingText);
        painter->drawText(stackSizeHeadingLabelRect, Qt::AlignCenter, stackSizeHeadingText);
        painter->drawText(stackPointerHeadingLabelRect, Qt::AlignCenter, stackPointerHeadingText);

        painter->setFont(valueFont);
        painter->setPen(valueLabelFontColor);
        painter->drawText(stackSizeValueLabelRect, Qt::AlignCenter, stackSizeValueText);
        painter->drawText(stackPointerValueLabelRect, Qt::AlignCenter, stackPointerValueText);
    }

    void HexViewerItemRenderer::generatePixmapCaches() {
        const auto lock = std::unique_lock(HexViewerItemRenderer::pixmapCacheMutex);

        if (HexViewerItemRenderer::pixmapCachesGenerated) {
            return;
        }

        static constexpr auto standardBackgroundColor = QColor(0x32, 0x33, 0x30, 0);
        static constexpr auto selectedBackgroundColor = QColor(0x3C, 0x59, 0x5C, 255);
        static constexpr auto primaryHighlightedBackgroundColor = QColor(0x3B, 0x59, 0x37, 255);
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

        auto primaryHighlightedTemplatePixmap = QPixmap(byteItemSize);
        primaryHighlightedTemplatePixmap.fill(primaryHighlightedBackgroundColor);

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

                HexViewerItemRenderer::standardPixmapsByValue.emplace_back(std::move(standardPixmap));
            }

            {
                auto selectedPixmap = selectedTemplatePixmap;
                auto painter = QPainter(&selectedPixmap);
                painter.setFont(font);
                painter.setPen(standardFontColor);
                painter.drawText(byteItemRect, Qt::AlignCenter, hexValue);

                HexViewerItemRenderer::selectedPixmapsByValue.emplace_back(std::move(selectedPixmap));
            }

            {
                auto primaryHighlightedPixmap = primaryHighlightedTemplatePixmap;
                auto painter = QPainter(&primaryHighlightedPixmap);
                painter.setFont(font);
                painter.setPen(standardFontColor);
                painter.drawText(byteItemRect, Qt::AlignCenter, hexValue);

                HexViewerItemRenderer::primaryHighlightedPixmapsByValue.emplace_back(std::move(primaryHighlightedPixmap));
            }

            {
                auto groupedPixmap = groupedTemplatePixmap;
                auto painter = QPainter(&groupedPixmap);
                painter.setFont(font);
                painter.setPen(standardFontColor);
                painter.drawText(byteItemRect, Qt::AlignCenter, hexValue);

                HexViewerItemRenderer::groupedPixmapsByValue.emplace_back(std::move(groupedPixmap));
            }

            {
                auto stackMemoryPixmap = stackMemoryTemplatePixmap;
                auto painter = QPainter(&stackMemoryPixmap);
                painter.setFont(font);
                painter.setPen(standardFontColor);
                painter.drawText(byteItemRect, Qt::AlignCenter, hexValue);

                HexViewerItemRenderer::stackMemoryPixmapsByValue.emplace_back(std::move(stackMemoryPixmap));
            }

            {
                auto changedMemoryPixmap = changedMemoryTemplatePixmap;
                auto painter = QPainter(&changedMemoryPixmap);
                painter.setFont(font);
                painter.setPen(standardFontColor);
                painter.drawText(byteItemRect, Qt::AlignCenter, hexValue);

                HexViewerItemRenderer::changedMemoryPixmapsByValue.emplace_back(std::move(changedMemoryPixmap));
            }

            {
                auto hoveredPrimaryPixmap = hoveredPrimaryTemplatePixmap;
                auto painter = QPainter(&hoveredPrimaryPixmap);
                painter.setFont(font);
                painter.setPen(standardFontColor);
                painter.drawText(byteItemRect, Qt::AlignCenter, hexValue);

                HexViewerItemRenderer::hoveredPrimaryPixmapsByValue.emplace_back(std::move(hoveredPrimaryPixmap));
            }

            {
                auto standardAsciiPixmap = standardTemplatePixmap;
                auto painter = QPainter(&standardAsciiPixmap);
                painter.setFont(font);
                painter.setPen(asciiValue.has_value() ? asciiFontColor : fadedFontColor);
                painter.drawText(byteItemRect, Qt::AlignCenter, asciiValue.value_or(hexValue));

                HexViewerItemRenderer::standardAsciiPixmapsByValue.emplace_back(std::move(standardAsciiPixmap));
            }

            {
                auto selectedAsciiPixmap = selectedTemplatePixmap;
                auto painter = QPainter(&selectedAsciiPixmap);
                painter.setFont(font);
                painter.setPen(asciiValue.has_value() ? asciiFontColor : fadedFontColor);
                painter.drawText(byteItemRect, Qt::AlignCenter, asciiValue.value_or(hexValue));

                HexViewerItemRenderer::selectedAsciiPixmapsByValue.emplace_back(std::move(selectedAsciiPixmap));
            }

            {
                auto primaryHighlightedAsciiPixmap = primaryHighlightedTemplatePixmap;
                auto painter = QPainter(&primaryHighlightedAsciiPixmap);
                painter.setFont(font);
                painter.setPen(asciiValue.has_value() ? asciiFontColor : fadedFontColor);
                painter.drawText(byteItemRect, Qt::AlignCenter, asciiValue.value_or(hexValue));

                HexViewerItemRenderer::primaryHighlightedAsciiPixmapsByValue.emplace_back(
                    std::move(primaryHighlightedAsciiPixmap)
                );
            }

            {
                auto groupedAsciiPixmap = groupedTemplatePixmap;
                auto painter = QPainter(&groupedAsciiPixmap);
                painter.setFont(font);
                painter.setPen(asciiValue.has_value() ? asciiFontColor : fadedFontColor);
                painter.drawText(byteItemRect, Qt::AlignCenter, asciiValue.value_or(hexValue));

                HexViewerItemRenderer::groupedAsciiPixmapsByValue.emplace_back(std::move(groupedAsciiPixmap));
            }

            {
                auto stackMemoryAsciiPixmap = stackMemoryTemplatePixmap;
                auto painter = QPainter(&stackMemoryAsciiPixmap);
                painter.setFont(font);
                painter.setPen(asciiValue.has_value() ? asciiFontColor : fadedFontColor);
                painter.drawText(byteItemRect, Qt::AlignCenter, asciiValue.value_or(hexValue));

                HexViewerItemRenderer::stackMemoryAsciiPixmapsByValue.emplace_back(std::move(stackMemoryAsciiPixmap));
            }

            {
                auto changedMemoryAsciiPixmap = asciiValue.has_value()
                    ? changedMemoryTemplatePixmap
                    : changedMemoryFadedTemplatePixmap;

                auto painter = QPainter(&changedMemoryAsciiPixmap);
                painter.setFont(font);
                painter.setPen(asciiValue.has_value() ? changedMemoryAsciiFontColor : fadedFontColor);
                painter.drawText(byteItemRect, Qt::AlignCenter, asciiValue.value_or(hexValue));

                HexViewerItemRenderer::changedMemoryAsciiPixmapsByValue.emplace_back(std::move(changedMemoryAsciiPixmap));
            }

            {
                auto hoveredPrimaryAsciiPixmap = hoveredPrimaryTemplatePixmap;
                auto painter = QPainter(&hoveredPrimaryAsciiPixmap);
                painter.setFont(font);
                painter.setPen(asciiValue.has_value() ? asciiFontColor : fadedFontColor);
                painter.drawText(byteItemRect, Qt::AlignCenter, asciiValue.value_or(hexValue));

                HexViewerItemRenderer::hoveredPrimaryAsciiPixmapsByValue.emplace_back(std::move(hoveredPrimaryAsciiPixmap));
            }
        }

        {
            HexViewerItemRenderer::missingDataPixmap = standardTemplatePixmap;
            auto painter = QPainter(&HexViewerItemRenderer::missingDataPixmap.value());
            painter.setFont(font);
            painter.setPen(standardFontColor);
            painter.drawText(byteItemRect, Qt::AlignCenter, "??");
        }

        {
            HexViewerItemRenderer::selectedMissingDataPixmap = selectedTemplatePixmap;
            auto painter = QPainter(&HexViewerItemRenderer::selectedMissingDataPixmap.value());
            painter.setFont(font);
            painter.setPen(standardFontColor);
            painter.drawText(byteItemRect, Qt::AlignCenter, "??");
        }

        {
            HexViewerItemRenderer::primaryHighlightedMissingDataPixmap = primaryHighlightedTemplatePixmap;
            auto painter = QPainter(&HexViewerItemRenderer::primaryHighlightedMissingDataPixmap.value());
            painter.setFont(font);
            painter.setPen(standardFontColor);
            painter.drawText(byteItemRect, Qt::AlignCenter, "??");
        }

        HexViewerItemRenderer::pixmapCachesGenerated = true;
    }
}
