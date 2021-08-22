#pragma once

#include <QFrame>
#include <QEvent>
#include <QMouseEvent>

#include "src/Logger/Logger.hpp"

namespace Bloom::Widgets
{
    class Q_WIDGETS_EXPORT ClickableWidget: public QFrame
    {
    Q_OBJECT
    protected:
        void mouseReleaseEvent(QMouseEvent* event) override;
        void mouseDoubleClickEvent(QMouseEvent* event) override;

    public:
        explicit ClickableWidget(QWidget* parent): QFrame(parent) {};

    signals:
        void clicked();
        void doubleClicked();

    };
}
