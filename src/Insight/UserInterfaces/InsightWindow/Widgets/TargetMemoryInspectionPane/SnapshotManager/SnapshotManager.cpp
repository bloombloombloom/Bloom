#include "SnapshotManager.hpp"

#include <QDesktopServices>
#include <algorithm>

#include "src/Insight/UserInterfaces/InsightWindow/UiLoader.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ConfirmationDialog.hpp"

#include "src/Insight/InsightSignals.hpp"
#include "src/Insight/InsightWorker/Tasks/RetrieveMemorySnapshots.hpp"
#include "src/Insight/InsightWorker/Tasks/CaptureMemorySnapshot.hpp"
#include "src/Insight/InsightWorker/Tasks/DeleteMemorySnapshot.hpp"
#include "src/Insight/InsightWorker/Tasks/WriteTargetMemory.hpp"
#include "src/Insight/InsightWorker/InsightWorker.hpp"

#include "src/Services/PathService.hpp"
#include "src/Exceptions/Exception.hpp"
#include "src/Logger/Logger.hpp"

namespace Bloom::Widgets
{
    using Bloom::Exceptions::Exception;

    SnapshotManager::SnapshotManager(
        const Targets::TargetMemoryDescriptor& memoryDescriptor,
        const std::optional<Targets::TargetMemoryBuffer>& data,
        const bool& staleData,
        const std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
        const std::vector<ExcludedMemoryRegion>& excludedMemoryRegions,
        const std::optional<Targets::TargetStackPointer>& stackPointer,
        PaneState& state,
        PanelWidget* parent
    )
        : PaneWidget(state, parent)
        , memoryDescriptor(memoryDescriptor)
        , data(data)
        , staleData(staleData)
        , focusedMemoryRegions(focusedMemoryRegions)
        , excludedMemoryRegions(excludedMemoryRegions)
        , stackPointer(stackPointer)
    {
        this->setObjectName("snapshot-manager");
        this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        auto widgetUiFile = QFile(
            QString::fromStdString(Services::PathService::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane"
                    + "/SnapshotManager/UiFiles/SnapshotManager.ui"
            )
        );

        if (!widgetUiFile.open(QFile::ReadOnly)) {
            throw Exception("Failed to open SnapshotManager UI file");
        }

        auto uiLoader = UiLoader(this);
        this->container = uiLoader.load(&widgetUiFile, this);

        this->container->setFixedSize(this->size());
        this->container->setContentsMargins(0, 0, 0, 0);

        this->toolBar = this->container->findChild<QWidget*>("tool-bar");
        this->createSnapshotButton = this->toolBar->findChild<SvgToolButton*>("create-snapshot-btn");
        this->deleteSnapshotButton = this->toolBar->findChild<SvgToolButton*>("delete-snapshot-btn");

        auto* containerLayout = this->container->findChild<QVBoxLayout*>();

        this->snapshotListView = new ListView({}, this);
        this->snapshotListView->viewport()->installEventFilter(parent);
        this->snapshotListView->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);

        this->snapshotListScene = this->snapshotListView->listScene();
        this->snapshotListScene->setSelectionLimit(2);

        containerLayout->addWidget(this->snapshotListView);

        this->createSnapshotWindow = new CreateSnapshotWindow(
            this->memoryDescriptor.type,
            this->data,
            this->staleData,
            this
        );

        QObject::connect(
            this->createSnapshotWindow,
            &CreateSnapshotWindow::snapshotCaptureRequested,
            this,
            &SnapshotManager::createSnapshot
        );

        QObject::connect(
            this->createSnapshotButton,
            &QToolButton::clicked,
            this,
            [this] {
                if (!this->createSnapshotWindow->isVisible()) {
                    this->createSnapshotWindow->show();
                    return;
                }

                this->createSnapshotWindow->activateWindow();
            }
        );

        QObject::connect(
            this->snapshotListScene,
            &ListScene::selectionChanged,
            this,
            &SnapshotManager::onSnapshotItemSelectionChanged
        );

        QObject::connect(
            this->snapshotListScene,
            &ListScene::itemDoubleClicked,
            this,
            [this] (ListItem* item) {
                auto* snapshotItem = dynamic_cast<MemorySnapshotItem*>(item);

                if (snapshotItem != nullptr) {
                    this->onSnapshotItemDoubleClick(snapshotItem);
                }
            }
        );

        QObject::connect(
            this->snapshotListScene,
            &ListScene::itemContextMenu,
            this,
            &SnapshotManager::onSnapshotItemContextMenu
        );

        QObject::connect(
            this->openSnapshotViewerAction,
            &QAction::triggered,
            this,
            [this] {
                if (this->selectedSnapshotItems.size() != 1) {
                    return;
                }

                this->openSnapshotViewer(this->selectedSnapshotItems.front()->memorySnapshot.id);
            }
        );

        QObject::connect(
            this->openSnapshotDiffAction,
            &QAction::triggered,
            this,
            [this] {
                if (this->selectedSnapshotItems.size() != 2) {
                    return;
                }

                const auto& first = this->selectedSnapshotItems.front()->memorySnapshot;
                const auto& second = this->selectedSnapshotItems.back()->memorySnapshot;
                this->openSnapshotDiff(
                    first.createdDate < second.createdDate ? first.id : second.id,
                    first.createdDate < second.createdDate ? second.id : first.id
                );
            }
        );

        QObject::connect(
            this->openSnapshotCurrentDiffAction,
            &QAction::triggered,
            this,
            [this] {
                if (this->selectedSnapshotItems.size() != 1) {
                    return;
                }

                this->openSnapshotCurrentDiff(this->selectedSnapshotItems.front()->memorySnapshot.id);
            }
        );

        QObject::connect(
            this->deleteSnapshotAction,
            &QAction::triggered,
            this,
            [this] {
                if (this->selectedSnapshotItems.size() != 1) {
                    return;
                }

                this->deleteSnapshot(this->selectedSnapshotItems.front()->memorySnapshot.id, true);
            }
        );

        QObject::connect(
            this->restoreSnapshotAction,
            &QAction::triggered,
            this,
            [this] {
                if (this->selectedSnapshotItems.size() != 1) {
                    return;
                }

                this->restoreSnapshot(this->selectedSnapshotItems.front()->memorySnapshot.id, true);
            }
        );

        auto* insightSignals = InsightSignals::instance();

        QObject::connect(
            insightSignals,
            &InsightSignals::targetStateUpdated,
            this,
            &SnapshotManager::onTargetStateChanged
        );

        const auto retrieveSnapshotsTask = QSharedPointer<RetrieveMemorySnapshots>(
            new RetrieveMemorySnapshots(this->memoryDescriptor.type),
            &QObject::deleteLater
        );

        QObject::connect(
            retrieveSnapshotsTask.get(),
            &RetrieveMemorySnapshots::memorySnapshotsRetrieved,
            this,
            [this] (std::vector<MemorySnapshot> snapshots) {
                for (auto& snapshot : snapshots) {
                    if (!snapshot.isCompatible(this->memoryDescriptor)) {
                        Logger::warning(
                            "Ignoring snapshot " + snapshot.id.toStdString()
                                + " - snapshot incompatible with current memory descriptor"
                        );
                        continue;
                    }

                    this->addSnapshot(std::move(snapshot));
                }

                this->snapshotListScene->refreshGeometry();
            }
        );

        InsightWorker::queueTask(retrieveSnapshotsTask);

        this->show();
    }

    void SnapshotManager::resizeEvent(QResizeEvent* event) {
        const auto size = this->size();
        this->container->setFixedSize(size.width(), size.height());

        PaneWidget::resizeEvent(event);
    }

    void SnapshotManager::showEvent(QShowEvent* event) {
        PaneWidget::showEvent(event);
    }

    void SnapshotManager::createSnapshot(
        const QString& name,
        const QString& description,
        bool captureFocusedRegions,
        bool captureDirectlyFromTarget
    ) {
        const auto captureTask = QSharedPointer<CaptureMemorySnapshot>(
            new CaptureMemorySnapshot(
                std::move(name),
                std::move(description),
                this->memoryDescriptor.type,
                captureFocusedRegions ? this->focusedMemoryRegions : std::vector<FocusedMemoryRegion>(),
                this->excludedMemoryRegions,
                captureDirectlyFromTarget ? std::nullopt : this->data
            ),
            &QObject::deleteLater
        );

        QObject::connect(
            captureTask.get(),
            &CaptureMemorySnapshot::memorySnapshotCaptured,
            this,
            [this] (MemorySnapshot snapshot) {
                this->addSnapshot(std::move(snapshot));
                this->snapshotListScene->refreshGeometry();
            }
        );

        emit this->insightWorkerTaskCreated(captureTask);

        InsightWorker::queueTask(captureTask);
    }

    void SnapshotManager::addSnapshot(MemorySnapshot&& snapshotTmp) {
        const auto snapshotIt = this->snapshotsById.insert(snapshotTmp.id, std::move(snapshotTmp));
        const auto& snapshot = *snapshotIt;

        const auto snapshotItemIt = this->snapshotItemsById.insert(snapshot.id, new MemorySnapshotItem(snapshot));
        auto& snapshotItem = *snapshotItemIt;

        this->snapshotListScene->addListItem(snapshotItem);
    }

    void SnapshotManager::onSnapshotItemSelectionChanged(const std::list<ListItem*>& selectedItems) {
        this->selectedSnapshotItems.clear();

        std::transform(
            selectedItems.begin(),
            selectedItems.end(),
            std::inserter(this->selectedSnapshotItems, this->selectedSnapshotItems.end()),
            [] (ListItem* item) {
                return dynamic_cast<MemorySnapshotItem*>(item);
            }
        );
    }

    void SnapshotManager::openSnapshotViewer(const QString& snapshotId) {
        auto snapshotViewerIt = this->snapshotViewersById.find(snapshotId);

        if (snapshotViewerIt == this->snapshotViewersById.end()) {
            const auto& snapshotIt = this->snapshotsById.find(snapshotId);

            if (snapshotIt == this->snapshotsById.end()) {
                return;
            }

            snapshotViewerIt = this->snapshotViewersById.insert(
                snapshotId,
                new SnapshotViewer(snapshotIt.value(), this->memoryDescriptor, this)
            );
        }

        auto* snapshotViewer = snapshotViewerIt.value();
        snapshotViewer->show();
        snapshotViewer->activateWindow();
    }

    void SnapshotManager::openSnapshotDiff(const QString& snapshotIdA, const QString& snapshotIdB) {
        const auto diffKey = snapshotIdA + snapshotIdB;
        auto snapshotDiffIt = this->snapshotDiffs.find(diffKey);

        if (snapshotDiffIt == this->snapshotDiffs.end()) {
            const auto& snapshotItA = this->snapshotsById.find(snapshotIdA);
            const auto& snapshotItB = this->snapshotsById.find(snapshotIdB);

            if (snapshotItA == this->snapshotsById.end() || snapshotItB == this->snapshotsById.end()) {
                return;
            }

            snapshotDiffIt = this->snapshotDiffs.insert(
                diffKey,
                new SnapshotDiff(
                    snapshotItA.value(),
                    snapshotItB.value(),
                    this->memoryDescriptor,
                    this
                )
            );
        }

        auto* snapshotDiff = snapshotDiffIt.value();
        snapshotDiff->show();
        snapshotDiff->activateWindow();
    }

    void SnapshotManager::openSnapshotCurrentDiff(const QString& snapshotIdA) {
        if (!this->data.has_value()) {
            return;
        }

        const auto diffKey = snapshotIdA;
        auto snapshotDiffIt = this->snapshotDiffs.find(diffKey);

        if (snapshotDiffIt == this->snapshotDiffs.end()) {
            const auto& snapshotItA = this->snapshotsById.find(snapshotIdA);

            if (snapshotItA == this->snapshotsById.end()) {
                return;
            }

            snapshotDiffIt = this->snapshotDiffs.insert(
                diffKey,
                new SnapshotDiff(
                    snapshotItA.value(),
                    *(this->data),
                    this->focusedMemoryRegions,
                    this->excludedMemoryRegions,
                    this->stackPointer.value_or(0),
                    this->memoryDescriptor,
                    this
                )
            );
        }

        auto* snapshotDiff = snapshotDiffIt.value();
        snapshotDiff->show();
        snapshotDiff->activateWindow();
    }

    void SnapshotManager::deleteSnapshot(const QString& snapshotId, bool confirmationPromptEnabled) {
        const auto& snapshotIt = this->snapshotsById.find(snapshotId);

        if (snapshotIt == this->snapshotsById.end()) {
            return;
        }

        const auto& snapshot = snapshotIt.value();

        if (confirmationPromptEnabled) {
            auto* confirmationDialog = new ConfirmationDialog(
                "Delete snapshot " + snapshot.id,
                "This operation will permanently delete the selected snapshot.<br/><br/>Are you sure you want to proceed?",
                "Proceed",
                std::nullopt,
                this
            );

            QObject::connect(
                confirmationDialog,
                &ConfirmationDialog::confirmed,
                this,
                [this, snapshotId] {
                    this->deleteSnapshot(snapshotId, false);
                }
            );

            confirmationDialog->show();
            return;
        }

        const auto deleteSnapshotTask = QSharedPointer<DeleteMemorySnapshot>(
            new DeleteMemorySnapshot(snapshot.id, snapshot.memoryType),
            &QObject::deleteLater
        );

        QObject::connect(
            deleteSnapshotTask.get(),
            &InsightWorkerTask::completed,
            this,
            [this, snapshotId] () {
                const auto& snapshotViewerIt = this->snapshotViewersById.find(snapshotId);
                const auto& snapshotItemIt = this->snapshotItemsById.find(snapshotId);

                if (snapshotItemIt != this->snapshotItemsById.end()) {
                    auto& snapshotItem = snapshotItemIt.value();
                    this->snapshotListScene->removeListItem(snapshotItem);
                    this->snapshotListScene->refreshGeometry();

                    this->snapshotItemsById.erase(snapshotItemIt);
                }

                if (snapshotViewerIt != this->snapshotViewersById.end()) {
                    auto& snapshotViewer = snapshotViewerIt.value();
                    QObject::connect(
                        snapshotViewer,
                        &QObject::destroyed,
                        this,
                        [this, snapshotId] {
                            this->snapshotsById.remove(snapshotId);
                        }
                    );

                    snapshotViewer->deleteLater();
                    this->snapshotViewersById.erase(snapshotViewerIt);

                } else {
                    this->snapshotsById.remove(snapshotId);
                }
            }
        );

        emit this->insightWorkerTaskCreated(deleteSnapshotTask);
        InsightWorker::queueTask(deleteSnapshotTask);
    }

    void SnapshotManager::restoreSnapshot(const QString& snapshotId, bool confirmationPromptEnabled) {
        const auto& snapshotIt = this->snapshotsById.find(snapshotId);

        if (snapshotIt == this->snapshotsById.end()) {
            return;
        }

        const auto& snapshot = snapshotIt.value();

        if (confirmationPromptEnabled) {
            auto* confirmationDialog = new ConfirmationDialog(
                "Restore snapshot",
                "This operation will overwrite the entire address range of the target's "
                    + QString(this->memoryDescriptor.type == Targets::TargetMemoryType::EEPROM ? "EEPROM" : "RAM")
                        + " with the contents of the selected snapshot.<br/><br/>Are you sure you want to proceed?",
                "Proceed",
                std::nullopt,
                this
            );

            QObject::connect(
                confirmationDialog,
                &ConfirmationDialog::confirmed,
                this,
                [this, snapshotId] {
                    this->restoreSnapshot(snapshotId, false);
                }
            );

            confirmationDialog->show();
            return;
        }

        /*
         * We don't restore any excluded regions from the snapshot, so we split the write operation into blocks of
         * contiguous data, leaving out any address range that is part of an excluded region.
         */
        auto writeBlocks = std::vector<WriteTargetMemory::Block>();

        auto sortedExcludedRegions = std::map<Targets::TargetMemoryAddress, const ExcludedMemoryRegion*>();
        std::transform(
            snapshot.excludedRegions.begin(),
            snapshot.excludedRegions.end(),
            std::inserter(sortedExcludedRegions, sortedExcludedRegions.end()),
            [] (const ExcludedMemoryRegion& excludedMemoryRegion) {
                return std::pair(excludedMemoryRegion.addressRange.startAddress, &excludedMemoryRegion);
            }
        );

        auto blockStartAddress = this->memoryDescriptor.addressRange.startAddress;

        for (const auto& [excludedRegionStartAddress, excludedRegion] : sortedExcludedRegions) {
            assert(excludedRegionStartAddress >= this->memoryDescriptor.addressRange.startAddress);
            assert(excludedRegion->addressRange.endAddress <= this->memoryDescriptor.addressRange.endAddress);

            const auto dataBeginOffset = blockStartAddress - this->memoryDescriptor.addressRange.startAddress;
            const auto dataEndOffset = excludedRegionStartAddress - this->memoryDescriptor.addressRange.startAddress;

            writeBlocks.emplace_back(
                blockStartAddress,
                Targets::TargetMemoryBuffer(
                    snapshot.data.begin() + dataBeginOffset,
                    snapshot.data.begin() + dataEndOffset
                )
            );

            blockStartAddress = excludedRegion->addressRange.endAddress + 1;
        }

        if (blockStartAddress < this->memoryDescriptor.addressRange.endAddress) {
            writeBlocks.emplace_back(
                blockStartAddress,
                Targets::TargetMemoryBuffer(
                    snapshot.data.begin() + (blockStartAddress - this->memoryDescriptor.addressRange.startAddress),
                    snapshot.data.end()
                )
            );
        }

        const auto writeMemoryTask = QSharedPointer<WriteTargetMemory>(
            new WriteTargetMemory(this->memoryDescriptor, std::move(writeBlocks)),
            &QObject::deleteLater
        );

        QObject::connect(
            writeMemoryTask.get(),
            &WriteTargetMemory::targetMemoryWritten,
            this,
            [this, snapshotId] () {
                emit this->snapshotRestored(snapshotId);
            }
        );

        emit this->insightWorkerTaskCreated(writeMemoryTask);
        InsightWorker::queueTask(writeMemoryTask);
    }

    void SnapshotManager::onSnapshotItemDoubleClick(MemorySnapshotItem* item) {
        this->openSnapshotViewer(item->memorySnapshot.id);
    }

    void SnapshotManager::onSnapshotItemContextMenu(ListItem *item, QPoint sourcePosition) {
        auto* snapshotItem = dynamic_cast<MemorySnapshotItem*>(item);

        if (snapshotItem == nullptr) {
            return;
        }

        auto* menu = new QMenu(this);

        menu->addAction(this->openSnapshotViewerAction);
        menu->addAction(this->deleteSnapshotAction);
        menu->addSeparator();
        menu->addAction(this->openSnapshotCurrentDiffAction);
        menu->addSeparator();
        menu->addAction(this->restoreSnapshotAction);

        this->openSnapshotViewerAction->setEnabled(this->selectedSnapshotItems.size() == 1);
        this->deleteSnapshotAction->setEnabled(this->selectedSnapshotItems.size() == 1);
        this->openSnapshotCurrentDiffAction->setEnabled(
            this->selectedSnapshotItems.size() == 1
            && this->data.has_value()
        );
        this->restoreSnapshotAction->setEnabled(
            this->selectedSnapshotItems.size() == 1 && this->targetState == Targets::TargetState::STOPPED
        );

        if (this->selectedSnapshotItems.size() == 2) {
            menu->addAction(this->openSnapshotDiffAction);
        }

        menu->exec(sourcePosition);
    }

    void SnapshotManager::onTargetStateChanged(Targets::TargetState newState) {
        this->targetState = newState;
    }
}
