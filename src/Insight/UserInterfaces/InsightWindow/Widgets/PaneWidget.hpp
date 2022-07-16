#pragma once

#include <QWidget>

#include "PanelWidget.hpp"
#include "PaneState.hpp"

namespace Bloom::Widgets
{
    class PaneWidget: public QWidget
    {
        Q_OBJECT

    public:
        bool activated = true;
        bool attached = true;
        PanelWidget* parentPanel = nullptr;

        explicit PaneWidget(PanelWidget* parent);

        [[nodiscard]] PaneState getCurrentState() const {
            return PaneState(
                this->activated
            );
        }

        void activate();
        void deactivate();

    signals:
        void paneActivated();
        void paneDeactivated();
        void paneAttached();
        void paneDetached();

    protected:
        void detach();
        void attach();
    };
}
