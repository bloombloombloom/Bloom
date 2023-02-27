#include "ConstructHexViewerTopLevelGroupItem.hpp"

namespace Bloom
{
    ConstructHexViewerTopLevelGroupItem::ConstructHexViewerTopLevelGroupItem(
        const std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
        const Widgets::HexViewerSharedState& hexViewerState
    )
        : focusedMemoryRegions(focusedMemoryRegions)
        , hexViewerState(hexViewerState)
    {}

    void ConstructHexViewerTopLevelGroupItem::run(Services::TargetControllerService&) {
        auto* item = new Widgets::TopLevelGroupItem(this->focusedMemoryRegions, this->hexViewerState);
        item->rebuildItemHierarchy();

        emit this->topLevelGroupItem(item);
    }
}
