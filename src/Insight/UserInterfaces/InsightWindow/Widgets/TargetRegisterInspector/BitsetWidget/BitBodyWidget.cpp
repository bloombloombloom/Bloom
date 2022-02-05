#include "BitBodyWidget.hpp"

#include <QVBoxLayout>
#include <QScrollArea>
#include <QMargins>

namespace Bloom::Widgets
{
    BitBodyWidget::BitBodyWidget(
        int bitIndex,
        std::bitset<std::numeric_limits<unsigned char>::digits>::reference bit,
        bool readOnly,
        QWidget* parent
    ): ClickableWidget(parent), bitIndex(bitIndex), bit(bit), readOnly(readOnly) {
        this->setFixedSize(BitBodyWidget::WIDTH, BitBodyWidget::HEIGHT);
        this->setContentsMargins(0, 0, 0, 0);
    }

    bool BitBodyWidget::event(QEvent* event) {
        if (this->isEnabled() && !this->readOnly) {
            switch (event->type()) {
                case QEvent::Enter: {
                    this->hoverActive = true;
                    this->update();
                    break;
                }
                case QEvent::Leave: {
                    this->hoverActive = false;
                    this->update();
                    break;
                }
                default: {
                    break;
                }
            }
        }

        return QWidget::event(event);
    }

    void BitBodyWidget::mouseReleaseEvent(QMouseEvent* event) {
        if (this->isEnabled()) {
            if (!this->readOnly && event->button() == Qt::MouseButton::LeftButton) {
                this->bit = !this->bit;
                this->update();
            }

            ClickableWidget::mouseReleaseEvent(event);
        }
    }

    void BitBodyWidget::paintEvent(QPaintEvent* event) {
        auto painter = QPainter(this);
        this->drawWidget(painter);
    }

    void BitBodyWidget::drawWidget(QPainter& painter) {
        painter.setRenderHints(
            QPainter::RenderHint::Antialiasing | QPainter::RenderHint::SmoothPixmapTransform,
            true
        );

        auto bodyColor = QColor(this->bit == true ? "#7B5E38" : "#908D85");

        if (!this->isEnabled()) {
            bodyColor.setAlpha(100);

        } else if (!this->hoverActive) {
            bodyColor.setAlpha(235);
        }

        painter.setPen(Qt::PenStyle::NoPen);
        painter.setBrush(bodyColor);

        painter.drawRect(
            0,
            0,
            BitBodyWidget::WIDTH,
            BitBodyWidget::HEIGHT
        );
    }
}
