#include "DifferentialItemGraphicsScene.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/ItemGraphicsView.hpp"

namespace Bloom::Widgets
{
    DifferentialItemGraphicsScene::DifferentialItemGraphicsScene(
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
    }

    ByteItem* DifferentialItemGraphicsScene::byteItemAtViewportVerticalCenter() {
        const auto scrollBarValue = this->getScrollbarValue();

        const auto midPosition = static_cast<qreal>(
            scrollBarValue
        );

        const auto gridPointIndex = static_cast<decltype(this->gridPoints)::size_type>(
            std::min(
                static_cast<int>(
                    std::floor(static_cast<float>(midPosition) / static_cast<float>(ItemGraphicsScene::GRID_SIZE)) + 1
                ),
                static_cast<int>(this->gridPoints.size() - 1)
            )
        );

        return static_cast<ByteItem*>(*(this->gridPoints[gridPointIndex]));
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
            byteItem.allocatedGraphicsItem == nullptr
            || itemPosition < scrollbarValue
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
        const std::unordered_map<Targets::TargetMemoryAddress, ByteItem*>& selectedByteItemsByAddress
    ) {
        if (!this->snapshotDiffSettings.syncHexViewerSelection || this->diffHexViewerState.syncingSelection) {
            return;
        }

        this->diffHexViewerState.syncingSelection = true;
        this->clearByteItemSelection();

        for (const auto& [address, otherByteItem] : selectedByteItemsByAddress) {
            auto& byteItem = this->topLevelGroup->byteItemsByAddress.at(address);
            byteItem.selected = true;
            this->selectedByteItemsByAddress.insert(std::pair(byteItem.startAddress, &byteItem));
        }

        emit this->selectionChanged(this->selectedByteItemsByAddress);
        this->diffHexViewerState.syncingSelection = false;

        this->update();
    }
}
