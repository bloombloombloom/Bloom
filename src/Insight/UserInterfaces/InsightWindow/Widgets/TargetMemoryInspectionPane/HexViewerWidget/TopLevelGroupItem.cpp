#include "TopLevelGroupItem.hpp"

namespace Bloom::Widgets
{
    TopLevelGroupItem::TopLevelGroupItem(
        const std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
        const HexViewerSharedState& hexViewerState
    )
        : GroupItem(0, nullptr)
        , focusedMemoryRegions(focusedMemoryRegions)
        , hexViewerState(hexViewerState)
    {
        const auto memorySize = this->hexViewerState.memoryDescriptor.size();
        const auto startAddress = this->hexViewerState.memoryDescriptor.addressRange.startAddress;

        for (Targets::TargetMemorySize i = 0; i < memorySize; i++) {
            const auto address = startAddress + i;
            this->byteItemsByAddress.emplace(address, ByteItem(address));
        }
    }

    void TopLevelGroupItem::rebuildItemHierarchy() {
        this->items.clear();
        this->focusedRegionGroupItems.clear();

        for (const auto& focusedRegion : this->focusedMemoryRegions) {
            this->focusedRegionGroupItems.emplace_back(focusedRegion, this->byteItemsByAddress, this);
            items.emplace_back(&(this->focusedRegionGroupItems.back()));
        }

        for (auto& [address, byteItem] : this->byteItemsByAddress) {
            if (byteItem.parent != nullptr && byteItem.parent != this) {
                // This ByteItem is managed by another group
                continue;
            }

            byteItem.parent = this;
            byteItem.grouped = false;
            this->items.push_back(&byteItem);
        }

        this->sortItems();
        this->refreshValues();
    }

    void TopLevelGroupItem::refreshValues() {
        for (auto& focusedRegionItem : this->focusedRegionGroupItems) {
            focusedRegionItem.refreshValue(this->hexViewerState);
        }
    }
}
