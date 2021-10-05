#pragma once

#include <QFrame>
#include <QSize>
#include <QEvent>
#include <QMouseEvent>
#include <QEnterEvent>

namespace Bloom::Widgets
{
    Q_NAMESPACE

    enum class PanelWidgetType: int
    {
        LEFT,
        BOTTOM,
    };
    Q_ENUM_NS(PanelWidgetType)

    class Q_WIDGETS_EXPORT PanelWidget: public QFrame
    {
    Q_OBJECT
        Q_PROPERTY(int handleSize READ getHandleSize WRITE setHandleSize DESIGNABLE true)
        Q_PROPERTY(int minimumResize READ getMinimumResize WRITE setMinimumResize DESIGNABLE true)
        Q_PROPERTY(Bloom::Widgets::PanelWidgetType panelType READ getPanelType WRITE setPanelType DESIGNABLE true)

    protected:
        int handleSize = 10;
        int minimumResize = 10;
        int maximumResize = 500;

        PanelWidgetType panelType = PanelWidgetType::LEFT;
        QCursor resizeCursor = Qt::SplitHCursor;
        bool resizingActive = false;
        int resizingOffset = 0;

        void mousePressEvent(QMouseEvent* event) override;
        void mouseReleaseEvent(QMouseEvent* event) override;
        bool event(QEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;

        std::pair<QPoint, QPoint> getHandleArea() const;
        bool isPositionWithinHandleArea(const QPoint& position) const;

    public:
        explicit PanelWidget(QWidget* parent);

        void setHandleSize(int handleSize) {
            this->handleSize = handleSize;
        }

        void setMinimumResize(int minimumResize);

        void setMaximumResize(int maximumResize);

        void setPanelType(PanelWidgetType panelType) {
            this->panelType = panelType;
            this->resizeCursor = this->panelType == PanelWidgetType::LEFT ? Qt::SplitHCursor : Qt::SplitVCursor;
        }

        int getHandleSize() {
            return this->handleSize;
        }

        int getMinimumResize() {
            return this->minimumResize;
        }

        int getMaximumResize() {
            return this->maximumResize;
        }

        PanelWidgetType getPanelType() {
            return this->panelType;
        }
    };
}
