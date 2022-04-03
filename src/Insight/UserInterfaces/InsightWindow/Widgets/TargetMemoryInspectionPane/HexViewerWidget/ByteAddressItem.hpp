#pragma once

#include <cstdint>
#include <QEvent>
#include <QGraphicsItem>
#include <optional>

#include "ByteItem.hpp"

namespace Bloom::Widgets
{
    class ByteAddressItem: public QGraphicsItem
    {
    public:
        static constexpr int WIDTH = 75;
        static constexpr int HEIGHT = ByteItem::HEIGHT;

        ByteAddressItem(QGraphicsItem* parent): QGraphicsItem(parent) {};

        void setAddressHex(const QString& address);

        [[nodiscard]] QRectF boundingRect() const override {
            return QRectF(
                0,
                0,
                ByteAddressItem::WIDTH,
                ByteAddressItem::HEIGHT
            );
        }

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    private:
        QString addressHex;
    };
}
