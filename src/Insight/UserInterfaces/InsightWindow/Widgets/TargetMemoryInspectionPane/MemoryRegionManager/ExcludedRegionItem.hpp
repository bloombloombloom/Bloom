#pragma once

#include "RegionItem.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/ExcludedMemoryRegion.hpp"

namespace Bloom::Widgets
{
    class ExcludedRegionItem: public RegionItem
    {
        Q_OBJECT

    public:
        ExcludedRegionItem(
            const ExcludedMemoryRegion& region,
            const Targets::TargetMemoryDescriptor& memoryDescriptor,
            QWidget *parent
        );

        [[nodiscard]] const ExcludedMemoryRegion& getMemoryRegion() const override {
            return this->memoryRegion;
        };

        virtual void applyChanges();

    private:
        ExcludedMemoryRegion memoryRegion;
    };
}
