#pragma once

#include <QWidget>
#include <QEvent>
#include <optional>

#include "PanelWidget.hpp"
#include "PaneState.hpp"

namespace Bloom::Widgets
{
    class PaneWidget: public QWidget
    {
        Q_OBJECT

    public:
        PaneState& state;
        PanelWidget* parentPanel = nullptr;

        explicit PaneWidget(PaneState& state, PanelWidget* parent);

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

        void resizeEvent(QResizeEvent* event) override;
        void moveEvent(QMoveEvent* event) override;
        void closeEvent(QCloseEvent* event) override;
    };
}
