#pragma once

#include <QGraphicsItem>
#include <cstdint>
#include <QEvent>
#include <QGraphicsScene>
#include <QFont>
#include <QColor>
#include <optional>
#include <map>

#include "ByteItem.hpp"
#include "ByteAddressItem.hpp"

namespace Bloom::Widgets
{
    class ByteAddressContainer: public QGraphicsItem
    {
    public:
        static constexpr int WIDTH = 85;

        ByteAddressContainer();

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
