#pragma once

#include <QGraphicsItem>
#include <QSize>
#include <QRectF>
#include <QPainter>
#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <set>

#include "src/Helpers/DereferenceLessComparator.hpp"

namespace Widgets
{
    class ListItem: public QGraphicsItem
    {
    public:
        using ListItemSetType = std::set<ListItem*, DereferenceLessComparator<ListItem*>>;

        bool selected = false;
        bool excluded = false;
        QSize size = {};

        ListItem() = default;

        explicit ListItem(QGraphicsItem* parent)
            : QGraphicsItem(parent)
        {}

        [[nodiscard]] QRectF boundingRect() const override {
            return QRectF{QPointF{0, 0}, this->size};
        }

        virtual void onGeometryChanged() {}

        virtual bool operator < (const ListItem& rhs) const {
            return true;
        }

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
