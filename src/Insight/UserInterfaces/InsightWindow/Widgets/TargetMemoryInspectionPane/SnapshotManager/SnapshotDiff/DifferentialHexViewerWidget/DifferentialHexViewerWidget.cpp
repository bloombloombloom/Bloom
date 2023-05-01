#include "DifferentialHexViewerWidget.hpp"

#include <QVBoxLayout>
#include <cassert>

namespace Bloom::Widgets
{
    DifferentialHexViewerWidget::DifferentialHexViewerWidget(
        DifferentialHexViewerWidgetType type,
        DifferentialHexViewerSharedState& state,
        const SnapshotDiffSettings& snapshotDiffSettings,
        const Targets::TargetMemoryDescriptor& targetMemoryDescriptor,
        const std::optional<Targets::TargetMemoryBuffer>& data,
        HexViewerWidgetSettings& settings,
        const std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
        const std::vector<ExcludedMemoryRegion>& excludedMemoryRegions,
        QWidget* parent
    )
        : HexViewerWidget(
            targetMemoryDescriptor,
            data,
            settings,
            focusedMemoryRegions,
            excludedMemoryRegions,
            parent
        )
        , type(type)
        , state(state)
        , snapshotDiffSettings(snapshotDiffSettings)
    {}

    void DifferentialHexViewerWidget::init() {
        this->differentialView = new DifferentialItemGraphicsView(
            this->state,
            this->snapshotDiffSettings,
            this->targetMemoryDescriptor,
            this->data,
            this->focusedMemoryRegions,
            this->excludedMemoryRegions,
            this->settings,
            this->container
        );

        if (this->type == DifferentialHexViewerWidgetType::PRIMARY) {
            this->differentialView->setLayoutDirection(Qt::LayoutDirection::RightToLeft);
        }

        this->byteItemGraphicsView = this->differentialView;
        this->byteItemGraphicsView->hide();

        auto* containerLayout = this->container->findChild<QVBoxLayout*>("hex-viewer-layout");
        containerLayout->insertWidget(2, this->byteItemGraphicsView);

        QObject::connect(
            this->byteItemGraphicsView,
            &ItemGraphicsView::sceneReady,
            this,
            [this] {
                this->differentialScene = dynamic_cast<DifferentialItemGraphicsScene*>(
                    this->byteItemGraphicsView->getScene()
                );
                this->byteItemGraphicsScene = this->differentialScene;

                QObject::connect(
                    this->byteItemGraphicsScene,
                    &ItemGraphicsScene::hoveredAddress,
                    this,
                    &DifferentialHexViewerWidget::onHoveredAddress
                );

                QObject::connect(
                    this->byteItemGraphicsScene,
                    &ItemGraphicsScene::selectionChanged,
                    this,
                    &DifferentialHexViewerWidget::onByteSelectionChanged
                );

                this->loadingHexViewerLabel->hide();
                this->byteItemGraphicsView->show();

                emit this->ready();
            }
        );

        this->byteItemGraphicsView->initScene();
    }

    void DifferentialHexViewerWidget::setOther(DifferentialHexViewerWidget* other) {
        assert(other->byteItemGraphicsView != nullptr);
        assert(other->byteItemGraphicsScene != nullptr);

        this->other = other;
        this->differentialView->setOther(this->other->differentialView);

        QObject::connect(
            this->other,
            &HexViewerWidget::settingsChanged,
            this,
            &DifferentialHexViewerWidget::onOtherSettingsChanged
        );
    }

    void DifferentialHexViewerWidget::onOtherSettingsChanged(const HexViewerWidgetSettings& settings) {
        if (!this->snapshotDiffSettings.syncHexViewerSettings || this->state.syncingSettings) {
            return;
        }

        this->state.syncingSettings = true;

        this->setStackMemoryGroupingEnabled(settings.groupStackMemory);
        this->setFocusedMemoryHighlightingEnabled(settings.highlightFocusedMemory);
        this->setHoveredRowAndColumnHighlightingEnabled(settings.highlightHoveredRowAndCol);
        this->setDisplayAsciiEnabled(settings.displayAsciiValues);
        this->setAnnotationsEnabled(settings.displayAnnotations);

        this->state.syncingSettings = false;

        this->differentialView->update();
    }
}
