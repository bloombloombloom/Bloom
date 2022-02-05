#include "ClickableWidget.hpp"

namespace Bloom::Widgets
{
    void ClickableWidget::mouseReleaseEvent(QMouseEvent* event) {
        if (event->button() == Qt::MouseButton::LeftButton) {
            emit this->clicked();

        } else if (event->button() == Qt::MouseButton::RightButton) {
            emit this->rightClicked();
        }

        QWidget::mouseReleaseEvent(event);
    }

    void ClickableWidget::mouseDoubleClickEvent(QMouseEvent* event) {
        if (event->button() == Qt::MouseButton::LeftButton) {
            emit this->doubleClicked();
        }

        QWidget::mouseDoubleClickEvent(event);
    }
}
