#include "TargetMemoryInspectionPane.hpp"

#include <QVBoxLayout>
#include <QToolButton>
#include <QLocale>

#include "src/Insight/UserInterfaces/InsightWindow/UiLoader.hpp"
#include "src/Insight/InsightSignals.hpp"
#include "src/Insight/InsightWorker/InsightWorker.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/Label.hpp"

#include "src/Insight/InsightWorker/Tasks/ReadTargetMemory.hpp"
#include "src/Insight/InsightWorker/Tasks/ReadStackPointer.hpp"

#include "src/Services/PathService.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Bloom::Widgets
{
    using namespace Bloom::Exceptions;

    using Bloom::Targets::TargetMemoryDescriptor;
    using Bloom::Targets::TargetMemoryType;
    using Bloom::Targets::TargetMemoryAddressRange;

    TargetMemoryInspectionPane::TargetMemoryInspectionPane(
        const TargetMemoryDescriptor& targetMemoryDescriptor,
        TargetMemoryInspectionPaneSettings& settings,
        PaneState& paneState,
        PanelWidget* parent
    )
        : PaneWidget(paneState, parent)
        , targetMemoryDescriptor(targetMemoryDescriptor)
        , settings(settings)
    {
        this->setObjectName("target-memory-inspection-pane");

        const auto memoryName = QString(
            this->targetMemoryDescriptor.type == TargetMemoryType::EEPROM
                ? "Internal EEPROM"
                : "Internal RAM"
        );

        this->setWindowTitle("Memory Inspection - " + memoryName);
        this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        auto uiFile = QFile(
            QString::fromStdString(Services::PathService::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/UiFiles/TargetMemoryInspectionPane.ui"
            )
        );

        auto stylesheetFile = QFile(
            QString::fromStdString(Services::PathService::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/Stylesheets/TargetMemoryInspectionPane.qss"
            )
        );

        if (!uiFile.open(QFile::ReadOnly)) {
            throw Exception("Failed to open MemoryInspectionPane UI file");
        }

        if (!stylesheetFile.open(QFile::ReadOnly)) {
            throw Exception("Failed to open MemoryInspectionPane stylesheet file");
        }

        auto uiLoader = UiLoader(this);
        this->container = uiLoader.load(&uiFile, this);
        this->container->setStyleSheet(stylesheetFile.readAll());
        this->container->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        this->subContainerLayout = this->container->findChild<QHBoxLayout*>("container-sub-layout");

        this->titleBar = this->container->findChild<QWidget*>("title-bar");

        this->bottomBar = this->container->findChild<QWidget*>("bottom-bar");
        this->bottomBarLayout = this->bottomBar->findChild<QHBoxLayout*>();

        this->manageMemoryRegionsButton = this->container->findChild<SvgToolButton*>("manage-memory-regions-btn");
        this->manageMemorySnapshotsButton = this->container->findChild<QToolButton*>("manage-memory-snapshots-btn");

        this->refreshButton = this->container->findChild<SvgToolButton*>("refresh-memory-btn");
        this->refreshOnTargetStopAction = this->refreshButton->findChild<QAction*>("refresh-target-stopped");
        this->refreshOnActivationAction = this->refreshButton->findChild<QAction*>("refresh-activation");

        this->detachPaneButton = this->container->findChild<SvgToolButton*>("detach-pane-btn");
        this->attachPaneButton = this->container->findChild<SvgToolButton*>("attach-pane-btn");

        this->manageMemorySnapshotsButton->layout()->setContentsMargins(0, 0, 0, 0);

        this->staleDataLabelContainer = this->container->findChild<QWidget*>("stale-data-label");

        this->titleBar->layout()->setContentsMargins(7, 0, 7, 0);
        auto* titleLabel = this->titleBar->findChild<Label*>("title");
        titleLabel->setText(memoryName);

        auto* memoryCapacityLabel = this->container->findChild<Label*>("memory-capacity-label");
        memoryCapacityLabel->setText(QLocale(QLocale::English).toString(this->targetMemoryDescriptor.size()) + " Bytes");

        // Quick sanity check to ensure the validity of persisted settings.
        this->sanitiseSettings();

        this->hexViewerWidget = new HexViewerWidget(
            this->targetMemoryDescriptor,
            this->data,
            this->settings.hexViewerWidgetSettings,
            this->settings.focusedMemoryRegions,
            this->settings.excludedMemoryRegions,
            this
        );
        this->hexViewerWidget->setDisabled(true);

        this->subContainerLayout->insertWidget(1, this->hexViewerWidget);

        this->hexViewerWidget->init();

        this->rightPanel = new PanelWidget(PanelWidgetType::RIGHT, this->settings.rightPanelState, this);
        this->rightPanel->setObjectName("right-panel");
        this->rightPanel->setMinimumResize(200);
        this->rightPanel->setHandleSize(6);
        this->subContainerLayout->insertWidget(2, this->rightPanel);

        this->snapshotManager = new SnapshotManager(
            this->targetMemoryDescriptor,
            this->data,
            this->staleData,
            this->settings.focusedMemoryRegions,
            this->settings.excludedMemoryRegions,
            this->settings.snapshotManagerState,
            this->rightPanel
        );
        this->rightPanel->layout()->addWidget(this->snapshotManager);

        this->setRefreshOnTargetStopEnabled(this->settings.refreshOnTargetStop);
        this->setRefreshOnActivationEnabled(this->settings.refreshOnActivation);

        this->taskProgressIndicator = new TaskProgressIndicator(this);
        this->bottomBarLayout->insertWidget(5, this->taskProgressIndicator);

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
            this->manageMemorySnapshotsButton,
            &QToolButton::clicked,
            this,
            &TargetMemoryInspectionPane::toggleMemorySnapshotManagerPane
        );

        QObject::connect(
            this->snapshotManager,
            &PaneWidget::paneActivated,
            this,
            [this] {
                this->manageMemorySnapshotsButton->setChecked(true);
            }
        );

        QObject::connect(
            this->snapshotManager,
            &PaneWidget::paneDeactivated,
            this,
            [this] {
                this->manageMemorySnapshotsButton->setChecked(false);
            }
        );

        QObject::connect(
            this->snapshotManager,
            &SnapshotManager::insightWorkerTaskCreated,
            this,
            &TargetMemoryInspectionPane::onSubtaskCreated
        );

        QObject::connect(
            this->snapshotManager,
            &SnapshotManager::snapshotRestored,
            this,
            &TargetMemoryInspectionPane::onSnapshotRestored
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

        auto* insightSignals = InsightSignals::instance();

        QObject::connect(
            insightSignals,
            &InsightSignals::targetStateUpdated,
            this,
            &TargetMemoryInspectionPane::onTargetStateChanged
        );

        QObject::connect(
            insightSignals,
            &InsightSignals::targetReset,
            this,
            &TargetMemoryInspectionPane::onTargetReset
        );

        QObject::connect(
            insightSignals,
            &InsightSignals::programmingModeEnabled,
            this,
            &TargetMemoryInspectionPane::onProgrammingModeEnabled
        );

        QObject::connect(
            insightSignals,
            &InsightSignals::programmingModeDisabled,
            this,
            &TargetMemoryInspectionPane::onProgrammingModeDisabled
        );

        QObject::connect(
            insightSignals,
            &InsightSignals::targetMemoryWritten,
            this,
            &TargetMemoryInspectionPane::onTargetMemoryWritten
        );

        // Restore the state
        if (this->state.attached) {
            this->attach();

        } else {
            this->detach();
        }

        if (this->state.activated) {
            this->activate();

        } else {
            this->deactivate();
        }

        this->snapshotManager->deactivate();
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

        const auto readMemoryTask = QSharedPointer<ReadTargetMemory>(
            new ReadTargetMemory(
                this->targetMemoryDescriptor.type,
                this->targetMemoryDescriptor.addressRange.startAddress,
                this->targetMemoryDescriptor.size(),
                excludedAddressRanges
            ),
            &QObject::deleteLater
        );

        QObject::connect(
            readMemoryTask.get(),
            &ReadTargetMemory::targetMemoryRead,
            this,
            [this, callback] (const Targets::TargetMemoryBuffer& data) {
                this->onMemoryRead(data);

                // Refresh the stack pointer if this is RAM.
                if (this->targetMemoryDescriptor.type == Targets::TargetMemoryType::RAM) {
                    const auto readStackPointerTask = QSharedPointer<ReadStackPointer>(
                        new ReadStackPointer(),
                        &QObject::deleteLater
                    );

                    QObject::connect(
                        readStackPointerTask.get(),
                        &ReadStackPointer::stackPointerRead,
                        this,
                        [this] (Targets::TargetStackPointer stackPointer) {
                            this->hexViewerWidget->setStackPointer(stackPointer);
                        }
                    );

                    QObject::connect(
                        readStackPointerTask.get(),
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
                            readStackPointerTask.get(),
                            &InsightWorkerTask::completed,
                            this,
                            callback.value()
                        );
                    }

                    this->taskProgressIndicator->addTask(readStackPointerTask);
                    InsightWorker::queueTask(readStackPointerTask);
                }
            }
        );

        QObject::connect(
            readMemoryTask.get(),
            &InsightWorkerTask::finished,
            this,
            [this, taskId = readMemoryTask->id] {
                if (this->activeRefreshTask.has_value() && this->activeRefreshTask->get()->id == taskId) {
                    this->activeRefreshTask.reset();
                }
            }
        );

        // If we're refreshing RAM, the UI should only be updated once we've retrieved the current stack pointer.
        if (this->targetMemoryDescriptor.type != Targets::TargetMemoryType::RAM) {
            QObject::connect(
                readMemoryTask.get(),
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
                    readMemoryTask.get(),
                    &InsightWorkerTask::completed,
                    this,
                    callback.value()
                );
            }

        } else {
            QObject::connect(
                readMemoryTask.get(),
                &InsightWorkerTask::failed,
                this,
                [this] {
                    this->refreshButton->stopSpin();

                    if (this->targetState == Targets::TargetState::STOPPED) {
                        this->refreshButton->setDisabled(false);
                    }
                }
            );
        }

        this->activeRefreshTask = readMemoryTask;
        this->taskProgressIndicator->addTask(readMemoryTask);
        InsightWorker::queueTask(readMemoryTask);
    }

    void TargetMemoryInspectionPane::resizeEvent(QResizeEvent* event) {
        const auto size = this->size();
        this->container->setFixedSize(size.width(), size.height());

        this->rightPanel->setMaximumResize(static_cast<int>(size.width() * 0.4));

        PaneWidget::resizeEvent(event);
    }

    void TargetMemoryInspectionPane::keyPressEvent(QKeyEvent* event) {
        if ((event->modifiers() & Qt::ControlModifier) != 0 && event->key() == Qt::Key::Key_R) {
            this->refreshButton->click();
            event->accept();
        }
    }

    void TargetMemoryInspectionPane::postActivate() {
        if (this->targetState == Targets::TargetState::STOPPED) {
            if (
                !this->activeRefreshTask.has_value()
                && (this->settings.refreshOnActivation || !this->data.has_value())
            ) {
                this->refreshMemoryValues([this] {
                    this->hexViewerWidget->setDisabled(false);
                });

            } else if (this->data.has_value()) {
                this->hexViewerWidget->setDisabled(false);
                this->refreshButton->setDisabled(false);
            }
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
            if (
                !this->targetMemoryDescriptor.addressRange.contains(focusedRegion.addressRange)
                || regionIntersects(focusedRegion)
            ) {
                continue;
            }

            processedFocusedMemoryRegions.emplace_back(focusedRegion);
        }

        for (const auto& excludedRegion : this->settings.excludedMemoryRegions) {
            if (
                !this->targetMemoryDescriptor.addressRange.contains(excludedRegion.addressRange)
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

        if (newState == TargetState::STOPPED) {
            if (this->state.activated && (this->settings.refreshOnTargetStop || !this->data.has_value())) {
                this->refreshMemoryValues([this] {
                    this->hexViewerWidget->setDisabled(false);
                });
            }
        }

        if (newState == TargetState::RUNNING) {
            this->hexViewerWidget->setDisabled(true);
            this->refreshButton->setDisabled(true);

            if (this->data.has_value()) {
                this->setStaleData(true);
            }
        }

        this->snapshotManager->createSnapshotWindow->refreshForm();
    }

    void TargetMemoryInspectionPane::setRefreshOnTargetStopEnabled(bool enabled) {
        this->refreshOnTargetStopAction->setChecked(enabled);
        this->settings.refreshOnTargetStop = enabled;
    }

    void TargetMemoryInspectionPane::setRefreshOnActivationEnabled(bool enabled) {
        this->refreshOnActivationAction->setChecked(enabled);
        this->settings.refreshOnActivation = enabled;
    }

    void TargetMemoryInspectionPane::onMemoryRead(const Targets::TargetMemoryBuffer& data) {
        assert(data.size() == this->targetMemoryDescriptor.size());

        this->data = data;
        this->hexViewerWidget->updateValues();
        this->setStaleData(false);

        this->snapshotManager->createSnapshotWindow->refreshForm();
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

    void TargetMemoryInspectionPane::toggleMemorySnapshotManagerPane() {
        if (!this->snapshotManager->state.activated) {
            this->snapshotManager->activate();
            return;
        }

        this->snapshotManager->deactivate();
    }

    void TargetMemoryInspectionPane::onMemoryRegionsChange() {
        this->hexViewerWidget->refreshRegions();
    }

    void TargetMemoryInspectionPane::onTargetReset() {
        if (this->data.has_value()) {
            this->setStaleData(true);
        }
    }

    void TargetMemoryInspectionPane::onProgrammingModeEnabled() {
        this->hexViewerWidget->setDisabled(true);
        this->refreshButton->setDisabled(true);

        if (this->data.has_value()) {
            this->setStaleData(true);
        }
    }

    void TargetMemoryInspectionPane::onProgrammingModeDisabled() {
        const auto disabled = this->targetState != Targets::TargetState::STOPPED;
        this->hexViewerWidget->setDisabled(disabled);
        this->refreshButton->setDisabled(disabled);
    }

    void TargetMemoryInspectionPane::onTargetMemoryWritten(
        TargetMemoryType memoryType,
        TargetMemoryAddressRange
    ) {
        if (memoryType == this->targetMemoryDescriptor.type && this->data.has_value()) {
            this->setStaleData(true);
            this->snapshotManager->createSnapshotWindow->refreshForm();
        }
    }

    void TargetMemoryInspectionPane::onSubtaskCreated(const QSharedPointer<InsightWorkerTask>& task) {
        this->taskProgressIndicator->addTask(task);
    }

    void TargetMemoryInspectionPane::onSnapshotRestored(const QString& snapshotId) {
        this->refreshButton->click();
    }

    void TargetMemoryInspectionPane::setStaleData(bool staleData) {
        this->staleData = staleData;
        this->staleDataLabelContainer->setVisible(this->staleData);
    }
}
