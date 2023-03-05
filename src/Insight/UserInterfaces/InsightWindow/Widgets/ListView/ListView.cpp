#include "ListView.hpp"

namespace Bloom::Widgets
{
    ListView::ListView(
        const std::vector<ListItem*>& items,
        QWidget* parent
    )
        : QGraphicsView(parent)
    {
        this->setObjectName("list-graphics-view");
        this->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
        this->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
        this->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        this->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
        this->setOptimizationFlag(QGraphicsView::DontSavePainterState, true);
        this->setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing, true);
        this->setCacheMode(QGraphicsView::CacheModeFlag::CacheNone);
        this->setFocusPolicy(Qt::StrongFocus);

        this->scene = new ListScene(std::move(items), this);
        this->setScene(this->scene);
    }

    bool ListView::event(QEvent* event) {
        const auto eventType = event->type();
        if (eventType == QEvent::Type::EnabledChange) {
            this->scene->setEnabled(this->isEnabled());
        }

        return QGraphicsView::event(event);
    }

    void ListView::resizeEvent(QResizeEvent* event) {
        QGraphicsView::resizeEvent(event);

        this->scene->refreshGeometry();
    }
}
