#pragma once

#include <QWidget>

namespace Bloom::Widgets::InsightTargetWidgets::Qfp
{
    class BodyWidget: public QWidget
    {
    Q_OBJECT
    Q_PROPERTY(QColor bodyColor READ getBodyColor WRITE setBodyColor DESIGNABLE true)
    Q_PROPERTY(int disableAlphaLevel READ getDisableAlphaLevel WRITE setDisableAlphaLevel DESIGNABLE true)

    private:
        // These properties can be modified via Qt style sheets (see Stylesheets/QuadFlatPackage.qss)
        QColor bodyColor = QColor("#908D85");
        int disableAlphaLevel = 100;

    protected:
        void paintEvent(QPaintEvent* event) override;
        void drawWidget(QPainter& painter);

    public:
        explicit BodyWidget(QWidget* parent): QWidget(parent) {
            this->setObjectName("target-body");
        }

        [[nodiscard]] QColor getBodyColor() const {
            return this->bodyColor;
        }

        void setBodyColor(QColor color) {
            this->bodyColor = color;
        }

        [[nodiscard]] int getDisableAlphaLevel() const {
            return this->disableAlphaLevel;
        }

        void setDisableAlphaLevel(int level) {
            this->disableAlphaLevel = level;
        }
    };
}
