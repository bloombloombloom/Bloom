#include "ItemGraphicsView.hpp"

namespace Bloom::Widgets
{
    using Bloom::Targets::TargetMemoryDescriptor;

    ItemGraphicsView::ItemGraphicsView(
        const TargetMemoryDescriptor& targetMemoryDescriptor,
        const std::optional<Targets::TargetMemoryBuffer>& data,
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
        this->setViewportUpdateMode(QGraphicsView::NoViewportUpdate);
        this->setOptimizationFlag(QGraphicsView::DontSavePainterState, true);
        this->setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing, true);
        this->setCacheMode(QGraphicsView::CacheModeFlag::CacheNone);
        this->setFocusPolicy(Qt::StrongFocus);

        this->scene = new ItemGraphicsScene(
            targetMemoryDescriptor,
            data,
            focusedMemoryRegions,
            excludedMemoryRegions,
            settings,
            hoveredAddressLabel,
            this
        );

        this->setScene(this->scene);
    }

    void ItemGraphicsView::initScene() {
        QObject::connect(
            this->scene,
            &ItemGraphicsScene::ready,
            this,
            [this] {
                this->scene->setEnabled(this->isEnabled());
                emit this->sceneReady();
            }
        );

        this->scene->init();
    }

    void ItemGraphicsView::scrollToByteItemAtAddress(Targets::TargetMemoryAddress address) {
        if (this->scene != nullptr) {
            this->centerOn(this->scene->getByteItemPositionByAddress(address));
        }
    }

    bool ItemGraphicsView::event(QEvent* event) {
        const auto eventType = event->type();
        if (eventType == QEvent::Type::EnabledChange && this->scene != nullptr) {
            this->scene->setEnabled(this->isEnabled());
        }

        return QGraphicsView::event(event);
    }

    void ItemGraphicsView::resizeEvent(QResizeEvent* event) {
        QGraphicsView::resizeEvent(event);

        if (this->scene != nullptr) {
            this->scene->adjustSize();
        }
    }

    void ItemGraphicsView::scrollContentsBy(int dx, int dy) {
        this->scene->allocateGraphicsItems();
        return QGraphicsView::scrollContentsBy(dx, dy);
    }
}
