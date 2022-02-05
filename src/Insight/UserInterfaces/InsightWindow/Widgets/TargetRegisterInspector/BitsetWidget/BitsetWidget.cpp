#include "BitsetWidget.hpp"

#include <QVBoxLayout>
#include <QScrollArea>
#include <QMargins>
#include <QLine>
#include <set>

#include "../TargetRegisterInspectorWindow.hpp"

namespace Bloom::Widgets
{
    BitsetWidget::BitsetWidget(int byteNumber, unsigned char& byte, bool readOnly, QWidget* parent)
    : QWidget(parent), byteNumber(byteNumber), byte(byte), readOnly(readOnly) {
        this->setObjectName("bitset-widget");
        auto* bitLayout = new QHBoxLayout(this);
        bitLayout->setSpacing(BitWidget::SPACING);
        bitLayout->setContentsMargins(0, 0, 0, 0);
        this->setContentsMargins(0, 0, 0, 0);
        this->setFixedSize(
            static_cast<int>((BitWidget::WIDTH + BitWidget::SPACING) * this->bitset.size() - BitWidget::SPACING),
            BitsetWidget::HEIGHT
        );

        for (int bitIndex = (std::numeric_limits<unsigned char>::digits - 1); bitIndex >= 0; bitIndex--) {
            auto* bitWidget = new BitWidget(
                bitIndex,
                (this->byteNumber * 8) + bitIndex,
                this->bitset,
                this->readOnly,
                this
            );

            bitLayout->addWidget(bitWidget, 0, Qt::AlignmentFlag::AlignHCenter | Qt::AlignmentFlag::AlignTop);
            QObject::connect(
                bitWidget,
                &BitWidget::bitChanged,
                this,
                [this] {
                    this->byte = static_cast<unsigned char>(this->bitset.to_ulong());
                    this->repaint();
                    emit this->byteChanged();
                }
            );
        }
    }

    void BitsetWidget::updateValue() {
        this->bitset = {this->byte};
        this->update();
    }

    void BitsetWidget::paintEvent(QPaintEvent* event) {
        QWidget::paintEvent(event);
        auto painter = QPainter(this);
        this->drawWidget(painter);
    }

    void BitsetWidget::drawWidget(QPainter& painter) {
        auto byteHex = "0x" + QString::number(this->byte, 16).toUpper();

        painter.setPen(QColor("#474747"));
        constexpr int labelHeight = 11;
        int containerWidth = this->width();
        constexpr int charWidth = 6;
        auto labelWidth = static_cast<int>(charWidth * byteHex.size()) + 13;
        auto width = (containerWidth - (BitWidget::WIDTH) - labelWidth) / 2;

        painter.drawLine(QLine(
            BitWidget::WIDTH / 2,
            BitWidget::HEIGHT,
            BitWidget::WIDTH / 2,
            BitWidget::HEIGHT + BitsetWidget::VALUE_GRAPHIC_HEIGHT
        ));

        painter.drawLine(QLine(
            containerWidth - (BitWidget::WIDTH / 2) - 1,
            BitWidget::HEIGHT,
            containerWidth - (BitWidget::WIDTH / 2) - 1,
            BitWidget::HEIGHT + BitsetWidget::VALUE_GRAPHIC_HEIGHT
        ));

        painter.drawLine(QLine(
            BitWidget::WIDTH / 2,
            BitWidget::HEIGHT + BitsetWidget::VALUE_GRAPHIC_HEIGHT,
            static_cast<int>((BitWidget::WIDTH / 2) + width),
            BitWidget::HEIGHT + BitsetWidget::VALUE_GRAPHIC_HEIGHT
        ));

        painter.drawLine(QLine(
            static_cast<int>((BitWidget::WIDTH / 2) + width + labelWidth),
            BitWidget::HEIGHT + BitsetWidget::VALUE_GRAPHIC_HEIGHT,
            containerWidth - (BitWidget::WIDTH / 2) - 1,
            BitWidget::HEIGHT + BitsetWidget::VALUE_GRAPHIC_HEIGHT
        ));

        painter.setPen(QColor("#8a8a8d"));
        painter.drawText(
            QRect(
                static_cast<int>((BitWidget::WIDTH / 2) + width),
                BitWidget::HEIGHT + BitsetWidget::VALUE_GRAPHIC_HEIGHT - (labelHeight / 2),
                labelWidth,
                labelHeight
            ),
            Qt::AlignCenter,
            byteHex
        );
    }
}
