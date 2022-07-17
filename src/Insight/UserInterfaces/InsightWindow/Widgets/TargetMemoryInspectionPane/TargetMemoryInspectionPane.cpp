#include "TargetMemoryInspectionPane.hpp"

#include <QVBoxLayout>
#include <QToolButton>

#include "src/Insight/UserInterfaces/InsightWindow/UiLoader.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/Label.hpp"

#include "src/Insight/InsightWorker/Tasks/ReadTargetMemory.hpp"
#include "src/Insight/InsightWorker/Tasks/ReadStackPointer.hpp"

#include "src/Helpers/Paths.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Bloom::Widgets
{
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

        const auto memoryName = QString(
            this->targetMemoryDescriptor.type == TargetMemoryType::EEPROM
                ? "Internal EEPROM"
                : "Internal RAM"
        );

        this->setWindowTitle("Memory Inspection - " + memoryName);
        this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

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
        this->container->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        this->titleBar = this->container->findChild<QWidget*>("title-bar");

        this->titleBar->layout()->setContentsMargins(7, 0, 7, 0);
        auto* titleLabel = this->titleBar->findChild<Label*>("title");
        titleLabel->setText(memoryName);

        // Quick sanity check to ensure the validity of persisted settings.
        this->sanitiseSettings();

        auto* containerLayout = this->container->findChild<QVBoxLayout*>("container-layout");
        this->manageMemoryRegionsButton = this->container->findChild<SvgToolButton*>("manage-memory-regions-btn");

        this->refreshButton = this->container->findChild<SvgToolButton*>("refresh-memory-btn");
        this->refreshOnTargetStopAction = this->refreshButton->findChild<QAction*>("refresh-target-stopped");
        this->refreshOnActivationAction = this->refreshButton->findChild<QAction*>("refresh-activation");

        this->detachPaneButton = this->container->findChild<SvgToolButton*>("detach-pane-btn");
        this->attachPaneButton = this->container->findChild<SvgToolButton*>("attach-pane-btn");

        this->hexViewerWidget = new HexViewerWidget(
            this->targetMemoryDescriptor,
            this->settings.hexViewerWidgetSettings,
            this->settings.focusedMemoryRegions,
            this->settings.excludedMemoryRegions,
            this->insightWorker,
            this
        );
        this->hexViewerWidget->setDisabled(true);

        containerLayout->addWidget(this->hexViewerWidget);

        this->setRefreshOnTargetStopEnabled(this->settings.refreshOnTargetStop);

        QObject::connect(
            this,
            &PaneWidget::paneActivated,
            this,
            &TargetMemoryInspectionPane::postActivate
        );

        QObject::connect(
            this,
            &PaneWidget::paneDeactivated,
            this,
            &TargetMemoryInspectionPane::postDeactivate
        );

        QObject::connect(
            this,
            &PaneWidget::paneAttached,
            this,
            &TargetMemoryInspectionPane::postAttach
        );

        QObject::connect(
            this,
            &PaneWidget::paneDetached,
            this,
            &TargetMemoryInspectionPane::postDetach
        );

        QObject::connect(
            this->manageMemoryRegionsButton,
            &QToolButton::clicked,
            this,
            &TargetMemoryInspectionPane::openMemoryRegionManagerWindow
        );

        QObject::connect(
            this->refreshButton,
            &QToolButton::clicked,
            this,
            [this] {
                this->refreshMemoryValues();
            }
        );

        QObject::connect(
            this->refreshOnTargetStopAction,
            &QAction::triggered,
            this,
            [this] (bool checked) {
                this->setRefreshOnTargetStopEnabled(checked);
            }
        );

        QObject::connect(
            this->refreshOnActivationAction,
            &QAction::triggered,
            this,
            [this] (bool checked) {
                this->setRefreshOnActivationEnabled(checked);
            }
        );

        QObject::connect(
            this->detachPaneButton,
            &QToolButton::clicked,
            this,
            &TargetMemoryInspectionPane::detach
        );

        QObject::connect(
            this->attachPaneButton,
            &QToolButton::clicked,
            this,
            &TargetMemoryInspectionPane::attach
        );

        QObject::connect(
            &insightWorker,
            &InsightWorker::targetStateUpdated,
            this,
            &TargetMemoryInspectionPane::onTargetStateChanged
        );

        QObject::connect(
            &insightWorker,
            &InsightWorker::programmingModeEnabled,
            this,
            &TargetMemoryInspectionPane::onProgrammingModeEnabled
        );

        QObject::connect(
            &insightWorker,
            &InsightWorker::programmingModeDisabled,
            this,
            &TargetMemoryInspectionPane::onProgrammingModeDisabled
        );
    }

    void TargetMemoryInspectionPane::refreshMemoryValues(std::optional<std::function<void(void)>> callback) {
        this->refreshButton->setDisabled(true);
        this->refreshButton->startSpin();

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
                this->refreshButton->stopSpin();

                if (this->targetState == Targets::TargetState::STOPPED) {
                    this->refreshButton->setDisabled(false);
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

    void TargetMemoryInspectionPane::resizeEvent(QResizeEvent* event) {
        const auto size = this->size();
        this->container->setFixedSize(size.width() - 1, size.height());
    }

    void TargetMemoryInspectionPane::closeEvent(QCloseEvent* event) {
        this->deactivate();
        QWidget::closeEvent(event);
    }

    void TargetMemoryInspectionPane::postActivate() {
        if (
            this->settings.refreshOnActivation
            && this->targetState == Targets::TargetState::STOPPED
        ) {
            this->refreshMemoryValues([this] {
                this->hexViewerWidget->setDisabled(false);
            });
        }
    }

    void TargetMemoryInspectionPane::postDeactivate() {

    }

    void TargetMemoryInspectionPane::postAttach() {
        this->attachPaneButton->hide();
        this->detachPaneButton->show();
    }

    void TargetMemoryInspectionPane::postDetach() {
        this->detachPaneButton->hide();
        this->attachPaneButton->show();
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
            if (this->settings.refreshOnTargetStop) {
                this->refreshMemoryValues([this] {
                    this->hexViewerWidget->setDisabled(false);
                });

            } else {
                this->hexViewerWidget->setDisabled(false);
            }

        } else if (newState == TargetState::RUNNING) {
            this->hexViewerWidget->setDisabled(true);
        }
    }

    void TargetMemoryInspectionPane::setRefreshOnTargetStopEnabled(bool enabled) {
        this->refreshOnTargetStopAction->setChecked(enabled);
        this->settings.refreshOnTargetStop = enabled;
    }

    void TargetMemoryInspectionPane::setRefreshOnActivationEnabled(bool enabled) {
        this->refreshOnActivationAction->setChecked(enabled);
        this->settings.refreshOnActivation = enabled;
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

    void TargetMemoryInspectionPane::onProgrammingModeEnabled() {
        this->hexViewerWidget->setDisabled(true);
    }

    void TargetMemoryInspectionPane::onProgrammingModeDisabled() {
        this->hexViewerWidget->setDisabled(this->targetState != Targets::TargetState::STOPPED);
    }
}
