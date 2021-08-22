#include "SvgWidget.hpp"

#include <QPainter>
#include <cmath>

using namespace Bloom::Widgets;

SvgWidget::SvgWidget(QWidget* parent): QFrame(parent) {
    this->containerWidth = parent->width();
    this->containerHeight = parent->height();
    this->setFixedSize(this->containerWidth, this->containerHeight);
    this->renderer.setAspectRatioMode(Qt::AspectRatioMode::KeepAspectRatioByExpanding);
}

void SvgWidget::paintEvent(QPaintEvent* paintEvent) {
    auto painter = QPainter(this);
    auto svgSize = this->renderer.defaultSize();
    auto margins = this->contentsMargins();

    painter.setRenderHints(QPainter::RenderHint::Antialiasing | QPainter::RenderHint::SmoothPixmapTransform, true);

    this->setFixedHeight(this->containerHeight + margins.top() + margins.bottom());
    this->setFixedWidth(this->containerWidth + margins.left() + margins.right());

    if (this->angle % 360 != 0) {
        painter.translate(
            std::ceil(static_cast<float>(this->containerWidth / 2)),
            std::ceil(static_cast<float>(this->containerHeight / 2))
        );
        painter.rotate(90);
        painter.translate(
            -std::ceil(static_cast<float>(this->containerWidth / 2)),
            -std::ceil(static_cast<float>(this->containerHeight / 2))
        );
    }

    this->renderer.render(&painter, QRectF(
        std::ceil(static_cast<float>(this->containerWidth - svgSize.width()) / 2 + static_cast<float>(margins.left())),
        std::ceil(static_cast<float>(this->containerHeight - svgSize.height()) / 2 + static_cast<float>(margins.top())),
        svgSize.width(),
        svgSize.height()
    ));
}
