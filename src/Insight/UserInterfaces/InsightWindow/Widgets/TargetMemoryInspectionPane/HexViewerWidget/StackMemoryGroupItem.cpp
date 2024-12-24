#include "StackMemoryGroupItem.hpp"

#include <cassert>

namespace Widgets
{
    StackMemoryGroupItem::StackMemoryGroupItem(
        Targets::TargetStackPointer stackPointer,
        const HexViewerSharedState& hexViewerState,
        const std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
        std::unordered_map<Targets::TargetMemoryAddress, ByteItem>& byteItemsByAddress,
        HexViewerItem* parent
    )
        : GroupItem(stackPointer + 1, parent)
        , stackPointer(stackPointer)
        , hexViewerState(hexViewerState)
    {
        const auto startAddress = this->startAddress;
        const auto endAddress = this->hexViewerState.memorySegmentDescriptor.addressRange.endAddress;

        // Sanity check
        assert(byteItemsByAddress.contains(startAddress) && byteItemsByAddress.contains(endAddress));

        for (const auto& focusedRegion : focusedMemoryRegions) {
            if (
                focusedRegion.addressRange.startAddress < startAddress
                || focusedRegion.addressRange.endAddress > endAddress
            ) {
                /*
                 * This focused region is outside of stack memory, so we don't need to handle it here.
                 *
                 * This will also catch the regions that intersect with stack memory but are not completely contained
                 * within the stack. There's no clear way to present these regions, so we ignore them, for now.
                 */
                continue;
            }

            this->focusedRegionGroupItems.emplace_back(focusedRegion, byteItemsByAddress, this);
            items.emplace_back(&(this->focusedRegionGroupItems.back()));
        }

        for (auto address = startAddress; address <= endAddress; ++address) {
            auto& byteItem = byteItemsByAddress.at(address);

            byteItem.stackMemory = true;

            if (byteItem.parent == nullptr || byteItem.parent == this->parent) {
                byteItem.parent = this;
                this->items.push_back(&byteItem);
            }
        }

        this->sortItems();
    }

    StackMemoryGroupItem::~StackMemoryGroupItem() {
        const auto updateChildItems = [] (const decltype(this->items)& items, const auto& updateChildItems) -> void {
            for (auto& item : items) {
                auto* byteItem = dynamic_cast<ByteItem*>(item);

                if (byteItem != nullptr) {
                    byteItem->stackMemory = false;
                    continue;
                }

                auto* groupItem = dynamic_cast<GroupItem*>(item);

                if (groupItem != nullptr) {
                    updateChildItems(groupItem->items, updateChildItems);
                }
            }
        };

        updateChildItems(this->items, updateChildItems);
    }

    void StackMemoryGroupItem::adjustItemPositions(int maximumWidth, const HexViewerSharedState* hexViewerState) {
        GroupItem::adjustItemPositions(maximumWidth, hexViewerState);
        this->groupSize.setWidth(maximumWidth);
    }

    void StackMemoryGroupItem::refreshValues() {
        for (auto& focusedRegionItem : this->focusedRegionGroupItems) {
            focusedRegionItem.refreshValue(this->hexViewerState);
        }
    }

    QMargins StackMemoryGroupItem::groupMargins(const HexViewerSharedState* hexViewerState, int maximumWidth) const {
        return {0, 100, 0, 20};
    }
}
