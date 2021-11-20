#pragma once

#include <QWidget>
#include <QPainter>

namespace Bloom::Widgets::InsightTargetWidgets::Dip
{
    class BodyWidget: public QWidget
    {
        Q_OBJECT
        Q_PROPERTY(QColor bodyColor READ getBodyColor WRITE setBodyColor DESIGNABLE true)
        Q_PROPERTY(int disableAlphaLevel READ getDisableAlphaLevel WRITE setDisableAlphaLevel DESIGNABLE true)

    public:
        static constexpr int HEIGHT = 130;

        explicit BodyWidget(QWidget* parent);

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
        // These properties can be modified via Qt style sheets (see Stylesheets/DualInlinePackage.qss)
        QColor bodyColor = QColor("#8E8B83");
        int disableAlphaLevel = 100;
    };
}
