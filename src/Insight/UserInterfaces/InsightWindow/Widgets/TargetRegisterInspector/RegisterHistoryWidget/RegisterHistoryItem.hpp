#pragma once

#include <QDateTime>
#include <QVBoxLayout>
#include <QLabel>

#include "Item.hpp"

namespace Bloom::Widgets
{
    class RegisterHistoryItem: public Item
    {
    Q_OBJECT
        QVBoxLayout* layout = new QVBoxLayout(this);
        QLabel* dateLabel = new QLabel(this);
        QLabel* valueLabel = new QLabel(this);
        QLabel* descriptionLayout = new QLabel(this);

    public:
        RegisterHistoryItem(
            const Targets::TargetMemoryBuffer& registerValue,
            const QDateTime& changeDate,
            QWidget *parent
        );
    };
}
