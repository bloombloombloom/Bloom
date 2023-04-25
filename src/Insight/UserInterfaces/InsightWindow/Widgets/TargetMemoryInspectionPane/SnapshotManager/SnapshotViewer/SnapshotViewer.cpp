#include "SnapshotViewer.hpp"

#include <QFile>
#include <QVBoxLayout>
#include <algorithm>
#include <QSize>
#include <QDesktopServices>
#include <algorithm>
#include <map>

#include "src/Insight/InsightWorker/Tasks/WriteTargetMemory.hpp"
#include "src/Insight/InsightWorker/InsightWorker.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/UiLoader.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/Label.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ConfirmationDialog.hpp"

#include "src/Services/PathService.hpp"
#include "src/Exceptions/Exception.hpp"

#include "src/Insight/InsightSignals.hpp"
#include "src/Logger/Logger.hpp"

namespace Bloom::Widgets
{
    using Bloom::Exceptions::Exception;

    SnapshotViewer::SnapshotViewer(
        MemorySnapshot& snapshot,
        const Targets::TargetMemoryDescriptor& memoryDescriptor,
        QWidget* parent
    )
        : QWidget(parent)
        , snapshot(snapshot)
        , memoryDescriptor(memoryDescriptor)
        , hexViewerData(snapshot.data)
    {
        this->setWindowFlag(Qt::Window);
        this->setObjectName("snapshot-viewer");
        this->setWindowTitle(this->snapshot.name + " (" + this->snapshot.id + ")");

        auto windowUiFile = QFile(
            QString::fromStdString(Services::PathService::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane"
                + "/SnapshotManager/SnapshotViewer/UiFiles/SnapshotViewer.ui"
            )
        );

        auto stylesheetFile = QFile(
            QString::fromStdString(Services::PathService::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane"
                + "/SnapshotManager/SnapshotViewer/Stylesheets/SnapshotViewer.qss"
            )
        );

        if (!windowUiFile.open(QFile::ReadOnly)) {
            throw Exception("Failed to open SnapshotViewer UI file");
        }

        if (!stylesheetFile.open(QFile::ReadOnly)) {
            throw Exception("Failed to open SnapshotViewer stylesheet file");
        }

        // Set ideal window size
        this->setFixedSize(1205, 850);
        this->setMinimumSize(700, 600);
        this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

        auto uiLoader = UiLoader(this);
        this->setStyleSheet(stylesheetFile.readAll());
        this->container = uiLoader.load(&windowUiFile, this);

        auto* containerLayout = this->container->findChild<QVBoxLayout*>();
        this->detailsContainer = this->container->findChild<QWidget*>("details-container");

        this->nameInput = this->detailsContainer->findChild<TextInput*>("name-input");
        this->descriptionInput = this->detailsContainer->findChild<PlainTextEdit*>("description-input");

        auto* detailsContainerLayout = this->detailsContainer->findChild<QHBoxLayout*>();
        detailsContainerLayout->setContentsMargins(0, 0, 0, 0);

        auto* attributesLayout = this->detailsContainer->findChild<QVBoxLayout*>("attributes-layout");
        attributesLayout->setContentsMargins(10, 10, 10, 0);

        auto* rightPanelLayout = this->detailsContainer->findChild<QVBoxLayout*>("right-panel-layout");
        rightPanelLayout->setContentsMargins(0, 0, 0, 0);

        if (!this->snapshot.excludedRegions.empty() || !this->snapshot.focusedRegions.empty()) {
            auto* memoryRegionsContainer = this->detailsContainer->findChild<QWidget*>("memory-regions-container");
            auto* memoryRegionsLayout = memoryRegionsContainer->findChild<QVBoxLayout*>();
            auto* noMemoryRegionsLabel = memoryRegionsContainer->findChild<Label*>("no-regions-label");

            std::transform(
                this->snapshot.focusedRegions.begin(),
                this->snapshot.focusedRegions.end(),
                std::back_inserter(this->memoryRegionItems),
                [] (const MemoryRegion& focusedRegion) {
                    return new MemoryRegionItem(focusedRegion);
                }
            );

            std::transform(
                this->snapshot.excludedRegions.begin(),
                this->snapshot.excludedRegions.end(),
                std::back_inserter(this->memoryRegionItems),
                [] (const MemoryRegion& excludedRegion) {
                    return new MemoryRegionItem(excludedRegion);
                }
            );

            this->memoryRegionListView = new ListView(
                ListScene::ListItemSetType(this->memoryRegionItems.begin(), this->memoryRegionItems.end()),
                this
            );
            this->memoryRegionListView->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);

            this->memoryRegionListScene = this->memoryRegionListView->listScene();
            this->memoryRegionListScene->margins = QMargins(0, 5, 0, 5);
            this->memoryRegionListScene->setSelectionLimit(2);

            noMemoryRegionsLabel->hide();
            memoryRegionsLayout->insertWidget(0, this->memoryRegionListView);
        }

        this->restoreBytesAction = new ContextMenuAction("Restore Selection", std::nullopt, this);

        this->hexViewerWidget = new HexViewerWidget(
            this->memoryDescriptor,
            this->hexViewerData,
            this->hexViewerWidgetSettings,
            this->snapshot.focusedRegions,
            this->snapshot.excludedRegions,
            this
        );

        containerLayout->insertWidget(1, this->hexViewerWidget);

        this->bottomBar = this->container->findChild<QWidget*>("bottom-bar");
        this->bottomBarLayout = this->bottomBar->findChild<QHBoxLayout*>();

        auto* memoryCapacityLabel = this->bottomBar->findChild<Label*>("memory-capacity-label");
        auto* snapshotIdLabel = this->bottomBar->findChild<Label*>("id-label");
        auto* programCounterLabel = this->bottomBar->findChild<Label*>("program-counter-label");
        auto* dateLabel = this->bottomBar->findChild<Label*>("date-label");

        memoryCapacityLabel->setText(QLocale(QLocale::English).toString(this->memoryDescriptor.size()) + " Bytes");
        snapshotIdLabel->setText(this->snapshot.id);
        programCounterLabel->setText(
            "0x" + QString::number(this->snapshot.programCounter, 16).rightJustified(8, '0').toUpper()
        );
        dateLabel->setText(this->snapshot.createdDate.toString("dd/MM/yyyy hh:mm"));

        this->nameInput->setText(this->snapshot.name);
        this->descriptionInput->setPlainText(this->snapshot.description);

        this->taskProgressIndicator = new TaskProgressIndicator(this);
        this->bottomBarLayout->insertWidget(2, this->taskProgressIndicator);

        auto* insightSignals = InsightSignals::instance();

        QObject::connect(
            this->restoreBytesAction,
            &ContextMenuAction::invoked,
            this,
            [this] (const std::unordered_map<Targets::TargetMemoryAddress, ByteItem*>& selectedByteItemsByAddress) {
                this->restoreSelectedBytes(selectedByteItemsByAddress, true);
            }
        );

        QObject::connect(this->hexViewerWidget, &HexViewerWidget::ready, this, &SnapshotViewer::onHexViewerReady);

        this->hexViewerWidget->init();
        this->move(this->parentWidget()->window()->geometry().center() - this->rect().center());
    }

    void SnapshotViewer::showEvent(QShowEvent* event) {
        QWidget::showEvent(event);
    }

    void SnapshotViewer::resizeEvent(QResizeEvent* event) {
        this->container->setFixedSize(this->size());

        QWidget::resizeEvent(event);
    }

    void SnapshotViewer::onHexViewerReady() {
        this->hexViewerWidget->addExternalContextMenuAction(this->restoreBytesAction);
        this->hexViewerWidget->setStackPointer(this->snapshot.stackPointer);
    }

    void SnapshotViewer::restoreSelectedBytes(
        const std::unordered_map<Targets::TargetMemoryAddress, ByteItem*>& selectedByteItemsByAddress,
        bool confirmationPromptEnabled
    ) {
        auto sortedByteItemsByAddress = std::map<Targets::TargetMemoryAddress, ByteItem*>();

        // Ideally, we'd use std::transform here, but that would require an additional pass to remove excluded bytes
        for (const auto& pair : selectedByteItemsByAddress) {
            if (pair.second->excluded) {
                continue;
            }

            sortedByteItemsByAddress.insert(pair);
        }

        if (sortedByteItemsByAddress.empty()) {
            // The user has only selected bytes that are within an excluded region - nothing to do here
            return;
        }

        if (confirmationPromptEnabled) {
            auto* confirmationDialog = new ConfirmationDialog(
                "Restore selected bytes",
                "This operation will write " + QString::number(sortedByteItemsByAddress.size())
                    + " byte(s) to the target's "
                    + QString(this->memoryDescriptor.type == Targets::TargetMemoryType::EEPROM ? "EEPROM" : "RAM")
                    + ".<br/><br/>Are you sure you want to proceed?",
                "Proceed",
                std::nullopt,
                this
            );

            QObject::connect(
                confirmationDialog,
                &ConfirmationDialog::confirmed,
                this,
                [this, selectedByteItemsByAddress] {
                    this->restoreSelectedBytes(selectedByteItemsByAddress, false);
                }
            );

            confirmationDialog->show();
            return;
        }

        auto writeBlocks = std::vector<WriteTargetMemory::Block>();

        Targets::TargetMemoryAddress blockStartAddress = sortedByteItemsByAddress.begin()->first;
        Targets::TargetMemoryAddress blockEndAddress = blockStartAddress;

        for (const auto& [address, byteItem] : sortedByteItemsByAddress) {
            if (address > (blockEndAddress + 1)) {
                // Commit the block
                const auto dataBeginOffset = blockStartAddress - this->memoryDescriptor.addressRange.startAddress;
                const auto dataEndOffset = blockEndAddress - this->memoryDescriptor.addressRange.startAddress + 1;

                writeBlocks.emplace_back(
                    blockStartAddress,
                    Targets::TargetMemoryBuffer(
                        this->snapshot.data.begin() + dataBeginOffset,
                        this->snapshot.data.begin() + dataEndOffset
                    )
                );

                blockStartAddress = address;
                blockEndAddress = address;
                continue;
            }

            blockEndAddress = address;
        }

        {
            const auto dataBeginOffset = blockStartAddress - this->memoryDescriptor.addressRange.startAddress;
            const auto dataEndOffset = blockEndAddress - this->memoryDescriptor.addressRange.startAddress + 1;

            writeBlocks.emplace_back(
                blockStartAddress,
                Targets::TargetMemoryBuffer(
                    this->snapshot.data.begin() + dataBeginOffset,
                    this->snapshot.data.begin() + dataEndOffset
                )
            );
        }

        const auto writeMemoryTask = QSharedPointer<WriteTargetMemory>(
            new WriteTargetMemory(this->memoryDescriptor, std::move(writeBlocks)),
            &QObject::deleteLater
        );

        this->taskProgressIndicator->addTask(writeMemoryTask);
        InsightWorker::queueTask(writeMemoryTask);
    }
}
