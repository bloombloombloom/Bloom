#pragma once

#include <QSize>

#include "Label.hpp"

namespace Widgets
{
    class RotatableLabel: public Label
    {
        Q_OBJECT
        Q_PROPERTY(int leftMargin READ getLeftMargin WRITE setLeftMargin DESIGNABLE true)
        Q_PROPERTY(int rightMargin READ getRightMargin WRITE setRightMargin DESIGNABLE true)
        Q_PROPERTY(int topMargin READ getTopMargin WRITE setTopMargin DESIGNABLE true)
        Q_PROPERTY(int bottomMargin READ getBottomMargin WRITE setBottomMargin DESIGNABLE true)
        Q_PROPERTY(int angle READ getAngle WRITE setAngle DESIGNABLE true)

    public:
        RotatableLabel(const QString& text, QWidget* parent)
            : Label(text, parent)
        {};

        RotatableLabel(int angleDegrees, const QString& text, QWidget* parent)
            : Label(text, parent)
            , angle(angleDegrees)
        {};

        int getLeftMargin() {
            return this->contentsMargins().left();
        }

        void setLeftMargin(int leftMargin) {
            auto margins = this->contentsMargins();
            margins.setLeft(leftMargin);
            this->setContentsMargins(margins);
        }

        int getRightMargin() {
            return this->contentsMargins().right();
        }

        void setRightMargin(int rightMargin) {
            auto margins = this->contentsMargins();
            margins.setRight(rightMargin);
            this->setContentsMargins(margins);
        }

        int getTopMargin() {
            return this->contentsMargins().top();
        }

        void setTopMargin(int topMargin) {
            auto margins = this->contentsMargins();
            margins.setTop(topMargin);
            this->setContentsMargins(margins);
        }

        int getBottomMargin() {
            return this->contentsMargins().bottom();
        }

        void setBottomMargin(int bottomMargin) {
            auto margins = this->contentsMargins();
            margins.setBottom(bottomMargin);
            this->setContentsMargins(margins);
        }

        int getAngle() {
            return this->angle;
        }

        void setAngle(int angleDegrees) {
            this->angle = angleDegrees;
        }

    protected:
        void paintEvent(QPaintEvent* event) override;

        [[nodiscard]] QSize sizeHint() const override {
            return this->getContainerSize();
        }

        [[nodiscard]] QSize minimumSizeHint() const override {
            return this->getContainerSize();
        }

    private:
        int angle = 90;

        [[nodiscard]] QSize getContainerSize() const;
    };
}
