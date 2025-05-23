#include "RegisterHistoryItem.hpp"

#include <QStyle>
#include <QHBoxLayout>
#include <QByteArray>

namespace Widgets
{
    RegisterHistoryItem::RegisterHistoryItem(
        const Targets::TargetMemoryBuffer& registerValue,
        const QDateTime& changeDate,
        QWidget* parent
    )
        : Item(registerValue, parent)
    {
        this->setObjectName("register-history-item");
        this->setFixedHeight(50);
        this->layout->setContentsMargins(5, 8, 5, 0);

        this->dateLabel->setText(changeDate.toString("dd/MM/yyyy hh:mm:ss"));
        this->dateLabel->setObjectName("date-label");

        this->valueLabel->setText("0x" + QString{QByteArray{
                reinterpret_cast<const char *>(registerValue.data()),
                static_cast<qsizetype>(registerValue.size())
        }.toHex()}.toUpper());
        this->valueLabel->setObjectName("value-label");

        this->descriptionLabel->setText("Register Written");
        this->descriptionLabel->setObjectName("description-label");

        auto* subLabelLayout = new QHBoxLayout{};
        subLabelLayout->setSpacing(0);
        subLabelLayout->setContentsMargins(0, 0, 0, 0);
        subLabelLayout->addWidget(this->valueLabel, 0, Qt::AlignmentFlag::AlignLeft);
        subLabelLayout->addStretch(1);
        subLabelLayout->addWidget(this->descriptionLabel, 0, Qt::AlignmentFlag::AlignRight);

        this->layout->setSpacing(5);
        this->layout->addWidget(this->dateLabel, 0, Qt::AlignmentFlag::AlignTop);
        this->layout->addLayout(subLabelLayout);
        this->layout->addStretch(1);
    }
}
