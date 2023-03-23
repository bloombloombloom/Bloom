#pragma once

#include <QFrame>
#include <QSize>
#include <optional>
#include <QEvent>
#include <QMouseEvent>
#include <QEnterEvent>

#include "PanelState.hpp"

namespace Bloom::Widgets
{
    Q_NAMESPACE

    enum class PanelWidgetType: int
    {
        LEFT,
        RIGHT,
        BOTTOM,
    };
    Q_ENUM_NS(PanelWidgetType)

    class PanelWidget: public QFrame
    {
        Q_OBJECT
        Q_PROPERTY(int handleSize READ getHandleSize WRITE setHandleSize DESIGNABLE true)
        Q_PROPERTY(int minimumResize READ getMinimumResize WRITE setMinimumResize DESIGNABLE true)

    public:
        PanelState& state;

        explicit PanelWidget(PanelWidgetType type, PanelState& state, QWidget* parent);

        void setHandleSize(int handleSize) {
            this->handleSize = handleSize;
        }

        void setMinimumResize(int minimumResize);

        void setMaximumResize(int maximumResize);

        void setSize(int size);

        [[nodiscard]] int getHandleSize() const {
            return this->handleSize;
        }

        [[nodiscard]] int getMinimumResize() const {
            return this->minimumResize;
        }

        [[nodiscard]] int getMaximumResize() const {
            return this->maximumResize;
        }

        /**
         * Will evaluate whether the panel should still be visible or not (depending on whether there are any
         * visible and attached child panes).
         *
         * This function is called whenever a child pane is activated/deactivate/attached/detached.
         * See PaneWidget::PaneWidget() for more.
         */
        void updateVisibility();

    signals:
        void closed();
        void opened();

    protected:
        int handleSize = 10;
        int minimumResize = 10;
        int maximumResize = 500;

        PanelWidgetType panelType = PanelWidgetType::LEFT;

        QCursor resizeCursor = Qt::SplitHCursor;
        std::optional<QPoint> initialResizePoint = std::nullopt;
        int initialResizeSize = 0;

        bool eventFilter(QObject *object, QEvent *event) override;
        bool event(QEvent* event) override;
        void mousePressEvent(QMouseEvent* event) override;
        void mouseReleaseEvent(QMouseEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;

        [[nodiscard]] std::pair<QPoint, QPoint> getHandleArea() const;
        [[nodiscard]] bool isPositionWithinHandleArea(const QPoint& position) const;
    };
}
