#include "DifferentialHexViewerItemRenderer.hpp"

#include <QScrollBar>
#include <QPointF>
#include <array>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/ByteAddressContainer.hpp"

namespace Bloom::Widgets
{
    DifferentialHexViewerItemRenderer::DifferentialHexViewerItemRenderer(
        DifferentialHexViewerWidgetType differentialHexViewerWidgetType,
        const HexViewerSharedState& hexViewerState,
        const HexViewerItemIndex& itemIndex,
        const QGraphicsView* view
    )
        : HexViewerItemRenderer(
            hexViewerState,
            itemIndex,
            view
        )
        , differentialHexViewerWidgetType(differentialHexViewerWidgetType)
    {}

    void DifferentialHexViewerItemRenderer::setOther(const DifferentialHexViewerItemRenderer* other) {
        this->other = other;
    }

    void DifferentialHexViewerItemRenderer::paint(
        QPainter* painter,
        const QStyleOptionGraphicsItem* option,
        QWidget* widget
    ) {
        if (this->other == nullptr) {
            return;
        }

        const auto vScrollBarValue = this->view->verticalScrollBar()->value();
        const auto otherVScrollBarValue = this->other->view->verticalScrollBar()->value();
        const auto viewportHeight = this->view->viewport()->height();

        const auto visibleItems = this->itemIndex.items(
            vScrollBarValue,
            vScrollBarValue + viewportHeight + (ByteItem::HEIGHT * 2)
        );

        painter->setRenderHints(QPainter::RenderHint::Antialiasing, false);

        // Paint the ancestors of the first visible item
        const auto& firstItem = *(visibleItems.begin());

        auto* parentItem = firstItem->parent;
        while (parentItem != nullptr) {
            painter->setOpacity(1);
            this->paintItem(parentItem, painter);
            parentItem = parentItem->parent;
        }

        const ByteItem* previousLineFirstByteItem = nullptr;
        const ByteItem* previousLineLastByteItem = nullptr;
        auto changedByteOnCurrentLine = false;

        const auto& byteItemYPosByAddress = this->itemIndex.byteItemYStartPositionsByAddress;
        painter->setOpacity(1);

        for (auto& item : visibleItems) {
            const auto* byteItem = dynamic_cast<const ByteItem*>(item);

            if (byteItem != nullptr) {
                if (
                    previousLineLastByteItem == nullptr
                    || previousLineLastByteItem->position().y() != byteItemYPosByAddress.at(byteItem->startAddress)
                ) {
                    if (changedByteOnCurrentLine) {
                        this->paintChangedLinePolygon(
                            previousLineFirstByteItem,
                            previousLineLastByteItem,
                            viewportHeight,
                            vScrollBarValue,
                            otherVScrollBarValue,
                            painter
                        );

                        changedByteOnCurrentLine = false;
                    }

                    previousLineFirstByteItem = byteItem;
                }

                changedByteOnCurrentLine = changedByteOnCurrentLine || byteItem->changed;
                previousLineLastByteItem = byteItem;
            }
        }

        if (changedByteOnCurrentLine) {
            this->paintChangedLinePolygon(
                previousLineFirstByteItem,
                previousLineLastByteItem,
                viewportHeight,
                vScrollBarValue,
                otherVScrollBarValue,
                painter
            );
        }

        for (auto& item : visibleItems) {
            painter->setOpacity(1);
            this->paintItem(item, painter);
        }
    }

    void DifferentialHexViewerItemRenderer::paintChangedLinePolygon(
        const ByteItem* firstByteItem,
        const ByteItem* lastByteItem,
        int viewportHeight,
        int vScrollBarValue,
        int otherVScrollBarValue,
        QPainter* painter
    ) {
        static constexpr auto backgroundColor = QColor(0x3A, 0x37, 0x39, 255);

        painter->setBrush(backgroundColor);
        painter->setPen(Qt::NoPen);

        const auto firstByteItemYPos = this->itemIndex.byteItemYStartPositionsByAddress.at(firstByteItem->startAddress);

        if (this->differentialHexViewerWidgetType == DifferentialHexViewerWidgetType::SECONDARY) {
            painter->drawRect(
                ByteAddressContainer::WIDTH,
                firstByteItemYPos - (ByteItem::BOTTOM_MARGIN / 2) + 1,
                this->size.width(),
                ByteItem::HEIGHT + ByteItem::BOTTOM_MARGIN - 1
            );

            return;
        }

        static constexpr auto rightSpacing = 20;
        const auto vScrollDifference = otherVScrollBarValue - vScrollBarValue;

        const auto& otherByteItemYPosByAddress = this->other->itemIndex.byteItemYStartPositionsByAddress;

        const auto otherFirstByteItemYPos = otherByteItemYPosByAddress.at(
            firstByteItem->startAddress
        ) - vScrollDifference;
        const auto otherLastByteItemYPos = otherByteItemYPosByAddress.at(
            lastByteItem->startAddress
        ) - vScrollDifference;

        const auto otherYStart = std::max(
            vScrollBarValue,
            std::min(
                otherFirstByteItemYPos - (ByteItem::BOTTOM_MARGIN / 2) + 1,
                vScrollBarValue + viewportHeight
            )
        );
        const auto otherYEnd = std::max(
            vScrollBarValue,
            std::min(
                otherLastByteItemYPos + ByteItem::HEIGHT + (ByteItem::BOTTOM_MARGIN / 2),
                vScrollBarValue + viewportHeight
            )
        );

        const auto points = std::array{
            QPointF(ByteAddressContainer::WIDTH, firstByteItemYPos - (ByteItem::BOTTOM_MARGIN / 2) + 1),
            QPointF(this->size.width() - rightSpacing, firstByteItemYPos - (ByteItem::BOTTOM_MARGIN / 2) + 1),
            QPointF(this->size.width(), otherYStart),
            QPointF(this->size.width(), otherYEnd),
            QPointF(this->size.width() - rightSpacing, firstByteItemYPos + ByteItem::HEIGHT + (ByteItem::BOTTOM_MARGIN / 2)),
            QPointF(ByteAddressContainer::WIDTH, firstByteItemYPos + ByteItem::HEIGHT + (ByteItem::BOTTOM_MARGIN / 2)),
        };

        painter->drawPolygon(points.data(), points.size());
    }
}
