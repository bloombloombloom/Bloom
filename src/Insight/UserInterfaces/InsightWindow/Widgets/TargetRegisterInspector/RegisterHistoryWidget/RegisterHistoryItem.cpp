#include "RegisterHistoryItem.hpp"

#include <QStyle>
#include <QVBoxLayout>
#include <QByteArray>

using namespace Bloom::Widgets;

RegisterHistoryItem::RegisterHistoryItem(
    const Targets::TargetMemoryBuffer& registerValue,
    const QDateTime& changeDate,
    QWidget* parent
): Item(registerValue, parent) {
    this->setObjectName("register-history-item");
    this->setFixedHeight(50);
    this->layout->setContentsMargins(5, 8, 5, 0);

    this->dateLabel->setText(changeDate.toString("dd/MM/yyyy hh:mm:ss"));
    this->dateLabel->setObjectName("date-label");

    this->valueLabel->setText("0x" + QString(QByteArray(
        reinterpret_cast<const char*>(registerValue.data()),
        static_cast<qsizetype>(registerValue.size())
    ).toHex()).toUpper());
    this->valueLabel->setObjectName("value-label");

    auto labelLayout = new QVBoxLayout();
    labelLayout->setSpacing(5);
    labelLayout->setContentsMargins(0, 0, 0, 0);
    labelLayout->addWidget(this->dateLabel, 0, Qt::AlignmentFlag::AlignTop);
    labelLayout->addWidget(this->valueLabel, 0, Qt::AlignmentFlag::AlignTop);
    labelLayout->addStretch(1);

    this->layout->setSpacing(1);
    this->layout->addLayout(labelLayout);

    auto onClick = [this] {
        this->setSelected(true);
    };

    this->connect(this, &ClickableWidget::clicked, this, onClick);
    this->connect(this, &ClickableWidget::rightClicked, this, onClick);

    this->setSelected(false);
}
