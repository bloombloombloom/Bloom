#pragma once

#include <QToolButton>
#include <QString>
#include <QChildEvent>

#include "SvgWidget.hpp"

namespace Bloom::Widgets
{
    class SvgToolButton: public QToolButton
    {
        Q_OBJECT
        Q_PROPERTY(QString svgFilePath READ getSvgFilePath WRITE setSvgFilePath DESIGNABLE true)
        Q_PROPERTY(QString disabledSvgFilePath READ getDisabledSvgFilePath WRITE setDisabledSvgFilePath DESIGNABLE true)

        Q_PROPERTY(int buttonWidth READ getButtonWidth WRITE setButtonWidth DESIGNABLE true)
        Q_PROPERTY(int buttonHeight READ getButtonHeight WRITE setButtonHeight DESIGNABLE true)

    public:
        explicit SvgToolButton(QWidget* parent);

        void setSvgFilePath(const QString& svgFilePath) {
            this->svgWidget->setSvgFilePath(svgFilePath);
        }

        [[nodiscard]] QString getSvgFilePath() const {
            return this->svgWidget->getSvgFilePath();
        }

        void setDisabledSvgFilePath(const QString& disabledSvgFilePath) {
            this->svgWidget->setDisabledSvgFilePath(disabledSvgFilePath);
        }

        [[nodiscard]] QString getDisabledSvgFilePath() const {
            return this->svgWidget->getDisabledSvgFilePath();
        }

        void setButtonWidth(int buttonWidth) {
            this->buttonWidth = buttonWidth;
            this->setFixedWidth(buttonWidth);
            this->svgWidget->setContainerWidth(buttonWidth);
        }

        [[nodiscard]] int getButtonWidth() const {
            return this->buttonWidth;
        }

        void setButtonHeight(int buttonHeight) {
            this->buttonHeight = buttonHeight;
            this->setFixedHeight(buttonHeight);
            this->svgWidget->setContainerHeight(buttonHeight);
        }

        [[nodiscard]] int getButtonHeight() const {
            return this->buttonHeight;
        }

        void startSpin() {
            this->svgWidget->startSpin();
        }

        void stopSpin() {
            this->svgWidget->stopSpin();
        }
    protected:
        void childEvent(QChildEvent* childEvent) override;

    private:
        SvgWidget* svgWidget = new SvgWidget(this);
        int buttonWidth = 0;
        int buttonHeight = 0;
    };
}
