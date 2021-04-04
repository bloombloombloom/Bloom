#pragma once

#include <QWidget>

namespace Bloom::InsightTargetWidgets::Qfp
{
    class BodyWidget: public QWidget
    {
    Q_OBJECT
    Q_PROPERTY(QColor bodyColor READ getBodyColor WRITE setBodyColor DESIGNABLE true)
    Q_PROPERTY(int disableAlphaLevel READ getDisableAlphaLevel WRITE setDisableAlphaLevel DESIGNABLE true)

    private:
        QColor bodyColor = QColor("#AFB1B3");
        int disableAlphaLevel = 100;

    protected:
        void paintEvent(QPaintEvent* event);
        void drawWidget(QPainter& painter);

    public:
        BodyWidget(QWidget* parent): QWidget(parent) {
            this->setObjectName("target-body");
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
    };
}
