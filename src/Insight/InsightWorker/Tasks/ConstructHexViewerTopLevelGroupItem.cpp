#include "ConstructHexViewerTopLevelGroupItem.hpp"

ConstructHexViewerTopLevelGroupItem::ConstructHexViewerTopLevelGroupItem(
    const std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
    const std::vector<ExcludedMemoryRegion>& excludedMemoryRegions,
    const Widgets::HexViewerSharedState& hexViewerState
)
    : focusedMemoryRegions(focusedMemoryRegions)
    , excludedMemoryRegions(excludedMemoryRegions)
    , hexViewerState(hexViewerState)
{}

QString ConstructHexViewerTopLevelGroupItem::brief() const {
    return "Preparing hex viewer";
}

void ConstructHexViewerTopLevelGroupItem::run(Services::TargetControllerService&) {
    auto* item = new Widgets::TopLevelGroupItem(
        this->focusedMemoryRegions,
        this->excludedMemoryRegions,
        this->hexViewerState
    );
    item->rebuildItemHierarchy();

    emit this->topLevelGroupItem(item);
}
