#include "ClickableWidget.hpp"

using namespace Bloom::Widgets;

void ClickableWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::MouseButton::LeftButton) {
        emit this->clicked();
    }

    QWidget::mouseReleaseEvent(event);
}

void ClickableWidget::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() == Qt::MouseButton::LeftButton) {
        emit this->doubleClicked();
    }

    QWidget::mouseDoubleClickEvent(event);
}
