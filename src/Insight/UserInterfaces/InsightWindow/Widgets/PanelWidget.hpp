#pragma once

#include <QFrame>
#include <QSize>
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
        BOTTOM,
    };
    Q_ENUM_NS(PanelWidgetType)

    class PanelWidget: public QFrame
    {
        Q_OBJECT
        Q_PROPERTY(int handleSize READ getHandleSize WRITE setHandleSize DESIGNABLE true)
        Q_PROPERTY(int minimumResize READ getMinimumResize WRITE setMinimumResize DESIGNABLE true)
        Q_PROPERTY(Bloom::Widgets::PanelWidgetType panelType READ getPanelType WRITE setPanelType DESIGNABLE true)

    public:
        explicit PanelWidget(QWidget* parent);

        void setHandleSize(int handleSize) {
            this->handleSize = handleSize;
        }

        void setMinimumResize(int minimumResize);

        void setMaximumResize(int maximumResize);

        void setPanelType(PanelWidgetType panelType);

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

        PanelWidgetType getPanelType() {
            return this->panelType;
        }

        PanelState getCurrentState() {
            return PanelState(
                this->panelType == PanelWidgetType::LEFT ? this->width() : this->height(),
                this->isVisible()
            );
        }

    protected:
        int handleSize = 10;
        int minimumResize = 10;
        int maximumResize = 500;

        PanelWidgetType panelType = PanelWidgetType::LEFT;
        QCursor resizeCursor = Qt::SplitHCursor;
        bool resizingActive = false;
        int resizingOffset = 0;

        bool event(QEvent* event) override;
        void mousePressEvent(QMouseEvent* event) override;
        void mouseReleaseEvent(QMouseEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;

        [[nodiscard]] std::pair<QPoint, QPoint> getHandleArea() const;
        [[nodiscard]] bool isPositionWithinHandleArea(const QPoint& position) const;
    };
}
