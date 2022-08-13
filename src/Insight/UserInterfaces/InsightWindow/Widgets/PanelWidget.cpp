#include "PanelWidget.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>

#include "PaneWidget.hpp"

namespace Bloom::Widgets
{
    PanelWidget::PanelWidget(PanelWidgetType type, PanelState& state, QWidget* parent)
        : panelType(type)
        , state(state)
        , QFrame(parent)
    {
        this->setMouseTracking(false);
        this->setAttribute(Qt::WA_Hover, true);

        if (this->panelType == PanelWidgetType::LEFT) {
            this->resizeCursor = Qt::SplitHCursor;
            this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);

            auto* layout = new QVBoxLayout(this);
            layout->setSpacing(0);
            layout->setContentsMargins(0, 0, 0, 0);
            this->setLayout(layout);

        } else {
            this->resizeCursor = Qt::SplitVCursor;
            this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

            auto* layout = new QHBoxLayout(this);
            layout->setSpacing(0);
            layout->setContentsMargins(0, 0, 0, 0);
            this->setLayout(layout);
        }
    }

    void PanelWidget::setMinimumResize(int minimumResize) {
        this->minimumResize = minimumResize;

        if (this->state.size < this->minimumResize) {
            this->setSize(this->minimumResize);
        }
    }

    void PanelWidget::setMaximumResize(int maximumResize) {
        this->maximumResize = maximumResize;

        if (this->state.size > this->maximumResize) {
            this->setSize(this->maximumResize);
        }
    }

    void PanelWidget::setSize(int size) {
        if (this->panelType == PanelWidgetType::LEFT) {
            const auto width = std::min(std::max(size, this->minimumResize), this->maximumResize);
            this->setFixedWidth(width);
            this->state.size = width;

        } else if (this->panelType == PanelWidgetType::BOTTOM) {
            const auto height = std::min(std::max(size, this->minimumResize), this->maximumResize);
            this->setFixedHeight(height);
            this->state.size = height;
        }
    }

    void PanelWidget::updateVisibility() {
        const auto paneWidgets = this->findChildren<PaneWidget*>();

        auto visible = false;
        for (const auto& paneWidget : paneWidgets) {
            if (paneWidget->state.activated && paneWidget->state.attached) {
                visible = true;
                break;
            }
        }

        this->setVisible(visible);
    }

    bool PanelWidget::event(QEvent* event) {
        if (event->type() == QEvent::Type::HoverMove) {
            auto* hoverEvent = dynamic_cast<QHoverEvent*>(event);
            if (this->resizingActive || this->isPositionWithinHandleArea(hoverEvent->position().toPoint())) {
                this->setCursor(this->resizeCursor);

            } else {
                this->setCursor(Qt::ArrowCursor);
            }
        }

        if (event->type() == QEvent::Type::Close || event->type() == QEvent::Type::Hide) {
            this->state.open = false;
            emit this->closed();

        } else if (event->type() == QEvent::Type::Show) {
            this->state.open = true;
            emit this->opened();
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
            this->setSize(this->panelType == PanelWidgetType::LEFT
                ? event->pos().x() + this->resizingOffset
                : this->height() + (-event->pos().y()) + this->resizingOffset
            );
        }
    }

    std::pair<QPoint, QPoint> PanelWidget::getHandleArea() const {
        const auto currentSize = this->size();

        if (this->panelType == PanelWidgetType::LEFT) {
            return std::pair(
                QPoint(currentSize.width() - this->handleSize, 0),
                QPoint(currentSize.width(), currentSize.height())
            );
        }

        return std::pair(
            QPoint(0, 0),
            QPoint(currentSize.width(), this->handleSize)
        );
    }

    bool PanelWidget::isPositionWithinHandleArea(const QPoint& position) const {
        const auto handleArea = this->getHandleArea();

        return (
            position.x() >= handleArea.first.x() && position.x() <= handleArea.second.x()
            && position.y() >= handleArea.first.y() && position.y() <= handleArea.second.y()
        );
    }
}
