#pragma once

#include <QWidget>
#include <QEvent>
#include <QMouseEvent>

#include "src/Targets/TargetVariant.hpp"
#include "src/Targets/TargetPinDescriptor.hpp"

namespace Bloom::Widgets::InsightTargetWidgets
{
    class TargetPinBodyWidget: public QWidget
    {
    Q_OBJECT
        /*
         * Pin body colors can be set in QSS files.
         */
        Q_PROPERTY(QColor defaultBodyColor READ getDefaultBodyColor WRITE setDefaultBodyColor DESIGNABLE true)
        Q_PROPERTY(QColor vccBodyColor READ getVccBodyColor WRITE setVccBodyColor DESIGNABLE true)
        Q_PROPERTY(QColor gndBodyColor READ getGndBodyColor WRITE setGndBodyColor DESIGNABLE true)
        Q_PROPERTY(QColor outputHighBodyColor READ getOutputHighBodyColor WRITE setOutputHighBodyColor DESIGNABLE true)
        Q_PROPERTY(QColor inputHighBodyColor READ getInputHighBodyColor WRITE setInputHighBodyColor DESIGNABLE true)

        Q_PROPERTY(int disableAlphaLevel READ getDisableAlphaLevel WRITE setDisableAlphaLevel DESIGNABLE true)

    protected:
        Targets::TargetPinDescriptor pinDescriptor;
        std::optional<Targets::TargetPinState> pinState;

        bool hoverActive = false;

        QColor defaultBodyColor = QColor("#908D85");
        QColor vccBodyColor = QColor("#70383A");
        QColor gndBodyColor = QColor("#484A4B");
        QColor outputHighBodyColor = QColor("#3C5E62");
        QColor inputHighBodyColor = QColor("#7B5E38");

        int disableAlphaLevel = 100;

        bool event(QEvent* event) override;

        void mouseReleaseEvent(QMouseEvent* event) override {
            if (event->button() == Qt::MouseButton::LeftButton) {
                emit this->clicked();
            }

            QWidget::mouseReleaseEvent(event);
        }

    public:
        TargetPinBodyWidget(QWidget* parent, Targets::TargetPinDescriptor pinDescriptor):
        QWidget(parent), pinDescriptor(std::move(pinDescriptor)) {
            this->setObjectName("target-pin-body");
        }

        QColor getBodyColor();

        void setPinState(const Targets::TargetPinState& pinState) {
            this->pinState = pinState;
        }

        const QColor& getDefaultBodyColor() const {
            return this->defaultBodyColor;
        }

        void setDefaultBodyColor(const QColor& color) {
            this->defaultBodyColor = color;
        }

        const QColor& getVccBodyColor() const {
            return this->vccBodyColor;
        }

        void setVccBodyColor(const QColor& vccBodyColor) {
            this->vccBodyColor = vccBodyColor;
        }

        const QColor& getGndBodyColor() const {
            return this->gndBodyColor;
        }

        void setGndBodyColor(const QColor& gndBodyColor) {
            this->gndBodyColor = gndBodyColor;
        }

        const QColor& getOutputHighBodyColor() const {
            return this->outputHighBodyColor;
        }

        void setOutputHighBodyColor(const QColor& outputHighBodyColor) {
            this->outputHighBodyColor = outputHighBodyColor;
        }

        const QColor& getInputHighBodyColor() const {
            return this->inputHighBodyColor;
        }

        void setInputHighBodyColor(const QColor& inputHighBodyColor) {
            this->inputHighBodyColor = inputHighBodyColor;
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
