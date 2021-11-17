#include "PanelWidget.hpp"

#include <QLayout>

using namespace Bloom::Widgets;

PanelWidget::PanelWidget(QWidget* parent): QFrame(parent) {
    this->setMouseTracking(false);
    this->setAttribute(Qt::WA_Hover, true);
}

void PanelWidget::setMinimumResize(int minimumResize) {
    this->minimumResize = minimumResize;

    const auto currentSize = this->size();

    if (this->panelType == PanelWidgetType::LEFT && currentSize.width() < this->minimumResize) {
        this->setFixedWidth(this->minimumResize);

    } else if (this->panelType == PanelWidgetType::BOTTOM && currentSize.height() < this->minimumResize) {
        this->setFixedHeight(this->minimumResize);
    }
}

void PanelWidget::setMaximumResize(int maximumResize) {
    this->maximumResize = maximumResize;

    const auto currentSize = this->size();

    if (this->panelType == PanelWidgetType::LEFT && currentSize.width() > this->maximumResize) {
        this->setFixedWidth(this->maximumResize);

    } else if (this->panelType == PanelWidgetType::BOTTOM && currentSize.height() > this->maximumResize) {
        this->setFixedHeight(this->maximumResize);
    }
}

void PanelWidget::setPanelType(PanelWidgetType panelType) {
    this->panelType = panelType;

    if (this->panelType == PanelWidgetType::LEFT) {
        this->resizeCursor = Qt::SplitHCursor;
        this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);

    } else {
        this->resizeCursor = Qt::SplitVCursor;
        this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    }
}

bool PanelWidget::event(QEvent* event) {
    if (event->type() == QEvent::Type::HoverMove) {
        auto hoverEvent = static_cast<QHoverEvent*>(event);
        if (this->resizingActive || this->isPositionWithinHandleArea(hoverEvent->position().toPoint())) {
            this->setCursor(this->resizeCursor);

        } else {
            this->setCursor(Qt::ArrowCursor);
        }
    }

    return QFrame::event(event);
}

void PanelWidget::mousePressEvent(QMouseEvent* event) {
    const auto position = event->pos();

    if (event->buttons() & Qt::LeftButton && this->isPositionWithinHandleArea(position)) {
        this->resizingActive = true;

        switch (this->panelType) {
            case PanelWidgetType::LEFT: {
                this->resizingOffset = this->width() - position.x();
                break;
            }
            case PanelWidgetType::BOTTOM: {
                this->resizingOffset = position.y();
                break;
            }
        }
    }
}

void PanelWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (this->resizingActive) {
        this->resizingActive = false;
        this->resizingOffset = 0;
        this->setCursor(Qt::ArrowCursor);
    }
}

void PanelWidget::mouseMoveEvent(QMouseEvent* event) {
    if (this->resizingActive) {
        const auto position = event->pos();

        if (this->panelType == PanelWidgetType::LEFT) {
            this->setFixedWidth(
                std::max(
                    this->minimumResize,
                    std::min(this->maximumResize, position.x() + this->resizingOffset)
                )
            );

        } else if (this->panelType == PanelWidgetType::BOTTOM) {
            this->setFixedHeight(
                std::max(
                    this->minimumResize,
                    std::min(this->maximumResize, this->height() + (-position.y()) + this->resizingOffset)
                )
            );
        }
    }
}

std::pair<QPoint, QPoint> PanelWidget::getHandleArea() const {
    const auto currentSize = this->size();

    if (this->panelType == PanelWidgetType::LEFT) {
        return std::pair(
            QPoint(currentSize.width() - this->handleSize, 0),
            QPoint(currentSize.width(), currentSize.height())
        );

    } else {
        return std::pair(
            QPoint(0, 0),
            QPoint(currentSize.width(), this->handleSize)
        );
    }
}

bool PanelWidget::isPositionWithinHandleArea(const QPoint& position) const {
    const auto handleArea = this->getHandleArea();

    return (
        position.x() >= handleArea.first.x() && position.x() <= handleArea.second.x()
        && position.y() >= handleArea.first.y() && position.y() <= handleArea.second.y()
    );
}
