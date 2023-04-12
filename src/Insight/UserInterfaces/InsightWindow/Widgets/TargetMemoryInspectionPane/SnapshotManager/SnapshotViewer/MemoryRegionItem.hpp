#pragma once

#include <QString>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ListView/ListItem.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemoryRegion.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace Bloom::Widgets
{
    class MemoryRegionItem: public ListItem
    {
    public:
        const MemoryRegion& memoryRegion;

        MemoryRegionItem(const MemoryRegion& memoryRegion);

        bool operator < (const ListItem& rhs) const override {
            const auto& rhsRegionItem = dynamic_cast<const MemoryRegionItem&>(rhs);
            return this->memoryRegion.createdDate < rhsRegionItem.memoryRegion.createdDate;
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
