#pragma once

#include <QWidget>
#include <optional>

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

        [[nodiscard]] PaneState getCurrentState() const;

        void activate();
        void deactivate();

        void restoreLastPaneState(const PaneState& lastPaneState);

    signals:
        void paneActivated();
        void paneDeactivated();
        void paneAttached();
        void paneDetached();

    protected:
        void detach();
        void attach();

        void closeEvent(QCloseEvent* event) override;

    private:
        std::optional<DetachedWindowState> lastDetachedWindowState;

        std::optional<DetachedWindowState> getDetachedWindowState() const;
    };
}
