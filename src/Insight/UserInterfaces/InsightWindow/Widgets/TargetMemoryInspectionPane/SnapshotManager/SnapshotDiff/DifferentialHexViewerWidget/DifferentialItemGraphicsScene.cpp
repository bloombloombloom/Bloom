#include "DifferentialItemGraphicsScene.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/ItemGraphicsView.hpp"

#include "DifferentialHexViewerItemRenderer.hpp"

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

        assert(this->other->differentialHexViewerItemRenderer != nullptr);

        if (this->differentialHexViewerItemRenderer != nullptr) {
            this->differentialHexViewerItemRenderer->setOther(this->other->differentialHexViewerItemRenderer);
        }

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

    ByteItem* DifferentialItemGraphicsScene::byteItemAtViewportTop() {
        return this->itemIndex->closestByteItem(this->getScrollbarValue());
    }

    void DifferentialItemGraphicsScene::updateByteItemChangedStates() {
        for (auto& [address, byteItem] : this->topLevelGroup->byteItemsByAddress) {
            byteItem.changed = !byteItem.excluded && this->diffHexViewerState.differences.contains(address);
        }

        this->update();
    }

    void DifferentialItemGraphicsScene::initRenderer() {
        this->differentialHexViewerItemRenderer = new DifferentialHexViewerItemRenderer(
            this->differentialHexViewerWidgetType,
            this->state,
            *(this->itemIndex.get()),
            this->views().first()
        );

        if (this->other != nullptr && this->other->differentialHexViewerItemRenderer != nullptr) {
            this->differentialHexViewerItemRenderer->setOther(this->other->differentialHexViewerItemRenderer);
        }

        this->renderer = this->differentialHexViewerItemRenderer;
        this->renderer->setPos(0, 0);
        this->addItem(this->renderer);
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
