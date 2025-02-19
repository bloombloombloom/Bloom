#include "PinoutContainer.hpp"

namespace Widgets::PinoutWidgets
{
    PinoutContainer::PinoutContainer(const Targets::TargetDescriptor& targetDescriptor, QWidget* parent)
        : QGraphicsView(parent)
        , pinoutScene(new PinoutScene{targetDescriptor, this})
        , targetDescriptor(targetDescriptor)
    {
        this->setObjectName("pinout-widget-container");
        this->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
        this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        this->setViewportUpdateMode(QGraphicsView::ViewportUpdateMode::MinimalViewportUpdate);
        this->setFrameShape(QFrame::NoFrame);
        this->setDragMode(QGraphicsView::DragMode::NoDrag);
        this->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        this->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        this->setMouseTracking(true);

        this->setScene(this->pinoutScene);
        this->setMinimumWidth(1);
    }

    void PinoutContainer::resizeEvent(QResizeEvent* event) {
        QGraphicsView::resizeEvent(event);
    }

    void PinoutContainer::keyPressEvent(QKeyEvent* event) {
        const auto viewSize = this->size();
        const auto sceneRect = this->pinoutScene->sceneRect();

        const auto key = event->key();
        if (key == Qt::Key_Control) {
            this->setDragMode(
                sceneRect.width() > viewSize.width() || sceneRect.height() > viewSize.height()
                    ? QGraphicsView::DragMode::ScrollHandDrag
                    : QGraphicsView::DragMode::NoDrag
            );
            return;
        }

        QGraphicsView::keyPressEvent(event);
    }

    void PinoutContainer::keyReleaseEvent(QKeyEvent* event) {
        const auto key = event->key();
        if (key == Qt::Key_Control) {
            this->setDragMode(QGraphicsView::DragMode::NoDrag);
        }

        QGraphicsView::keyReleaseEvent(event);
    }

    void PinoutContainer::mousePressEvent(QMouseEvent* event) {
        if (this->dragMode() == QGraphicsView::DragMode::ScrollHandDrag) {
            return QGraphicsView::mousePressEvent(event);
        }

        const auto modifiers = event->modifiers();
        if ((modifiers & Qt::KeyboardModifier::ControlModifier) != 0) {
            const auto viewSize = this->size();
            const auto sceneRect = this->pinoutScene->sceneRect();
            this->setDragMode(
                sceneRect.width() > viewSize.width() || sceneRect.height() > viewSize.height()
                    ? QGraphicsView::DragMode::ScrollHandDrag
                    : QGraphicsView::DragMode::NoDrag
            );
        }

        return QGraphicsView::mousePressEvent(event);
    }
}
