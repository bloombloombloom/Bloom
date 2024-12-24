#pragma once

#include <QWidget>
#include <QPainter>

namespace Widgets::InsightTargetWidgets::Dip
{
    class BodyWidget: public QWidget
    {
        Q_OBJECT
        Q_PROPERTY(QColor bodyColor READ getBodyColor WRITE setBodyColor DESIGNABLE true)
        Q_PROPERTY(int disableAlphaLevel READ getDisableAlphaLevel WRITE setDisableAlphaLevel DESIGNABLE true)

    public:
        explicit BodyWidget(QWidget* parent, std::size_t pinCount);

        [[nodiscard]] QColor getBodyColor() const {
            return this->bodyColor;
        }

        void setBodyColor(const QColor& color) {
            this->bodyColor = color;
        }

        [[nodiscard]] int getDisableAlphaLevel() const {
            return this->disableAlphaLevel;
        }

        void setDisableAlphaLevel(int level) {
            this->disableAlphaLevel = level;
        }

    protected:
        void paintEvent(QPaintEvent* event) override;
        void drawWidget(QPainter& painter);

    private:
        static constexpr int MAXIMUM_BODY_HEIGHT = 156;
        static constexpr int MINIMUM_BODY_HEIGHT = 96;
        static constexpr int MAXIMUM_FIRST_PIN_INDICATOR_HEIGHT = 16;
        static constexpr int MINIMUM_FIRST_PIN_INDICATOR_HEIGHT = 12;

        // These properties can be modified via Qt style sheets (see Stylesheets/DualInlinePackage.qss)
        QColor bodyColor = {"#8E8B83"};
        int disableAlphaLevel = 100;
        int firstPinIndicatorDiameter = 14;
        int orientationIndicatorDiameter = 16;
    };
}
