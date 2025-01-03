#pragma once

#include <QString>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ListView/ListItem.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemoryRegion.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace Widgets
{
    class MemoryRegionItem: public ListItem
    {
    public:
        const MemoryRegion& memoryRegion;

        MemoryRegionItem(const MemoryRegion& memoryRegion);

        bool operator < (const ListItem& rhs) const override {
            return this->memoryRegion.createdDate < dynamic_cast<const MemoryRegionItem&>(rhs).memoryRegion.createdDate;
        }

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    private:
        static constexpr int HEIGHT = 50;

        QString nameText;
        QString addressRangeText;
        QString regionTypeText;
        QString createdDateText;
    };
}
