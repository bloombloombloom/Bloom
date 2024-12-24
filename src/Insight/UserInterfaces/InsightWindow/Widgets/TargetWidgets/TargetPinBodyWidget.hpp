#pragma once

#include <QWidget>
#include <QEvent>
#include <QMouseEvent>
#include <optional>
#include <functional>

#include "src/Targets/TargetPinDescriptor.hpp"
#include "src/Targets/TargetPadDescriptor.hpp"
#include "src/Targets/TargetGpioPadState.hpp"

namespace Widgets::InsightTargetWidgets
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

    public:
        TargetPinBodyWidget(
            const Targets::TargetPinDescriptor& pinDescriptor,
            std::optional<std::reference_wrapper<const Targets::TargetPadDescriptor>> padDescriptor,
            QWidget* parent
        );

        void setPadState(const Targets::TargetGpioPadState& padState) {
            this->padState = padState;
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

    protected:
        const Targets::TargetPinDescriptor& pinDescriptor;
        std::optional<std::reference_wrapper<const Targets::TargetPadDescriptor>> padDescriptor;
        std::optional<Targets::TargetGpioPadState> padState;

        bool hoverActive = false;

        QColor defaultBodyColor = {"#908D85"};
        QColor vccBodyColor = {"#70383A"};
        QColor gndBodyColor = {"#484A4B"};
        QColor outputHighBodyColor = {"#3C5E62"};
        QColor inputHighBodyColor = {"#7B5E38"};

        int disableAlphaLevel = 100;

        QColor getBodyColor();

        bool event(QEvent* event) override;

        void mouseReleaseEvent(QMouseEvent* event) override {
            if (event->button() == Qt::MouseButton::LeftButton) {
                emit this->clicked();
            }

            QWidget::mouseReleaseEvent(event);
        }
    };
}
