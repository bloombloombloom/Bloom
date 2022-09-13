#include "ByteItemContainerGraphicsView.hpp"

#include "src/Insight/InsightWorker/InsightWorker.hpp"
#include "src/Insight/InsightWorker/Tasks/ConstructHexViewerByteItems.hpp"

namespace Bloom::Widgets
{
    using Bloom::Targets::TargetMemoryDescriptor;

    ByteItemContainerGraphicsView::ByteItemContainerGraphicsView(
        const TargetMemoryDescriptor& targetMemoryDescriptor,
        std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
        std::vector<ExcludedMemoryRegion>& excludedMemoryRegions,
        HexViewerWidgetSettings& settings,
        Label* hoveredAddressLabel,
        QWidget* parent
    )
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

        this->scene = new ByteItemGraphicsScene(
            targetMemoryDescriptor,
            focusedMemoryRegions,
            excludedMemoryRegions,
            settings,
            hoveredAddressLabel,
            this
        );

        this->setScene(this->scene);
    }

    void ByteItemContainerGraphicsView::initScene() {
        QObject::connect(
            this->scene,
            &ByteItemGraphicsScene::ready,
            this,
            [this] {
                this->scene->setEnabled(this->isEnabled());
                emit this->sceneReady();
            }
        );

        this->scene->init();
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
