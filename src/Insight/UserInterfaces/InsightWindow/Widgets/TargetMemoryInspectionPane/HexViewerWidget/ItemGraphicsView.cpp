#include "ItemGraphicsView.hpp"

#include <QLayout>

#include "ByteItem.hpp"

namespace Widgets
{
    using Targets::TargetMemoryDescriptor;

    ItemGraphicsView::ItemGraphicsView(
        const TargetMemoryDescriptor& targetMemoryDescriptor,
        const std::optional<Targets::TargetMemoryBuffer>& data,
        const std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
        const std::vector<ExcludedMemoryRegion>& excludedMemoryRegions,
        HexViewerWidgetSettings& settings,
        QWidget* parent
    )
        : QGraphicsView(parent)
        , targetMemoryDescriptor(targetMemoryDescriptor)
        , data(data)
        , focusedMemoryRegions(focusedMemoryRegions)
        , excludedMemoryRegions(excludedMemoryRegions)
        , settings(settings)
    {
        this->setObjectName("graphics-view");
        this->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
        this->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
        this->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        this->setViewportUpdateMode(QGraphicsView::ViewportUpdateMode::MinimalViewportUpdate);
        this->setOptimizationFlag(QGraphicsView::DontSavePainterState, true);
        this->setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing, true);
        this->setCacheMode(QGraphicsView::CacheModeFlag::CacheNone);
        this->setFocusPolicy(Qt::StrongFocus);

        this->verticalScrollBar()->setSingleStep((ByteItem::HEIGHT + (ByteItem::BOTTOM_MARGIN / 2)));
        this->setViewportMargins(-1, 0, -2, 0);
        this->setFrameShape(QFrame::NoFrame);
    }

    void ItemGraphicsView::initScene() {
        this->scene = new ItemGraphicsScene(
            this->targetMemoryDescriptor,
            this->data,
            this->focusedMemoryRegions,
            this->excludedMemoryRegions,
            this->settings,
            this
        );

        this->setScene(this->scene);

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
        if (this->scene == nullptr) {
            return;
        }

        this->centerOn(this->scene->getByteItemPositionByAddress(address));
    }

    bool ItemGraphicsView::event(QEvent* event) {
        const auto eventType = event->type();
        if (this->scene != nullptr && eventType == QEvent::Type::EnabledChange) {
            this->scene->setEnabled(this->isEnabled());
        }

        return QGraphicsView::event(event);
    }

    void ItemGraphicsView::resizeEvent(QResizeEvent* event) {
        QGraphicsView::resizeEvent(event);

        if (this->scene == nullptr) {
            return;
        }

        this->scene->adjustSize();
    }
}
