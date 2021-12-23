#pragma once

#include "RegionItem.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/ExcludedMemoryRegion.hpp"

namespace Bloom::Widgets
{
    class ExcludedRegionItem: public RegionItem
    {
        Q_OBJECT

    public:
        ExcludedRegionItem(const ExcludedMemoryRegion& region, QWidget *parent);

        [[nodiscard]] const MemoryRegion& getMemoryRegion() const override {
            return this->memoryRegion;
        };

        [[nodiscard]] virtual ExcludedMemoryRegion generateExcludedMemoryRegionFromInput();

    private:
        ExcludedMemoryRegion memoryRegion;
    };
}
