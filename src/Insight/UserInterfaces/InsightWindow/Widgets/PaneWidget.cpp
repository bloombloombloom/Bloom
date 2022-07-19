#include "PaneWidget.hpp"

namespace Bloom::Widgets
{
    PaneWidget::PaneWidget(PanelWidget* parent)
        : QWidget(parent)
        , parentPanel(parent)
    {
        this->setMouseTracking(false);
        this->setAttribute(Qt::WA_Hover, true);

        QObject::connect(this, &PaneWidget::paneActivated, parent, &PanelWidget::updateVisibility);
        QObject::connect(this, &PaneWidget::paneDeactivated, parent, &PanelWidget::updateVisibility);

        QObject::connect(this, &PaneWidget::paneAttached, parent, &PanelWidget::updateVisibility);
        QObject::connect(this, &PaneWidget::paneDetached, parent, &PanelWidget::updateVisibility);
    }

    PaneState PaneWidget::getCurrentState() const {
        return PaneState(
            this->activated,
            this->attached,
            this->getDetachedWindowState()
        );
    }

    void PaneWidget::activate() {
        if (this->activated) {
            return;
        }

        this->show();
        this->activated = true;
        emit this->paneActivated();
    }

    void PaneWidget::deactivate() {
        if (!this->activated) {
            return;
        }

        this->hide();
        this->activated = false;
        emit this->paneDeactivated();
    }

    void PaneWidget::restoreLastPaneState(const PaneState& lastPaneState) {
        if (lastPaneState.detachedWindowState.has_value()) {
            this->lastDetachedWindowState = lastPaneState.detachedWindowState;
        }

        if (!lastPaneState.attached && lastPaneState.detachedWindowState.has_value()) {
            this->detach();
        }

        if (lastPaneState.activated) {
            this->activate();
        }
    }

    void PaneWidget::detach() {
        if (!this->attached) {
            return;
        }

        this->setWindowFlag(Qt::Window);

        if (this->lastDetachedWindowState.has_value()) {
            this->resize(this->lastDetachedWindowState->size);
            this->move(this->lastDetachedWindowState->position);
        }

        if (this->activated) {
            this->show();
        }

        this->attached = false;
        emit this->paneDetached();
    }

    void PaneWidget::attach() {
        if (this->attached) {
            return;
        }

        this->lastDetachedWindowState = this->getDetachedWindowState();
        this->setWindowFlag(Qt::Window, false);

        if (this->activated) {
            this->show();
        }

        this->attached = true;
        emit this->paneAttached();
    }

    void PaneWidget::closeEvent(QCloseEvent* event) {
        this->deactivate();
        QWidget::closeEvent(event);
    }

    std::optional<DetachedWindowState> PaneWidget::getDetachedWindowState() const {
        if (!this->attached) {
            return DetachedWindowState(this->size(), this->pos());
        }

        return this->lastDetachedWindowState;
    }
}
