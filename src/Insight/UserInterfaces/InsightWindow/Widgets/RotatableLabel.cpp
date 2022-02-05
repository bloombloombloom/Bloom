#include "RotatableLabel.hpp"

#include <QPainter>

namespace Bloom::Widgets
{
    void RotatableLabel::paintEvent(QPaintEvent* event) {
        auto painter = QPainter(this);
        static auto containerSize = this->getContainerSize();
        static auto textSize = QLabel::minimumSizeHint();
        static auto margins = this->contentsMargins();
        painter.setClipRect(0, 0, containerSize.width(), containerSize.height());
        painter.save();
        painter.setPen(Qt::PenStyle::SolidLine);
        painter.setPen(QColor(this->isEnabled() ? "#afb1b3" : "#808484"));
        painter.translate(std::ceil(containerSize.width() / 2), std::ceil(containerSize.height() / 2));
        painter.rotate(this->angle);
        painter.drawText(
            -(textSize.width() / 2) + margins.left(),
            (textSize.height() / 2) + margins.top(),
            this->text()
        );

        painter.restore();
    }

    QSize RotatableLabel::getContainerSize() const {
        auto size = QSize();
        auto textSize = QLabel::sizeHint();

        if (this->angle % 360 == 0 || this->angle % 180 == 0) {
            size = textSize;

        } else if (this->angle % 90 == 0) {
            size.setHeight(textSize.width());
            size.setWidth(textSize.height());

        } else {
            auto angle = this->angle;

            if (angle > 90) {
                float angleMultiplier = static_cast<float>(angle) / 90;
                angleMultiplier = static_cast<float>(angleMultiplier) - std::floor(angleMultiplier);
                angle = static_cast<int>(90 * angleMultiplier);
            }

            auto angleRadians = angle * (M_PI / 180);

            size.setWidth(static_cast<int>(
                std::cos(angleRadians) * textSize.width()
                + std::ceil(std::sin(angleRadians) * textSize.height())
            ))
            ;
            size.setHeight(static_cast<int>(
                std::sin(angleRadians) * textSize.width()
                + std::ceil(std::cos(angleRadians) * textSize.height())
            ));
        }

        return size;
    }
}
