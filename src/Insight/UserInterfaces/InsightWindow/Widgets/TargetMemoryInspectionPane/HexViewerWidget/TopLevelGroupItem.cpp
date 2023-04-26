#include "TopLevelGroupItem.hpp"

namespace Bloom::Widgets
{
    TopLevelGroupItem::TopLevelGroupItem(
        const std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
        const std::vector<ExcludedMemoryRegion>& excludedMemoryRegions,
        const HexViewerSharedState& hexViewerState
    )
        : GroupItem(0, nullptr)
        , focusedMemoryRegions(focusedMemoryRegions)
        , excludedMemoryRegions(excludedMemoryRegions)
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
            && *currentStackPointer >= this->hexViewerState.memoryDescriptor.addressRange.startAddress
            && (*currentStackPointer + 1) <= this->hexViewerState.memoryDescriptor.addressRange.endAddress;

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
            byteItem.excluded = false;

            if (byteItem.parent != nullptr && byteItem.parent != this) {
                // This ByteItem is managed by another group
                continue;
            }

            byteItem.parent = this;
            this->items.push_back(&byteItem);
        }

        for (const auto& excludedRegion : this->excludedMemoryRegions) {
            const auto& startAddress = excludedRegion.addressRange.startAddress;
            const auto& endAddress = excludedRegion.addressRange.endAddress;

            // Sanity check
            assert(byteItemsByAddress.contains(startAddress) && byteItemsByAddress.contains(endAddress));

            for (auto address = startAddress; address <= endAddress; ++address) {
                auto& byteItem = byteItemsByAddress.at(address);
                byteItem.excluded = true;
            }
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
