#pragma once

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ClickableWidget.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace Bloom::Widgets
{
    class Item: public ClickableWidget
    {
        Q_OBJECT
    public:
        Targets::TargetMemoryBuffer registerValue;
        Item(const Targets::TargetMemoryBuffer& registerValue, QWidget *parent);

    public slots:
        void setSelected(bool selected);

    signals:
        void selected(Item*);
    };
}
