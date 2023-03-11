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
        this->stackMemoryGroupItem.reset();

        const auto& currentStackPointer = this->hexViewerState.currentStackPointer;
        const auto stackGroupingRequired = currentStackPointer.has_value()
            && this->hexViewerState.settings.groupStackMemory
            && (
                static_cast<std::int64_t>(this->hexViewerState.memoryDescriptor.addressRange.endAddress)
                - static_cast<std::int64_t>(*currentStackPointer + 1)
            ) > 0;

        for (const auto& focusedRegion : this->focusedMemoryRegions) {
            if (
                stackGroupingRequired
                && (
                    focusedRegion.addressRange.startAddress > *currentStackPointer + 1
                    || focusedRegion.addressRange.endAddress > *currentStackPointer + 1
                )
            ) {
                /*
                 * This focused region contains stack memory - the StackMemoryGroupItem will create and manage the
                 * corresponding FocusedMemoryRegionGroupItem for it.
                 */
                continue;
            }

            this->focusedRegionGroupItems.emplace_back(focusedRegion, this->byteItemsByAddress, this);
            items.emplace_back(&(this->focusedRegionGroupItems.back()));
        }

        if (stackGroupingRequired) {
            this->stackMemoryGroupItem.emplace(
                *(currentStackPointer),
                this->hexViewerState,
                this->focusedMemoryRegions,
                this->byteItemsByAddress,
                this
            );

            items.emplace_back(&*(this->stackMemoryGroupItem));
        }

        for (auto& [address, byteItem] : this->byteItemsByAddress) {
            if (byteItem.parent != nullptr && byteItem.parent != this) {
                // This ByteItem is managed by another group
                continue;
            }

            byteItem.parent = this;
            this->items.push_back(&byteItem);
        }

        this->sortItems();
        this->refreshValues();
    }

    void TopLevelGroupItem::refreshValues() {
        for (auto& focusedRegionItem : this->focusedRegionGroupItems) {
            focusedRegionItem.refreshValue(this->hexViewerState);
        }

        if (this->stackMemoryGroupItem.has_value()) {
            this->stackMemoryGroupItem->refreshValues();
        }
    }
}
