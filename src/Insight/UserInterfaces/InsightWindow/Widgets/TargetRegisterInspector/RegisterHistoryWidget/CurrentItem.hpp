#pragma once

#include <QDateTime>
#include <QHBoxLayout>
#include <QLabel>

#include "Item.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/SvgWidget.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace Bloom::Widgets
{
    class CurrentItem: public Item
    {
    Q_OBJECT
        QHBoxLayout* layout = new QHBoxLayout(this);
        QLabel* titleLabel = new QLabel(this);

    public:
        CurrentItem(
            const Targets::TargetMemoryBuffer& registerValue,
            QWidget *parent
        );
    };
}
