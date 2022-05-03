#pragma once

#include <QDateTime>
#include <QVBoxLayout>

#include "Item.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/Label.hpp"

namespace Bloom::Widgets
{
    class RegisterHistoryItem: public Item
    {
        Q_OBJECT

    public:
        RegisterHistoryItem(
            const Targets::TargetMemoryBuffer& registerValue,
            const QDateTime& changeDate,
            QWidget *parent
        );

    private:
        QVBoxLayout* layout = new QVBoxLayout(this);
        Label* dateLabel = new Label(this);
        Label* valueLabel = new Label(this);
        Label* descriptionLabel = new Label(this);
    };
}
