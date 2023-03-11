#pragma once

#include <unordered_map>
#include <vector>
#include <list>
#include <optional>

#include "GroupItem.hpp"
#include "FocusedRegionGroupItem.hpp"
#include "StackMemoryGroupItem.hpp"
#include "ByteItem.hpp"

#include "src/Targets/TargetMemory.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/FocusedMemoryRegion.hpp"

namespace Bloom::Widgets
{
    class TopLevelGroupItem: public GroupItem
    {
    public:
        std::unordered_map<Targets::TargetMemoryAddress, ByteItem> byteItemsByAddress;

        TopLevelGroupItem(
            const std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
            const HexViewerSharedState& hexViewerState
        );

        void rebuildItemHierarchy();

        void refreshValues();

        void adjustItemPositions(const int maximumWidth) {
            GroupItem::adjustItemPositions(maximumWidth, &(this->hexViewerState));
        }

        void setPosition(const QPoint& position) {
            this->relativePosition = position;
        }

    private:
        const std::vector<FocusedMemoryRegion>& focusedMemoryRegions;
        const HexViewerSharedState& hexViewerState;

        std::list<FocusedRegionGroupItem> focusedRegionGroupItems;
        std::optional<StackMemoryGroupItem> stackMemoryGroupItem;
    };
}
