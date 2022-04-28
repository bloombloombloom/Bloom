#include "TargetMemoryInspectionPane.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>

#include "src/Insight/UserInterfaces/InsightWindow/UiLoader.hpp"

#include "src/Insight/InsightWorker/Tasks/ReadTargetMemory.hpp"
#include "src/Insight/InsightWorker/Tasks/ReadStackPointer.hpp"

#include "src/Helpers/Paths.hpp"
#include "src/Exceptions/Exception.hpp"

using namespace Bloom::Widgets;
using namespace Bloom::Exceptions;

using Bloom::Targets::TargetMemoryDescriptor;
using Bloom::Targets::TargetMemoryType;

TargetMemoryInspectionPane::TargetMemoryInspectionPane(
    const TargetMemoryDescriptor& targetMemoryDescriptor,
    TargetMemoryInspectionPaneSettings& settings,
    InsightWorker& insightWorker,
    PanelWidget* parent
)
    : PaneWidget(parent)
    , targetMemoryDescriptor(targetMemoryDescriptor)
    , settings(settings)
    , insightWorker(insightWorker)
{
    this->setObjectName("target-memory-inspection-pane");

    auto memoryInspectionPaneUiFile = QFile(
        QString::fromStdString(Paths::compiledResourcesPath()
            + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/UiFiles/TargetMemoryInspectionPane.ui"
        )
    );

    if (!memoryInspectionPaneUiFile.open(QFile::ReadOnly)) {
        throw Exception("Failed to open MemoryInspectionPane UI file");
    }

    auto uiLoader = UiLoader(this);
    this->container = uiLoader.load(&memoryInspectionPaneUiFile, this);
    this->container->setFixedSize(parent->width(), parent->maximumHeight());

    this->titleBar = this->container->findChild<QWidget*>("title-bar");

    this->titleBar->layout()->setContentsMargins(7, 0, 7, 0);
    auto* titleLabel = this->titleBar->findChild<QLabel*>("title");
    titleLabel->setText(
        this->targetMemoryDescriptor.type == TargetMemoryType::EEPROM ? "Internal EEPROM" : "Internal RAM"
    );

    // Quick sanity check to ensure the validity of persisted settings.
    this->sanitiseSettings();

    auto* subContainerLayout = this->container->findChild<QHBoxLayout*>("sub-container-layout");
    this->manageMemoryRegionsButton = this->container->findChild<SvgToolButton*>("manage-memory-regions-btn");
    this->hexViewerWidget = new HexViewerWidget(
        this->targetMemoryDescriptor,
        this->settings.hexViewerWidgetSettings,
        this->settings.focusedMemoryRegions,
        this->settings.excludedMemoryRegions,
        this->insightWorker,
        this
    );
    this->hexViewerWidget->setDisabled(true);

    subContainerLayout->addWidget(this->hexViewerWidget);

    QObject::connect(
        this->manageMemoryRegionsButton,
        &QToolButton::clicked,
        this,
        &TargetMemoryInspectionPane::openMemoryRegionManagerWindow
    );

    QObject::connect(
        &insightWorker,
        &InsightWorker::targetStateUpdated,
        this,
        &TargetMemoryInspectionPane::onTargetStateChanged
    );

    QObject::connect(
        this->hexViewerWidget->refreshButton,
        &QToolButton::clicked,
        this,
        [this] {
            this->refreshMemoryValues();
        }
    );
}

void TargetMemoryInspectionPane::refreshMemoryValues(std::optional<std::function<void(void)>> callback) {
    this->hexViewerWidget->refreshButton->setDisabled(true);
    this->hexViewerWidget->refreshButton->startSpin();

    auto excludedAddressRanges = std::set<Targets::TargetMemoryAddressRange>();
    std::transform(
        this->settings.excludedMemoryRegions.begin(),
        this->settings.excludedMemoryRegions.end(),
        std::inserter(excludedAddressRanges, excludedAddressRanges.begin()),
        [this] (const ExcludedMemoryRegion& excludedRegion) {
            return excludedRegion.addressRange;
        }
    );

    auto* readMemoryTask = new ReadTargetMemory(
        this->targetMemoryDescriptor.type,
        this->targetMemoryDescriptor.addressRange.startAddress,
        this->targetMemoryDescriptor.size(),
        excludedAddressRanges
    );

    QObject::connect(
        readMemoryTask,
        &ReadTargetMemory::targetMemoryRead,
        this,
        [this] (const Targets::TargetMemoryBuffer& buffer) {
            this->onMemoryRead(buffer);

            // Refresh the stack pointer if this is RAM.
            if (this->targetMemoryDescriptor.type == Targets::TargetMemoryType::RAM) {
                auto* readStackPointerTask = new ReadStackPointer();
                QObject::connect(
                    readStackPointerTask,
                    &ReadStackPointer::stackPointerRead,
                    this,
                    [this] (std::uint32_t stackPointer) {
                        this->hexViewerWidget->setStackPointer(stackPointer);
                    }
                );

                this->insightWorker.queueTask(readStackPointerTask);
            }
        }
    );

    QObject::connect(
        readMemoryTask,
        &InsightWorkerTask::finished,
        this,
        [this] {
            this->hexViewerWidget->refreshButton->stopSpin();

            if (this->targetState == Targets::TargetState::STOPPED) {
                this->hexViewerWidget->refreshButton->setDisabled(false);
            }
        }
    );

    if (callback.has_value()) {
        QObject::connect(
            readMemoryTask,
            &InsightWorkerTask::completed,
            this,
            callback.value()
        );
    }

    this->insightWorker.queueTask(readMemoryTask);
}

void TargetMemoryInspectionPane::activate() {
    this->show();
    this->activated = true;
    this->postActivate();
}

void TargetMemoryInspectionPane::deactivate() {
    this->hide();
    this->activated = false;
    this->postDeactivate();
}

void TargetMemoryInspectionPane::resizeEvent(QResizeEvent* event) {
    const auto parentSize = this->parentPanel->size();
    const auto width = parentSize.width() - 1;
    this->container->setFixedSize(width, parentSize.height());
}

void TargetMemoryInspectionPane::postActivate() {
    if (this->targetState == Targets::TargetState::STOPPED) {
        this->refreshMemoryValues([this] {
            this->hexViewerWidget->setDisabled(false);
        });
    }
}

void TargetMemoryInspectionPane::postDeactivate() {

}

void TargetMemoryInspectionPane::sanitiseSettings() {
    // Remove any invalid memory regions. It's very unlikely that there will be any, but not impossible.
    auto processedFocusedMemoryRegions = std::vector<FocusedMemoryRegion>();
    auto processedExcludedMemoryRegions = std::vector<ExcludedMemoryRegion>();

    const auto regionIntersects = [
        &processedFocusedMemoryRegions,
        &processedExcludedMemoryRegions
    ] (const MemoryRegion& region) {
        for (const auto& processedFocusedRegion : processedFocusedMemoryRegions) {
            if (processedFocusedRegion.intersectsWith(region)) {
                return true;
            }
        }

        for (const auto& processedExcludedRegion : processedExcludedMemoryRegions) {
            if (processedExcludedRegion.intersectsWith(region)) {
                return true;
            }
        }

        return false;
    };

    for (const auto& focusedRegion : this->settings.focusedMemoryRegions) {
        if (!this->targetMemoryDescriptor.addressRange.contains(focusedRegion.addressRange)
            || regionIntersects(focusedRegion)
        ) {
            continue;
        }

        processedFocusedMemoryRegions.emplace_back(focusedRegion);
    }

    for (const auto& excludedRegion : this->settings.excludedMemoryRegions) {
        if (!this->targetMemoryDescriptor.addressRange.contains(excludedRegion.addressRange)
            || regionIntersects(excludedRegion)
        ) {
            continue;
        }

        processedExcludedMemoryRegions.emplace_back(excludedRegion);
    }

    this->settings.focusedMemoryRegions = std::move(processedFocusedMemoryRegions);
    this->settings.excludedMemoryRegions = std::move(processedExcludedMemoryRegions);
}

void TargetMemoryInspectionPane::onTargetStateChanged(Targets::TargetState newState) {
    if (this->targetState == newState) {
        return;
    }

    using Targets::TargetState;
    this->targetState = newState;

    if (newState == TargetState::STOPPED && this->activated) {
        this->refreshMemoryValues([this] {
            this->hexViewerWidget->setDisabled(false);
        });

    } else if (newState == TargetState::RUNNING) {
        this->hexViewerWidget->setDisabled(true);
    }
}

void TargetMemoryInspectionPane::onMemoryRead(const Targets::TargetMemoryBuffer& buffer) {
    this->hexViewerWidget->updateValues(buffer);
}

void TargetMemoryInspectionPane::openMemoryRegionManagerWindow() {
    if (this->memoryRegionManagerWindow == nullptr) {
        this->memoryRegionManagerWindow = new MemoryRegionManagerWindow(
            this->targetMemoryDescriptor,
            this->settings.focusedMemoryRegions,
            this->settings.excludedMemoryRegions,
            this

        );

        QObject::connect(
            this->memoryRegionManagerWindow,
            &MemoryRegionManagerWindow::changesApplied,
            this,
            &TargetMemoryInspectionPane::onMemoryRegionsChange
        );
    }

    if (!this->memoryRegionManagerWindow->isVisible()) {
        this->memoryRegionManagerWindow->refreshRegions();
        this->memoryRegionManagerWindow->show();

    } else {
        this->memoryRegionManagerWindow->activateWindow();
    }
}

void TargetMemoryInspectionPane::onMemoryRegionsChange() {
    this->hexViewerWidget->refreshRegions();
}
