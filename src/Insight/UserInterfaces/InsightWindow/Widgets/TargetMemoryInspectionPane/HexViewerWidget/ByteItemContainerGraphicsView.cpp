#include "ByteItemContainerGraphicsView.hpp"

#include <QVBoxLayout>
#include <QTableWidget>
#include <QScrollBar>
#include <QPainter>
#include <cmath>

#include "src/Logger/Logger.hpp"

using namespace Bloom::Widgets;
using namespace Bloom::Exceptions;

using Bloom::Targets::TargetMemoryDescriptor;

ByteItemContainerGraphicsView::ByteItemContainerGraphicsView(
    const TargetMemoryDescriptor& targetMemoryDescriptor,
    InsightWorker& insightWorker,
    const HexViewerWidgetSettings& settings,
    QLabel* hoveredAddressLabel,
    QWidget* parent
): QGraphicsView(parent) {
    this->setObjectName("graphics-view");
    this->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    this->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);

    this->scene = new ByteItemGraphicsScene(
        targetMemoryDescriptor,
        insightWorker,
        settings,
        hoveredAddressLabel,
        this
    );

    this->setScene(this->scene);

}

bool ByteItemContainerGraphicsView::event(QEvent* event) {
    const auto eventType = event->type();
    if (eventType == QEvent::Type::EnabledChange) {
        this->scene->setEnabled(this->isEnabled());
    }

    return QGraphicsView::event(event);
}

void ByteItemContainerGraphicsView::resizeEvent(QResizeEvent* event) {
    this->scene->adjustByteWidgets();
    QGraphicsView::resizeEvent(event);
}
