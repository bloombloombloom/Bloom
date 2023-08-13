#pragma once

#include <QFrame>
#include <QSvgRenderer>
#include <QString>
#include <QEvent>
#include <QSize>
#include <QPropertyAnimation>

namespace Widgets
{
    class SvgWidget: public QFrame
    {
        Q_OBJECT
        Q_PROPERTY(QString svgFilePath READ getSvgFilePath WRITE setSvgFilePath DESIGNABLE true)
        Q_PROPERTY(QString disabledSvgFilePath READ getDisabledSvgFilePath WRITE setDisabledSvgFilePath DESIGNABLE true)
        Q_PROPERTY(int containerWidth READ getContainerWidth WRITE setContainerWidth DESIGNABLE true)
        Q_PROPERTY(int containerHeight READ getContainerHeight WRITE setContainerHeight DESIGNABLE true)
        Q_PROPERTY(int angle READ getAngle WRITE setAngle DESIGNABLE true)

    public:
        explicit SvgWidget(QWidget* parent);

        void setSvgFilePath(const QString& svgFilePath) {
            this->svgFilePath = svgFilePath;
            this->renderer.load(this->svgFilePath);
        }

        QString getSvgFilePath() {
            return this->svgFilePath;
        }

        void setDisabledSvgFilePath(const QString& disabledSvgFilePath) {
            this->disabledSvgFilePath = disabledSvgFilePath;
        }

        [[nodiscard]] QString getDisabledSvgFilePath() const {
            return this->disabledSvgFilePath;
        }

        void setContainerWidth(int containerWidth) {
            this->containerWidth = containerWidth;
            this->setFixedWidth(this->containerWidth);
        }

        [[nodiscard]] int getContainerWidth() const {
            return this->containerWidth;
        }

        void setContainerHeight(int containerHeight) {
            this->containerHeight = containerHeight;
            this->setFixedHeight(this->containerHeight);
        }

        [[nodiscard]] int getContainerHeight() const {
            return this->containerHeight;
        }

        void setAngle(int angle) {
            this->angle = angle;

            if (this->spinningAnimation != nullptr) {
                this->update();
            }
        }

        [[nodiscard]] int getAngle() const {
            return this->angle;
        }

        /**
         * Starts a spinning animation, where the widget will rotate on its center, indefinitely.
         *
         * This is typically used in SvgToolButton instances, where the spinning of the tool button icon indicates
         * some action being in progress (see the refresh tool button in the TargetMemoryInspectionPane widget).
         */
        void startSpin();

        /**
         * Stops the spinning animation.
         */
        void stopSpin();

    protected:
        void paintEvent(QPaintEvent* paintEvent) override;
        void changeEvent(QEvent* event) override;

    private:
        QSvgRenderer renderer = new QSvgRenderer(this);
        QString svgFilePath;
        QString disabledSvgFilePath;
        int containerWidth = 0;
        int containerHeight = 0;
        int angle = 0;
        QPropertyAnimation* spinningAnimation = nullptr;
    };
}
