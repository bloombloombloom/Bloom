#pragma once

#include <QGraphicsItem>

namespace Widgets::PinoutWidgets
{
    class PinoutItem: public QGraphicsItem
    {
    public:
        virtual void refreshGeometry() = 0;
    };
}
