#pragma once

#include <QFrame>
#include <QEvent>
#include <QMouseEvent>

namespace Bloom::Widgets
{
    class ClickableWidget: public QFrame
    {
        Q_OBJECT

    public:
        explicit ClickableWidget(QWidget* parent): QFrame(parent) {};

    signals:
        void rightClicked();
        void clicked();
        void doubleClicked();

    protected:
        void mouseReleaseEvent(QMouseEvent* event) override;
        void mouseDoubleClickEvent(QMouseEvent* event) override;
    };
}
