#include "SnapshotManager.hpp"

#include <QDesktopServices>

#include "src/Insight/UserInterfaces/InsightWindow/UiLoader.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ErrorDialogue/ErrorDialogue.hpp"

#include "src/Insight/InsightWorker/Tasks/RetrieveMemorySnapshots.hpp"
#include "src/Insight/InsightWorker/InsightWorker.hpp"

#include "src/Services/PathService.hpp"
#include "src/Helpers/EnumToStringMappings.hpp"
#include "src/Exceptions/Exception.hpp"
#include "src/Logger/Logger.hpp"

namespace Bloom::Widgets
{
    using Bloom::Exceptions::Exception;

    SnapshotManager::SnapshotManager(
        const Targets::TargetMemoryDescriptor& memoryDescriptor,
        const std::optional<Targets::TargetMemoryBuffer>& data,
        const bool& staleData,
        PaneState& state,
        PanelWidget* parent
    )
        : PaneWidget(state, parent)
        , memoryDescriptor(memoryDescriptor)
        , data(data)
        , staleData(staleData)
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

                this->snapshotListScene->sortItems();
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
                {},
                {},
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
                this->snapshotListScene->sortItems();
                this->snapshotListScene->refreshGeometry();
            }
        );

        emit this->captureTaskCreated(captureTask);

        InsightWorker::queueTask(captureTask);
    }

    void SnapshotManager::addSnapshot(MemorySnapshot&& snapshotTmp) {
        const auto snapshotIt = this->snapshotsById.emplace(snapshotTmp.id, std::move(snapshotTmp));
        const auto& snapshot = *snapshotIt;

        const auto snapshotItemIt = this->snapshotItemsById.emplace(snapshot.id, new MemorySnapshotItem(snapshot));
        auto& snapshotItem = *snapshotItemIt;

        this->snapshotListScene->addListItem(snapshotItem);
    }

    void SnapshotManager::onSnapshotItemSelected(MemorySnapshotItem* item) {
        if (this->selectedItem != nullptr && this->selectedItem != item) {
            this->selectedItem->setSelected(false);
        }

        this->selectedItem = item;
    }
}
