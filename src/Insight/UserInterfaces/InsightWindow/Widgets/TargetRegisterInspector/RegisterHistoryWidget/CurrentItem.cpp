#include "CurrentItem.hpp"

#include <QStyle>
#include <QVBoxLayout>

namespace Widgets
{
    CurrentItem::CurrentItem(
        const Targets::TargetMemoryBuffer& registerValue,
        QWidget* parent
    ): Item(registerValue, parent) {
        this->setObjectName("current-item");
        this->setFixedHeight(30);
        this->titleLabel->setText("Current Value");
        this->layout->addWidget(titleLabel, 0, Qt::AlignmentFlag::AlignLeft);
        this->layout->setContentsMargins(5, 0, 5, 0);
    }
}
