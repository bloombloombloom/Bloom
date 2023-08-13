#pragma once

#include <QString>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ListView/ListItem.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemorySnapshot.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace Widgets
{
    class MemorySnapshotItem: public ListItem
    {
    public:
        const MemorySnapshot& memorySnapshot;

        MemorySnapshotItem(const MemorySnapshot& memorySnapshot);

        bool operator < (const ListItem& rhs) const override {
            const auto& rhsSnapshotItem = dynamic_cast<const MemorySnapshotItem&>(rhs);
            return this->memorySnapshot.createdDate > rhsSnapshotItem.memorySnapshot.createdDate;
        }

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    private:
        static constexpr int HEIGHT = 50;

        QString nameText;
        QString programCounterText;
        QString createdDateText;
    };
}
