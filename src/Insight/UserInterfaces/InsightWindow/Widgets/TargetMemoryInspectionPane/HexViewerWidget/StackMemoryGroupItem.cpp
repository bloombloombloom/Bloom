#include "StackMemoryGroupItem.hpp"

#include <cassert>
#include <QColor>

namespace Bloom::Widgets
{
    StackMemoryGroupItem::StackMemoryGroupItem(
        Targets::TargetStackPointer stackPointer,
        const HexViewerSharedState& hexViewerState,
        const std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
        std::unordered_map<Targets::TargetMemoryAddress, ByteItem>& byteItemsByAddress,
        HexViewerItem* parent
    )
        : GroupItem(stackPointer + 1, parent)
        , stackPointer(stackPointer)
        , hexViewerState(hexViewerState)
    {
        const auto startAddress = this->startAddress;
        const auto endAddress = this->hexViewerState.memoryDescriptor.addressRange.endAddress;

        // Sanity check
        assert(byteItemsByAddress.contains(startAddress) && byteItemsByAddress.contains(endAddress));

        for (const auto& focusedRegion : focusedMemoryRegions) {
            if (
                focusedRegion.addressRange.startAddress < startAddress
                || focusedRegion.addressRange.endAddress > endAddress
            ) {
                /*
                 * This focused region is outside of stack memory, so we don't need to handle it here.
                 *
                 * This will also catch the regions that intersect with stack memory but are not completely contained
                 * within the stack. There's no clear way to present these regions, so we ignore them, for now.
                 */
                continue;
            }

            this->focusedRegionGroupItems.emplace_back(focusedRegion, byteItemsByAddress, this);
            items.emplace_back(&(this->focusedRegionGroupItems.back()));
        }

        for (auto address = startAddress; address <= endAddress; ++address) {
            auto& byteItem = byteItemsByAddress.at(address);

            byteItem.stackMemory = true;

            if (byteItem.parent == nullptr || byteItem.parent == this->parent) {
                byteItem.parent = this;
                this->items.push_back(&byteItem);
            }
        }

        this->sortItems();
    }

    StackMemoryGroupItem::~StackMemoryGroupItem() {
        const auto updateChildItems = [] (const decltype(this->items)& items, const auto& updateChildItems) -> void {
            for (auto& item : items) {
                auto* byteItem = dynamic_cast<ByteItem*>(item);

                if (byteItem != nullptr) {
                    byteItem->stackMemory = false;
                    continue;
                }

                auto* groupItem = dynamic_cast<GroupItem*>(item);

                if (groupItem != nullptr) {
                    updateChildItems(groupItem->items, updateChildItems);
                }
            }
        };

        updateChildItems(this->items, updateChildItems);
    }

    void StackMemoryGroupItem::adjustItemPositions(
        const int maximumWidth,
        const HexViewerSharedState* hexViewerState
    ) {
        GroupItem::adjustItemPositions(maximumWidth, hexViewerState);
        this->groupSize.setWidth(maximumWidth);
    }

    void StackMemoryGroupItem::refreshValues() {
        for (auto& focusedRegionItem : this->focusedRegionGroupItems) {
            focusedRegionItem.refreshValue(this->hexViewerState);
        }
    }

    void StackMemoryGroupItem::paint(
        QPainter* painter,
        const HexViewerSharedState* hexViewerState,
        const QGraphicsItem* graphicsItem
    ) const {
        painter->setRenderHints(QPainter::RenderHint::Antialiasing, false);

        if (!graphicsItem->isEnabled()) {
            painter->setOpacity(0.5);
        }

        const auto headingText = QString("Stack Memory");

        const auto stackSize = hexViewerState->memoryDescriptor.addressRange.endAddress - this->startAddress + 1;
        const auto& memoryCapacity = hexViewerState->memoryDescriptor.size();

        const auto stackSizeHeadingText = QString("Stack size:");
        const auto stackSizeValueText = QString::number(stackSize) + " byte(s) ("
            + QString::number(static_cast<float>(stackSize) / static_cast<float>(memoryCapacity / 100), 'f' , 1)
            + "% of memory capacity)";

        const auto stackPointerHeadingText = QString("Stack pointer:");
        const auto stackPointerValueText = "0x" + QString::number(
            this->stackPointer,
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

        const auto groupWidth = this->groupSize.width();
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

        const auto verticalLineYStart = static_cast<int>(heightOffset - 5);
        const auto verticalLineYEnd = static_cast<int>(heightOffset);

        const auto lineStartX = ByteItem::WIDTH / 2;
        const auto lineEndX = groupWidth - + (ByteItem::WIDTH / 2);

        const auto labelRect = QRect(
            (groupWidth - headingLabelSize.width()) / 2,
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
            groupWidth / 2,
            verticalLineYStart - labelLineHeight,
            groupWidth / 2,
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

    QMargins StackMemoryGroupItem::groupMargins(
        const HexViewerSharedState* hexViewerState,
        const int maximumWidth
    ) const {
        return QMargins(0, 100, 0, 20);
    }
}
