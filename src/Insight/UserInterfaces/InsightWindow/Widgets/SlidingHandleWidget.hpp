#pragma once

#include <QFrame>
#include <QSize>
#include <QEvent>
#include <QMouseEvent>
#include <QEnterEvent>

namespace Bloom::Widgets
{
    class Q_WIDGETS_EXPORT SlidingHandleWidget: public QFrame
    {
    Q_OBJECT
        Q_PROPERTY(int handleWidth READ getHandleWidth WRITE setHandleWidth DESIGNABLE true)

    protected:
        int handleWidth = 10;

        void mouseMoveEvent(QMouseEvent* event) override;
        void enterEvent(QEnterEvent* event) override;
        void leaveEvent(QEvent* event) override;

    public:
        explicit SlidingHandleWidget(QWidget* parent): QFrame(parent) {};

        QSize minimumSizeHint() const override {
            return QSize(this->handleWidth, this->parentWidget()->height());
        };

        QSize sizeHint() const override {
            return QSize(this->handleWidth, this->parentWidget()->height());
        };

        void setHandleWidth(int handleWidth) {
            this->handleWidth = handleWidth;
            this->setFixedWidth(handleWidth);
        }

        int getHandleWidth() {
            return this->handleWidth;
        }

    signals:
        void horizontalSlide(int position);

    };
}
