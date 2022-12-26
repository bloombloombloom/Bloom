#include "MemorySnapshotItem.hpp"

#include "src/Services/DateTimeService.hpp"

namespace Bloom::Widgets
{
    MemorySnapshotItem::MemorySnapshotItem(
        const MemorySnapshot& memorySnapshot,
        QWidget *parent
    )
        : memorySnapshot(memorySnapshot)
        , ClickableWidget(parent)
    {
        this->setObjectName("snapshot-item");
        this->setFixedHeight(50);
        this->layout->setContentsMargins(5, 5, 5, 0);

        this->nameLabel->setText(memorySnapshot.name);
        this->nameLabel->setObjectName("name-label");

        this->programCounterLabel->setText("0x" + QString::number(this->memorySnapshot.programCounter, 16).toUpper());
        this->programCounterLabel->setObjectName("program-counter-label");

        this->createdDateLabel->setText(
            memorySnapshot.createdDate.toString(
                memorySnapshot.createdDate.date() == Services::DateTimeService::currentDate()
                    ? "hh:mm"
                    : "dd/MM/yyyy hh:mm"
            )
        );
        this->createdDateLabel->setObjectName("created-date-label");

        auto* topLabelLayout = new QHBoxLayout();
        topLabelLayout->setSpacing(0);
        topLabelLayout->setContentsMargins(0, 0, 0, 0);
        topLabelLayout->addWidget(this->nameLabel, 0, Qt::AlignmentFlag::AlignLeft);
        topLabelLayout->addStretch(1);
        topLabelLayout->addWidget(this->programCounterLabel, 0, Qt::AlignmentFlag::AlignRight);

        auto* bottomLabelLayout = new QHBoxLayout();
        bottomLabelLayout->setSpacing(0);
        bottomLabelLayout->setContentsMargins(0, 0, 0, 0);
        bottomLabelLayout->addWidget(this->createdDateLabel, 0, Qt::AlignmentFlag::AlignLeft);

        this->layout->setSpacing(5);
        this->layout->addLayout(topLabelLayout);
        this->layout->addLayout(bottomLabelLayout);
        this->layout->addStretch(1);

        auto onClick = [this] {
            this->setSelected(true);
        };

        QObject::connect(this, &ClickableWidget::clicked, this, onClick);
        QObject::connect(this, &ClickableWidget::rightClicked, this, onClick);

        this->setSelected(false);
    }

    void MemorySnapshotItem::setSelected(bool selected) {
        this->setProperty("selected", selected);
        this->style()->unpolish(this);
        this->style()->polish(this);

        if (selected) {
            emit this->selected(this);
        }
    }
}
