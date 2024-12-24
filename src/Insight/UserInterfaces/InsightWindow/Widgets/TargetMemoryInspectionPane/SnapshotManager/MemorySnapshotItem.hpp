#pragma once

#include <QString>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ListView/ListItem.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemorySnapshot.hpp"

namespace Widgets
{
    class MemorySnapshotItem: public ListItem
    {
    public:
        const MemorySnapshot& memorySnapshot;

        MemorySnapshotItem(const MemorySnapshot& memorySnapshot);

        bool operator < (const ListItem& rhs) const override {
            return this->memorySnapshot.createdDate >
                dynamic_cast<const MemorySnapshotItem&>(rhs).memorySnapshot.createdDate;
        }

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    private:
        static constexpr int HEIGHT = 50;

        QString nameText;
        QString programCounterText;
        QString createdDateText;
    };
}
