#include "SlidingHandleWidget.hpp"

#include <QPainter>

using namespace Bloom::Widgets;

void SlidingHandleWidget::mouseMoveEvent(QMouseEvent* event) {
    emit this->horizontalSlide(event->pos().x());
}

void SlidingHandleWidget::enterEvent(QEnterEvent* event) {
    this->setCursor(Qt::SplitHCursor);
}

void SlidingHandleWidget::leaveEvent(QEvent* event) {
    this->setCursor(Qt::ArrowCursor);
}
