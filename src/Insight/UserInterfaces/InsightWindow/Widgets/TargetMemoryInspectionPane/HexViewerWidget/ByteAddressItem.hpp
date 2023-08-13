#pragma once

#include <QGraphicsItem>
#include <QPainter>

#include "ByteItem.hpp"
#include "HexViewerSharedState.hpp"

namespace Widgets
{
    class ByteAddressItem: public QGraphicsItem
    {
    public:
        static constexpr int WIDTH = 75;
        static constexpr int HEIGHT = ByteItem::HEIGHT;

        Targets::TargetMemoryAddress address = 0;

        explicit ByteAddressItem(const HexViewerSharedState& hexViewerState, QGraphicsItem* parent);

        [[nodiscard]] QRectF boundingRect() const override {
            return {
                0,
                0,
                ByteAddressItem::WIDTH,
                ByteAddressItem::HEIGHT
            };
        }

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    private:
        const HexViewerSharedState& hexViewerState;
    };
}
