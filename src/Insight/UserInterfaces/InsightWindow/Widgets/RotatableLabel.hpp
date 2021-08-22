#pragma once

#include <QLabel>
#include <QSize>

namespace Bloom::Widgets
{
    class RotatableLabel: public QLabel
    {
    Q_OBJECT
    private:
        int angle = 90;

        [[nodiscard]] QSize getContainerSize() const;

    protected:
        void paintEvent(QPaintEvent* event) override;

        [[nodiscard]] QSize sizeHint() const override {
            return this->getContainerSize();
        };

        [[nodiscard]] QSize minimumSizeHint() const override {
            return this->getContainerSize();
        };

    public:
        RotatableLabel(const QString& text, QWidget* parent): QLabel(text, parent) {};
        RotatableLabel(int angleDegrees, const QString& text, QWidget* parent): QLabel(text, parent) {
            this->setAngle(angleDegrees);
        };

        void setAngle(int angleDegrees) {
            this->angle = angleDegrees;
        }
    };
}
