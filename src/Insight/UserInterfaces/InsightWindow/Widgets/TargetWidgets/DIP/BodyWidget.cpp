#include "BodyWidget.hpp"

namespace Bloom::Widgets::InsightTargetWidgets::Dip
{
    BodyWidget::BodyWidget(QWidget* parent, std::size_t pinCount): QWidget(parent) {
        this->setObjectName("target-body");

        /*
         * The DIP package widget looks awkward when the body height is fixed. For this reason, the body height and
         * indicator sizes are proportional to the pin count.
         *
         * The proportionality constants used below were chosen because they look the nicest. No other reason.
         */
        this->setFixedHeight(
            std::min(
                BodyWidget::MAXIMUM_BODY_HEIGHT,
                std::max(
                    BodyWidget::MINIMUM_BODY_HEIGHT,
                    static_cast<int>(std::ceil(3.6 * static_cast<double>(pinCount)))
                )
            )
        );

        this->firstPinIndicatorDiameter = std::min(
            BodyWidget::MAXIMUM_FIRST_PIN_INDICATOR_HEIGHT,
            std::max(BodyWidget::MINIMUM_FIRST_PIN_INDICATOR_HEIGHT, static_cast<int>(std::floor(pinCount / 2)))
        );

        this->orientationIndicatorDiameter = this->firstPinIndicatorDiameter % 2 == 0 ?
            this->firstPinIndicatorDiameter + 3
            : this->firstPinIndicatorDiameter + 2;
    }

    void BodyWidget::paintEvent(QPaintEvent* event) {
        auto painter = QPainter(this);
        this->drawWidget(painter);
    }

    void BodyWidget::drawWidget(QPainter& painter) {
        painter.setRenderHints(
            QPainter::RenderHint::Antialiasing | QPainter::RenderHint::SmoothPixmapTransform,
            true
        );
        const auto bodyHeight = this->height();
        const auto bodyRect = QRectF(
            0,
            0,
            this->width(),
            bodyHeight
        );

        // The first pin indicator is just a circle positioned close to the first pin
        const auto firstPinIndicatorRect = QRectF(
            6,
            (bodyHeight - this->firstPinIndicatorDiameter - 6),
            this->firstPinIndicatorDiameter,
            this->firstPinIndicatorDiameter
        );

        /*
         * The orientation indicator is just a half-circle cut-out, positioned on the side of the DIP package
         * closest to the first pin.
         */
        const auto orientationIndicatorRect = QRectF(
            -(this->orientationIndicatorDiameter / 2),
            (bodyHeight / 2) - (this->orientationIndicatorDiameter / 2),
            this->orientationIndicatorDiameter,
            this->orientationIndicatorDiameter
        );

        static const auto backgroundColor = QColor(0x37, 0x38, 0x35);
        auto targetBodyColor = this->getBodyColor();

        if (!this->isEnabled()) {
            targetBodyColor.setAlpha(this->getDisableAlphaLevel());
        }

        painter.setPen(Qt::PenStyle::NoPen);
        painter.setBrush(targetBodyColor);
        painter.drawRect(bodyRect);
        painter.setBrush(backgroundColor);
        painter.drawEllipse(firstPinIndicatorRect);
        painter.drawEllipse(orientationIndicatorRect);
    }
}
