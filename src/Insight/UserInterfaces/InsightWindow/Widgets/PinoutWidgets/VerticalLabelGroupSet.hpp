#pragma once

#include <QGraphicsItem>
#include <QSize>
#include <QRectF>
#include <QPainter>
#include <vector>

#include "VerticalLabelGroup.hpp"

namespace Widgets::PinoutWidgets
{
    class VerticalLabelGroupSet: public QGraphicsItem
    {
    public:
        static constexpr int GROUP_HORIZONTAL_MARGIN = 16;
        static constexpr int GROUP_VERTICAL_MARGIN = 10;

        enum Position: std::uint8_t
        {
            TOP,
            BOTTOM
        };

        std::vector<VerticalLabelGroup*> labelGroups;
        QSize size;
        const Position position;

        explicit VerticalLabelGroupSet(Position position);

        void refreshGeometry();
        [[nodiscard]] QRectF boundingRect() const override;
        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    private:
        bool isGroupOnFirstRow(const VerticalLabelGroup* group) const;
        int getFirstRowHeight() const;
    };
}
