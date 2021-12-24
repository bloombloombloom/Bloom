#include "HexViewerWidget.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/UiLoader.hpp"

#include "src/Helpers/Paths.hpp"
#include "src/Exceptions/Exception.hpp"

using namespace Bloom::Widgets;
using namespace Bloom::Exceptions;

using Bloom::Targets::TargetMemoryDescriptor;

HexViewerWidget::HexViewerWidget(
    const TargetMemoryDescriptor& targetMemoryDescriptor,
    std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
    std::vector<ExcludedMemoryRegion>& excludedMemoryRegions,
    InsightWorker& insightWorker,
    QWidget* parent
):
    QWidget(parent),
    targetMemoryDescriptor(targetMemoryDescriptor),
    focusedMemoryRegions(focusedMemoryRegions),
    excludedMemoryRegions(excludedMemoryRegions),
    insightWorker(insightWorker)
{
    this->setObjectName("hex-viewer-widget");
    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    auto widgetUiFile = QFile(
        QString::fromStdString(Paths::compiledResourcesPath()
            + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget"
            + "/UiFiles/HexViewerWidget.ui"
        )
    );

    if (!widgetUiFile.open(QFile::ReadOnly)) {
        throw Exception("Failed to open HexViewerWidget UI file");
    }

    auto uiLoader = UiLoader(this);
    this->container = uiLoader.load(&widgetUiFile, this);
    this->container->setFixedSize(this->size());
    this->container->setContentsMargins(0, 0, 0, 0);

    this->toolBar = this->container->findChild<QWidget*>("tool-bar");
    this->bottomBar = this->container->findChild<QWidget*>("bottom-bar");

    this->refreshButton = this->toolBar->findChild<SvgToolButton*>("refresh-memory-btn");
    this->highlightStackMemoryButton = this->toolBar->findChild<SvgToolButton*>("highlight-stack-memory-btn");
    this->highlightHoveredRowAndColumnButton = this->toolBar->findChild<SvgToolButton*>(
        "highlight-hovered-rows-columns-btn"
    );
    this->highlightFocusedMemoryButton = this->container->findChild<SvgToolButton*>(
        "highlight-focused-memory-btn"
    );

    this->toolBar->setContentsMargins(0, 0, 0, 0);
    this->toolBar->layout()->setContentsMargins(5, 0, 5, 1);

    this->bottomBar->setContentsMargins(0, 0, 0, 0);
    this->bottomBar->layout()->setContentsMargins(5, 0, 5, 0);

    this->hoveredAddressLabel = this->bottomBar->findChild<QLabel*>("byte-address-label");

    this->byteItemGraphicsViewContainer = this->container->findChild<QWidget*>("graphics-view-container");
    this->byteItemGraphicsView = new ByteItemContainerGraphicsView(
        this->targetMemoryDescriptor,
        this->focusedMemoryRegions,
        this->excludedMemoryRegions,
        this->insightWorker,
        this->settings,
        this->hoveredAddressLabel,
        this->byteItemGraphicsViewContainer
    );
    this->byteItemGraphicsScene = this->byteItemGraphicsView->getScene();

    this->setStackMemoryHighlightingEnabled(true);
    this->setHoveredRowAndColumnHighlightingEnabled(true);
    this->setFocusedMemoryHighlightingEnabled(true);

    QObject::connect(
        this->highlightStackMemoryButton,
        &QToolButton::clicked,
        this,
        [this] {
            this->setStackMemoryHighlightingEnabled(!this->settings.highlightStackMemory);
        }
    );

    QObject::connect(
        this->highlightHoveredRowAndColumnButton,
        &QToolButton::clicked,
        this,
        [this] {
            this->setHoveredRowAndColumnHighlightingEnabled(!this->settings.highlightHoveredRowAndCol);
        }
    );

    QObject::connect(
        this->highlightFocusedMemoryButton,
        &QToolButton::clicked,
        this,
        [this] {
            this->setFocusedMemoryHighlightingEnabled(!this->settings.highlightFocusedMemory);
        }
    );

    QObject::connect(
        &insightWorker,
        &InsightWorker::targetStateUpdated,
        this,
        &HexViewerWidget::onTargetStateChanged
    );

    this->show();
}

void HexViewerWidget::updateValues(const Targets::TargetMemoryBuffer& buffer) {
    this->byteItemGraphicsScene->updateValues(buffer);
}

void HexViewerWidget::refreshRegions() {
    this->byteItemGraphicsScene->refreshRegions();
}

void HexViewerWidget::setStackPointer(std::uint32_t stackPointer) {
    this->settings.stackPointerAddress = stackPointer;
    this->byteItemGraphicsScene->invalidateChildItemCaches();
}

void HexViewerWidget::resizeEvent(QResizeEvent* event) {
    this->container->setFixedSize(
        this->width(),
        this->height()
    );

    this->byteItemGraphicsView->setFixedSize(this->byteItemGraphicsViewContainer->size());
}

void HexViewerWidget::showEvent(QShowEvent* event) {
    this->byteItemGraphicsView->setFixedSize(this->byteItemGraphicsViewContainer->size());
}

void HexViewerWidget::onTargetStateChanged(Targets::TargetState newState) {
    using Targets::TargetState;
    this->targetState = newState;
}

void HexViewerWidget::setStackMemoryHighlightingEnabled(bool enabled) {
    this->highlightStackMemoryButton->setChecked(enabled);
    this->settings.highlightStackMemory = enabled;

    this->byteItemGraphicsScene->invalidateChildItemCaches();
}

void HexViewerWidget::setHoveredRowAndColumnHighlightingEnabled(bool enabled) {
    this->highlightHoveredRowAndColumnButton->setChecked(enabled);
    this->settings.highlightHoveredRowAndCol = enabled;

    this->byteItemGraphicsScene->invalidateChildItemCaches();
}

void HexViewerWidget::setFocusedMemoryHighlightingEnabled(bool enabled) {
    this->highlightFocusedMemoryButton->setChecked(enabled);
    this->settings.highlightFocusedMemory = enabled;

    this->byteItemGraphicsScene->invalidateChildItemCaches();
}
