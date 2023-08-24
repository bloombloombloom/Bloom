#include "DifferentialItemGraphicsScene.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/ItemGraphicsView.hpp"

namespace Widgets
{
    DifferentialItemGraphicsScene::DifferentialItemGraphicsScene(
        DifferentialHexViewerWidgetType differentialHexViewerWidgetType,
        DifferentialHexViewerSharedState& state,
        const SnapshotDiffSettings& snapshotDiffSettings,
        const Targets::TargetMemoryDescriptor& targetMemoryDescriptor,
        const std::optional<Targets::TargetMemoryBuffer>& data,
        const std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
        const std::vector<ExcludedMemoryRegion>& excludedMemoryRegions,
        HexViewerWidgetSettings& settings,
        QGraphicsView* parent
    )
        : ItemGraphicsScene(
            targetMemoryDescriptor,
            data,
            focusedMemoryRegions,
            excludedMemoryRegions,
            settings,
            parent
        )
        , differentialHexViewerWidgetType(differentialHexViewerWidgetType)
        , diffHexViewerState(state)
        , snapshotDiffSettings(snapshotDiffSettings)
    {}

    void DifferentialItemGraphicsScene::setOther(DifferentialItemGraphicsScene* other) {
        this->other = other;

        QObject::connect(
            this->other,
            &DifferentialItemGraphicsScene::hoveredAddress,
            this,
            &DifferentialItemGraphicsScene::onOtherHoveredAddress
        );

        QObject::connect(
            this->other,
            &DifferentialItemGraphicsScene::selectionChanged,
            this,
            &DifferentialItemGraphicsScene::onOtherSelectionChanged
        );

        QObject::connect(
            this->other,
            &DifferentialItemGraphicsScene::highlightingChanged,
            this,
            &DifferentialItemGraphicsScene::onOtherHighlightedByteRangesChanged
        );
    }

    ByteItem* DifferentialItemGraphicsScene::byteItemAtViewportTop() {
        return this->itemIndex->closestByteItem(this->getScrollbarValue());
    }

    void DifferentialItemGraphicsScene::updateByteItemChangedStates() {
        for (auto& [address, byteItem] : this->topLevelGroup->byteItemsByAddress) {
            byteItem.changed = !byteItem.excluded && this->diffHexViewerState.differences.contains(address);
        }

        this->update();
    }

    QMargins DifferentialItemGraphicsScene::margins() {
        auto margins = ItemGraphicsScene::margins();
        margins.setRight(DifferentialItemGraphicsScene::CENTER_WIDTH / 2);
        return margins;
    }

    void DifferentialItemGraphicsScene::onOtherHoveredAddress(
        const std::optional<Targets::TargetMemoryAddress>& address
    ) {
        if (!this->snapshotDiffSettings.syncHexViewerHover || this->diffHexViewerState.syncingHover) {
            return;
        }

        if (!address.has_value()) {
            if (this->state.hoveredByteItem != nullptr) {
                this->onByteItemLeave();
            }

            return;
        }

        auto& byteItem = this->topLevelGroup->byteItemsByAddress.find(*address)->second;
        const auto itemPosition = byteItem.position().y();
        const auto scrollbarValue = this->getScrollbarValue();

        if (
            itemPosition < scrollbarValue
            || itemPosition > (scrollbarValue + this->views().first()->viewport()->height())
        ) {
            // The item isn't visible
            return;
        }

        this->diffHexViewerState.syncingHover = true;
        this->onByteItemEnter(byteItem);
        this->diffHexViewerState.syncingHover = false;
    }

    void DifferentialItemGraphicsScene::onOtherSelectionChanged(
        const std::set<Targets::TargetMemoryAddress>& addresses
    ) {
        if (!this->snapshotDiffSettings.syncHexViewerSelection || this->diffHexViewerState.syncingSelection) {
            return;
        }

        this->diffHexViewerState.syncingSelection = true;
        this->selectByteItems(addresses);
        this->diffHexViewerState.syncingSelection = false;
    }

    void DifferentialItemGraphicsScene::onOtherHighlightedByteRangesChanged(
        const std::set<Targets::TargetMemoryAddressRange>& addressRanges
    ) {
        if (this->diffHexViewerState.syncingHighlightedRanges) {
            return;
        }

        this->diffHexViewerState.syncingHighlightedRanges = true;
        this->highlightByteItemRanges(addressRanges);
        this->diffHexViewerState.syncingHighlightedRanges = false;

        this->update();
    }
}
