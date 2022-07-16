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

    void PaneWidget::detach() {
        this->setWindowFlag(Qt::Window);
        this->show();

        this->attached = false;
        emit this->paneDetached();
    }

    void PaneWidget::attach() {
        this->setWindowFlag(Qt::Window, false);
        this->hide();
        this->show();

        this->attached = true;
        emit this->paneAttached();
    }
}
