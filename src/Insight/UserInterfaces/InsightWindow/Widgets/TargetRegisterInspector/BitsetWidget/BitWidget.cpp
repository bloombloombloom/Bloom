#include "BitWidget.hpp"

#include <QVBoxLayout>
#include <QScrollArea>
#include <QMargins>

#include "BitBodyWidget.hpp"

namespace Bloom::Widgets
{
    BitWidget::BitWidget(
        int bitIndex,
        int bitNumber,
        std::bitset<std::numeric_limits<unsigned char>::digits>& bitset,
        bool readOnly,
        QWidget* parent
    ): QWidget(parent), bitIndex(bitIndex), bitNumber(bitNumber), bitset(bitset), readOnly(readOnly) {
        this->setFixedSize(BitWidget::WIDTH, BitWidget::HEIGHT);

        auto* layout = new QVBoxLayout(this);
        layout->setSpacing(0);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setAlignment(Qt::AlignmentFlag::AlignTop);

        this->body = new BitBodyWidget(
            this->bitIndex,
            this->bitset[static_cast<size_t>(this->bitIndex)],
            this->readOnly,
            this
        );

        this->bitLabel = new QLabel("Bit", this);
        this->bitNumberLabel = new QLabel(QString::number(this->bitNumber), this);

        this->bitLabel->setObjectName("register-bit-label");
        this->bitNumberLabel->setObjectName("register-bit-number-label");

        this->bitLabel->setFixedHeight(BitWidget::LABEL_HEIGHT);
        this->bitNumberLabel->setFixedHeight(BitWidget::LABEL_HEIGHT);

        this->bitLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);
        this->bitNumberLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

        layout->addWidget(this->bitLabel);
        layout->addWidget(this->bitNumberLabel);
        layout->addSpacing(BitWidget::VERTICAL_SPACING);
        layout->addWidget(this->body);
        layout->addStretch(1);

        if (!this->readOnly) {
            QObject::connect(this->body, &BitBodyWidget::clicked, this, [this] {
                emit this->bitChanged();
            });
        }
    }
}
