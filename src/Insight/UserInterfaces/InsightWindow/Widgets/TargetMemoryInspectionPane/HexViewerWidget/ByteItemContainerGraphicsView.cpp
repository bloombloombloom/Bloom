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
    QLabel* hoveredAddressLabel,
    QWidget* parent
): QGraphicsView(parent) {
    this->setObjectName("graphics-view");
    this->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    this->scene = new ByteItemGraphicsScene(
        targetMemoryDescriptor,
        insightWorker,
        hoveredAddressLabel,
        this
    );

    this->setScene(this->scene);
}

bool ByteItemContainerGraphicsView::event(QEvent* event) {
    if (event->type() == QEvent::Type::EnabledChange) {
        this->scene->setEnabled(this->isEnabled());
    }

    return QGraphicsView::event(event);
}

void ByteItemContainerGraphicsView::resizeEvent(QResizeEvent* event) {
    Logger::warning("Resizing");
    this->scene->adjustByteWidgets();
    QGraphicsView::resizeEvent(event);
}
