#pragma once

#include <QWidget>
#include <QMouseEvent>

#include "src/Targets/TargetPinDescriptor.hpp"

namespace Bloom::InsightTargetWidgets::Dip
{
    using Targets::TargetPinDescriptor;
    using Targets::TargetPinType;
    using Targets::TargetPinState;

    class PinBodyWidget: public QWidget
    {
    Q_OBJECT
        Q_PROPERTY(QColor bodyColor READ getBodyColor WRITE setBodyColor DESIGNABLE true)
        Q_PROPERTY(int disableAlphaLevel READ getDisableAlphaLevel WRITE setDisableAlphaLevel DESIGNABLE true)

    private:
        TargetPinDescriptor pinDescriptor;
        std::optional<TargetPinState> pinState;
        QColor bodyColor = QColor("#AFB1B3");
        int disableAlphaLevel = 100;

    protected:
        bool hoverActive = false;
        void paintEvent(QPaintEvent* event) override;
        void drawWidget(QPainter& painter);
        bool event(QEvent* event) override;

        void mouseReleaseEvent(QMouseEvent* event) override {
            if (event->button() == Qt::MouseButton::LeftButton) {
                emit this->clicked();
            }
            QWidget::mouseReleaseEvent(event);
        }

    public:
        static const int WIDTH = 30;
        static const int HEIGHT = 40;

        PinBodyWidget(QWidget* parent, const TargetPinDescriptor& pinDescriptor): QWidget(parent), pinDescriptor(pinDescriptor) {
            this->setFixedSize(PinBodyWidget::WIDTH, PinBodyWidget::HEIGHT);
            this->setObjectName("target-pin-body");
        }

        void setPinState(const TargetPinState& pinState) {
            this->pinState = pinState;
        }

        QColor getBodyColor() const {
            return this->bodyColor;
        }

        void setBodyColor(QColor color) {
            this->bodyColor = color;
        }

        int getDisableAlphaLevel() const {
            return this->disableAlphaLevel;
        }

        void setDisableAlphaLevel(int level) {
            this->disableAlphaLevel = level;
        }

    signals:
        void clicked();
    };
}
