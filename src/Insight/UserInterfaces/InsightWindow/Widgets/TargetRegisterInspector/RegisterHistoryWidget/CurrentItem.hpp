#pragma once

#include <QHBoxLayout>

#include "Item.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/Label.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/SvgWidget.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace Widgets
{
    class CurrentItem: public Item
    {
        Q_OBJECT

    public:
        CurrentItem(const Targets::TargetMemoryBuffer& registerValue, QWidget* parent);

    private:
        QHBoxLayout* layout = new QHBoxLayout{this};
        Label* titleLabel = new Label{this};
    };
}
