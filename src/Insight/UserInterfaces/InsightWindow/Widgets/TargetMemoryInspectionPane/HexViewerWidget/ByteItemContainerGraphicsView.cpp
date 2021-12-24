#include "ByteItemContainerGraphicsView.hpp"

using namespace Bloom::Widgets;

using Bloom::Targets::TargetMemoryDescriptor;

ByteItemContainerGraphicsView::ByteItemContainerGraphicsView(
    const TargetMemoryDescriptor& targetMemoryDescriptor,
    std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
    std::vector<ExcludedMemoryRegion>& excludedMemoryRegions,
    InsightWorker& insightWorker,
    const HexViewerWidgetSettings& settings,
    QLabel* hoveredAddressLabel,
    QWidget* parent
): QGraphicsView(parent) {
    this->setObjectName("graphics-view");
    this->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    this->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);

    this->scene = new ByteItemGraphicsScene(
        targetMemoryDescriptor,
        focusedMemoryRegions,
        excludedMemoryRegions,
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
    QGraphicsView::resizeEvent(event);
    this->scene->adjustSize();
}
