#pragma once

#include <QGraphicsItem>
#include <QSize>
#include <QRectF>
#include <QPainter>
#include <vector>

#include "HorizontalLabelGroup.hpp"

namespace Widgets::PinoutWidgets
{
    class HorizontalLabelGroupSet: public QGraphicsItem
    {
    public:
        static constexpr int GROUP_VERTICAL_MARGIN = 5;

        enum Position: std::uint8_t
        {
            LEFT,
            RIGHT
        };

        std::vector<HorizontalLabelGroup*> labelGroups;
        QSize size;
        const Position position;

        explicit HorizontalLabelGroupSet(Position position);

        void refreshGeometry();
        [[nodiscard]] QRectF boundingRect() const override;
        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    };
}
