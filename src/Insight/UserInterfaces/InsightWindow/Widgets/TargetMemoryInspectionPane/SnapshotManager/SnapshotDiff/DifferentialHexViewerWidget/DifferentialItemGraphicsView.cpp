#include "DifferentialItemGraphicsView.hpp"

namespace Widgets
{
    DifferentialItemGraphicsView::DifferentialItemGraphicsView(
        DifferentialHexViewerWidgetType differentialHexViewerWidgetType,
        DifferentialHexViewerSharedState& state,
        const SnapshotDiffSettings& snapshotDiffSettings,
        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        const Targets::TargetState& targetState,
        const std::optional<Targets::TargetMemoryBuffer>& data,
        const std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
        const std::vector<ExcludedMemoryRegion>& excludedMemoryRegions,
        HexViewerWidgetSettings& settings,
        QWidget* parent
    )
        : ItemGraphicsView(
            addressSpaceDescriptor,
            memorySegmentDescriptor,
            targetState,
            data,
            focusedMemoryRegions,
            excludedMemoryRegions,
            settings,
            parent
        )
        , differentialHexViewerWidgetType(differentialHexViewerWidgetType)
        , state(state)
        , snapshotDiffSettings(snapshotDiffSettings)
    {}

    void DifferentialItemGraphicsView::initScene() {
        this->differentialScene = new DifferentialItemGraphicsScene{
            this->differentialHexViewerWidgetType,
            this->state,
            this->snapshotDiffSettings,
            this->addressSpaceDescriptor,
            this->memorySegmentDescriptor,
            this->targetState,
            this->data,
            this->focusedMemoryRegions,
            this->excludedMemoryRegions,
            this->settings,
            this
        };

        this->scene = this->differentialScene;
        this->setScene(this->scene);

        QObject::connect(
        this->scene,
            &ItemGraphicsScene::ready,
            this,
            [this] {
                this->differentialScene->updateByteItemChangedStates();
                this->scene->setEnabled(this->isEnabled());
                emit this->sceneReady();
            }
        );

        this->scene->init();
    }

    void DifferentialItemGraphicsView::setOther(DifferentialItemGraphicsView* other) {
        this->other = other;
        this->differentialScene->setOther(this->other->differentialScene);
    }

    void DifferentialItemGraphicsView::alignScroll(
        Targets::TargetMemoryAddress otherByteItemAddress,
        int otherByteItemYOffset
    ) {
        const auto byteItemPosition = this->differentialScene->getByteItemPositionByAddress(otherByteItemAddress);
        this->verticalScrollBar()->setValue(
            std::max(static_cast<int>(byteItemPosition.y() - otherByteItemYOffset), 0)
        );

        this->update();
    }

    void DifferentialItemGraphicsView::scrollContentsBy(int dx, int dy) {
        ItemGraphicsView::scrollContentsBy(dx, dy);

        if (!this->snapshotDiffSettings.syncHexViewerScroll) {
            this->other->update();
            return;
        }

        if (this->state.syncingScroll) {
            return;
        }

        auto* byteItem = this->differentialScene->byteItemAtViewportTop();

        this->state.syncingScroll = true;
        this->other->alignScroll(byteItem->startAddress, byteItem->position().y() - this->verticalScrollBar()->value());
        this->state.syncingScroll = false;
    }
}
