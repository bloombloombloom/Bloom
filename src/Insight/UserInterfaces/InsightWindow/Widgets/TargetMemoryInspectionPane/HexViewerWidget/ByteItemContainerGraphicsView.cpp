#include "ByteItemContainerGraphicsView.hpp"

#include "src/Insight/InsightWorker/InsightWorker.hpp"
#include "src/Insight/InsightWorker/Tasks/ConstructHexViewerByteItemScene.hpp"

namespace Bloom::Widgets
{
    using Bloom::Targets::TargetMemoryDescriptor;

    ByteItemContainerGraphicsView::ByteItemContainerGraphicsView(QWidget* parent)
        : QGraphicsView(parent)
    {
        this->setObjectName("graphics-view");
        this->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
        this->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
        this->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        this->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
        this->setOptimizationFlag(QGraphicsView::DontSavePainterState, true);
        this->setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing, true);
        this->setCacheMode(QGraphicsView::CacheBackground);
    }

    void ByteItemContainerGraphicsView::initScene(
        const TargetMemoryDescriptor& targetMemoryDescriptor,
        std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
        std::vector<ExcludedMemoryRegion>& excludedMemoryRegions,
        HexViewerWidgetSettings& settings,
        Label* hoveredAddressLabel
    ) {
        auto* constructSceneTask = new ConstructHexViewerByteItemScene(
            targetMemoryDescriptor,
            focusedMemoryRegions,
            excludedMemoryRegions,
            settings,
            hoveredAddressLabel
        );

        QObject::connect(
            constructSceneTask,
            &ConstructHexViewerByteItemScene::sceneCreated,
            this,
            [this] (ByteItemGraphicsScene* scene) {
                scene->moveToThread(this->thread());
                scene->setParent(this);

                this->scene = scene;
                this->scene->refreshRegions();
                this->scene->setEnabled(this->isEnabled());
                this->setScene(this->scene);

                emit this->ready();
            }
        );

        InsightWorker::queueTask(constructSceneTask);
    }

    void ByteItemContainerGraphicsView::scrollToByteItemAtAddress(Targets::TargetMemoryAddress address) {
        if (this->scene != nullptr) {
            this->centerOn(this->scene->getByteItemPositionByAddress(address));
        }
    }

    bool ByteItemContainerGraphicsView::event(QEvent* event) {
        const auto eventType = event->type();
        if (eventType == QEvent::Type::EnabledChange && this->scene != nullptr) {
            this->scene->setEnabled(this->isEnabled());
        }

        return QGraphicsView::event(event);
    }

    void ByteItemContainerGraphicsView::resizeEvent(QResizeEvent* event) {
        QGraphicsView::resizeEvent(event);

        if (this->scene != nullptr) {
            this->scene->adjustSize();
        }
    }
}
