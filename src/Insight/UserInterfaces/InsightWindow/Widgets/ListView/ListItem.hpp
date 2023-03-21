#pragma once

#include <QGraphicsItem>
#include <QSize>
#include <QRectF>
#include <QPainter>
#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>

namespace Bloom::Widgets
{
    class ListItem: public QGraphicsItem
    {
    public:
        bool selected = false;
        QSize size = QSize();

        ListItem() = default;

        [[nodiscard]] QRectF boundingRect() const override {
            return QRectF(QPointF(0, 0), this->size);
        }

        virtual void onGeometryChanged() {
            return;
        }

        virtual bool operator < (const ListItem& rhs) const = 0;

        bool operator > (const ListItem& rhs) const {
            return rhs < *this;
        }

        bool operator <= (const ListItem& rhs) const {
            return !(rhs < *this);
        }

        bool operator >= (const ListItem& rhs) const {
            return !(*this < rhs);
        }

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override = 0;
    };
}
