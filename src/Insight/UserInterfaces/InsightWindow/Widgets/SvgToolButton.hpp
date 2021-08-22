#pragma once

#include <QToolButton>
#include <QString>

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

    private:
        SvgWidget* svgWidget = new SvgWidget(this);
        SvgWidget* disabledSvgWidget = nullptr;
        int buttonWidth = 0;
        int buttonHeight = 0;

    protected:
        void changeEvent(QEvent* event) override {
            if (event->type() == QEvent::EnabledChange && this->disabledSvgWidget != nullptr) {
                auto enabled = this->isEnabled();
                this->svgWidget->setVisible(enabled);
                this->disabledSvgWidget->setVisible(!enabled);
            }
        };

    public:
        explicit SvgToolButton(QWidget* parent): QToolButton(parent) {
            this->setButtonWidth(10);
            this->setButtonHeight(10);
        };

        void setSvgFilePath(const QString& svgFilePath) {
            this->svgWidget->setSvgFilePath(svgFilePath);
        }

        [[nodiscard]] QString getSvgFilePath() const {
            return this->svgWidget->getSvgFilePath();
        }

        void setDisabledSvgFilePath(const QString& disabledSvgFilePath) {
            if (this->disabledSvgWidget == nullptr) {
                this->disabledSvgWidget = new SvgWidget(this);
                this->disabledSvgWidget->setContainerWidth(this->buttonWidth);
                this->disabledSvgWidget->setContainerHeight(this->buttonHeight);
            }

            this->disabledSvgWidget->setSvgFilePath(disabledSvgFilePath);
        }

        [[nodiscard]] QString getDisabledSvgFilePath() const {
            if (this->disabledSvgWidget != nullptr) {
                return this->disabledSvgWidget->getSvgFilePath();
            }

            return QString();
        }

        void setButtonWidth(int buttonWidth) {
            this->buttonWidth = buttonWidth;
            this->setFixedWidth(buttonWidth);
            this->svgWidget->setContainerWidth(buttonWidth);

            if (this->disabledSvgWidget != nullptr) {
                this->disabledSvgWidget->setContainerWidth(buttonWidth);
            }
        }

        [[nodiscard]] int getButtonWidth() const {
            return this->buttonWidth;
        }

        void setButtonHeight(int buttonHeight) {
            this->buttonHeight = buttonHeight;
            this->setFixedHeight(buttonHeight);
            this->svgWidget->setContainerHeight(buttonHeight);

            if (this->disabledSvgWidget != nullptr) {
                this->disabledSvgWidget->setContainerHeight(buttonHeight);
            }
        }

        [[nodiscard]] int getButtonHeight() const {
            return this->buttonHeight;
        }
    };
}
