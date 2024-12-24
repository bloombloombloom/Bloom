#pragma once

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ListView/ListItem.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace Widgets
{
    class ChangeListItem: public ListItem
    {
    public:
        Targets::TargetMemoryAddressRange addressRange;

        explicit ChangeListItem(const Targets::TargetMemoryAddressRange& addressRange);

        bool operator < (const ListItem& rhs) const override {
            const auto& rhsSnapshotItem = dynamic_cast<const ChangeListItem&>(rhs);
            return this->addressRange.startAddress < rhsSnapshotItem.addressRange.startAddress;
        }

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    private:
        static constexpr int HEIGHT = 50;
    };
}
