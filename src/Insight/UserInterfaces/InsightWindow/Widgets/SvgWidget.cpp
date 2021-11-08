#include "SvgWidget.hpp"

#include <QPainter>
#include <cmath>

using namespace Bloom::Widgets;

SvgWidget::SvgWidget(QWidget* parent): QFrame(parent) {
    this->renderer.setAspectRatioMode(Qt::AspectRatioMode::KeepAspectRatioByExpanding);
}

void SvgWidget::paintEvent(QPaintEvent* paintEvent) {
    auto painter = QPainter(this);
    auto svgSize = this->renderer.defaultSize();
    auto margins = this->contentsMargins();
    const auto containerSize = this->frameSize();

    painter.setRenderHints(QPainter::RenderHint::Antialiasing | QPainter::RenderHint::SmoothPixmapTransform, true);

    if (this->angle % 360 != 0) {
        painter.translate(
            std::ceil(static_cast<float>(containerSize.width() / 2)),
            std::ceil(static_cast<float>(containerSize.height() / 2))
        );
        painter.rotate(this->angle);
        painter.translate(
            -std::ceil(static_cast<float>(containerSize.width() / 2)),
            -std::ceil(static_cast<float>(containerSize.height() / 2))
        );
    }

    this->renderer.render(&painter, QRectF(
        std::ceil(static_cast<float>(containerSize.width() - svgSize.width()) / 2 + static_cast<float>(margins.left())),
        std::ceil(static_cast<float>(containerSize.height() - svgSize.height()) / 2 + static_cast<float>(margins.top())),
        svgSize.width(),
        svgSize.height()
    ));
}

void SvgWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::EnabledChange && !this->disabledSvgFilePath.isEmpty()) {
        if (!this->isEnabled()) {
            this->renderer.load(this->disabledSvgFilePath);

        } else {
            this->renderer.load(this->svgFilePath);
        }

        this->repaint();
    }
}
