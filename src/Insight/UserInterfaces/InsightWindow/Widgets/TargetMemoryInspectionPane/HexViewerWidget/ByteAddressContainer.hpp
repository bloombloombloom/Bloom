#pragma once

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QPainter>
#include <vector>

#include "HexViewerSharedState.hpp"
#include "ByteAddressItem.hpp"

namespace Bloom::Widgets
{
    class ByteAddressContainer: public QGraphicsItem
    {
    public:
        static constexpr int WIDTH = 88;

        ByteAddressContainer(const HexViewerSharedState& hexViewerState);

        [[nodiscard]] QRectF boundingRect() const override {
            return QRectF(
                0,
                0,
                ByteAddressContainer::WIDTH,
                this->scene()->height()
            );
        }

        void adjustAddressLabels(const std::vector<const ByteItem*>& firstByteItemByLine);
        void invalidateChildItemCaches();
        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    private:
        const HexViewerSharedState& hexViewerState;
        std::vector<ByteAddressItem*> addressItems;
    };
}
