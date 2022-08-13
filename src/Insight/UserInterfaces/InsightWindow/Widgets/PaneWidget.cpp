#include "PaneWidget.hpp"

namespace Bloom::Widgets
{
    PaneWidget::PaneWidget(PaneState& state, PanelWidget* parent)
        : state(state)
        , parentPanel(parent)
        , QWidget(parent)
    {
        this->setMouseTracking(false);
        this->setAttribute(Qt::WA_Hover, true);

        QObject::connect(this, &PaneWidget::paneActivated, parent, &PanelWidget::updateVisibility);
        QObject::connect(this, &PaneWidget::paneDeactivated, parent, &PanelWidget::updateVisibility);

        QObject::connect(this, &PaneWidget::paneAttached, parent, &PanelWidget::updateVisibility);
        QObject::connect(this, &PaneWidget::paneDetached, parent, &PanelWidget::updateVisibility);
    }

    void PaneWidget::activate() {
        this->show();
        this->state.activated = true;
        emit this->paneActivated();
    }

    void PaneWidget::deactivate() {
        if (this->isVisible()) {
            this->hide();
        }

        this->state.activated = false;
        emit this->paneDeactivated();
    }

    void PaneWidget::detach() {
        this->setWindowFlag(Qt::Window);

        if (this->state.detachedWindowState.has_value()) {
            this->resize(this->state.detachedWindowState->size);
            this->move(this->state.detachedWindowState->position);

        } else {
            this->state.detachedWindowState = DetachedWindowState(this->size(), this->pos());
        }

        this->state.attached = false;
        emit this->paneDetached();
    }

    void PaneWidget::attach() {
        this->setWindowFlag(Qt::Window, false);

        this->state.attached = true;
        emit this->paneAttached();
    }

    void PaneWidget::resizeEvent(QResizeEvent* event) {
        if (!this->state.attached && this->state.detachedWindowState.has_value()) {
            this->state.detachedWindowState->size = this->size();
        }

        QWidget::resizeEvent(event);
    }

    void PaneWidget::moveEvent(QMoveEvent* event) {
        if (!this->state.attached && this->state.detachedWindowState.has_value()) {
            this->state.detachedWindowState->position = this->pos();
        }

        QWidget::moveEvent(event);
    }

    void PaneWidget::closeEvent(QCloseEvent* event) {
        this->deactivate();
        QWidget::closeEvent(event);
    }
}
