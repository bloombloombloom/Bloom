#pragma once

#include <QGraphicsItem>
#include <cstdint>
#include <QPainter>
#include <QGraphicsScene>
#include <map>
#include <vector>

#include "ByteItem.hpp"
#include "ByteAddressItem.hpp"

namespace Bloom::Widgets
{
    class ByteAddressContainer: public QGraphicsItem
    {
    public:
        static constexpr int WIDTH = 85;

        ByteAddressContainer() = default;

        [[nodiscard]] QRectF boundingRect() const override {
            return QRectF(
                0,
                0,
                ByteAddressContainer::WIDTH,
                this->scene()->height()
            );
        }

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
        void adjustAddressLabels(const std::map<std::size_t, std::vector<ByteItem*>>& byteItemsByRowIndex);

    private:
        std::map<std::size_t, ByteAddressItem*> addressItemsByRowIndex;
    };
}
