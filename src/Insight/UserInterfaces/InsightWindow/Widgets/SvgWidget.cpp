#include "SvgWidget.hpp"

#include <QPainter>
#include <cmath>

namespace Widgets
{
    SvgWidget::SvgWidget(QWidget* parent): QFrame(parent) {
        this->renderer->setAspectRatioMode(Qt::AspectRatioMode::KeepAspectRatioByExpanding);
    }

    void SvgWidget::startSpin() {
        if (this->spinningAnimation == nullptr) {
            this->spinningAnimation = new QPropertyAnimation{this, "angle", this};
            this->spinningAnimation->setDuration(2000);
            this->spinningAnimation->setStartValue(0);
            this->spinningAnimation->setEndValue(360);

            QObject::connect(this->spinningAnimation, &QPropertyAnimation::finished, this, [this] {
                this->spinningAnimation->start();
            });
        }

        this->spinningAnimation->start();
    }

    void SvgWidget::stopSpin() {
        if (this->spinningAnimation != nullptr) {
            this->spinningAnimation->stop();
            this->setAngle(0);
        }
    }

    void SvgWidget::paintEvent(QPaintEvent* paintEvent) {
        auto painter = QPainter(this);
        auto svgSize = this->renderer->defaultSize();
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

        this->renderer->render(&painter, QRectF(
            std::ceil(
                static_cast<float>(containerSize.width() - svgSize.width()) / 2 + static_cast<float>(margins.left())
            ),
            std::ceil(
                static_cast<float>(containerSize.height() - svgSize.height()) / 2 + static_cast<float>(margins.top())
            ),
            svgSize.width(),
            svgSize.height()
        ));
    }

    void SvgWidget::changeEvent(QEvent* event) {
        if (event->type() == QEvent::EnabledChange && !this->disabledSvgFilePath.isEmpty()) {
            if (!this->isEnabled()) {
                this->renderer->load(this->disabledSvgFilePath);

            } else {
                this->renderer->load(this->svgFilePath);
            }

            this->repaint();
        }
    }
}
