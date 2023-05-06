#pragma once

#include <optional>
#include <QString>

#include "GroupItem.hpp"
#include "ByteItem.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/FocusedMemoryRegion.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace Bloom::Widgets
{
    class FocusedRegionGroupItem: public GroupItem
    {
    public:
        static constexpr int ANNOTATION_HEIGHT = 35;

        const FocusedMemoryRegion& focusedMemoryRegion;
        std::optional<QString> valueLabel;

        FocusedRegionGroupItem(
            const FocusedMemoryRegion& focusedRegion,
            std::unordered_map<Targets::TargetMemoryAddress, ByteItem>& byteItemsByAddress,
            HexViewerItem* parent
        );

        ~FocusedRegionGroupItem();

        void refreshValue(const HexViewerSharedState& hexViewerState);

    protected:
        QMargins groupMargins(const HexViewerSharedState* hexViewerState, const int maximumWidth) const override;
    };
}
