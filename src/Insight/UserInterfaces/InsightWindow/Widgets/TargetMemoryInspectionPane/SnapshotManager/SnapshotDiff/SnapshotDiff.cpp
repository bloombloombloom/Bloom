#include "SnapshotDiff.hpp"

#include <QFile>
#include <QVBoxLayout>
#include <QLocale>
#include <algorithm>

#include "src/Insight/InsightWorker/Tasks/WriteTargetMemory.hpp"
#include "src/Insight/InsightWorker/InsightWorker.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/UiLoader.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ConfirmationDialog.hpp"

#include "src/Services/PathService.hpp"
#include "src/Helpers/EnumToStringMappings.hpp"
#include "src/Exceptions/Exception.hpp"

#include "src/Insight/InsightSignals.hpp"
#include "src/Logger/Logger.hpp"

namespace Widgets
{
    using Exceptions::Exception;

    SnapshotDiff::SnapshotDiff(
        MemorySnapshot& snapshotA,
        MemorySnapshot& snapshotB,
        const Targets::TargetMemoryDescriptor& memoryDescriptor,
        Targets::TargetState currentTargetState,
        QWidget* parent
    )
        : QWidget(parent)
        , memoryDescriptor(memoryDescriptor)
        , hexViewerDataA(snapshotA.data)
        , focusedRegionsA(snapshotA.focusedRegions)
        , excludedRegionsA(snapshotA.excludedRegions)
        , stackPointerA(snapshotA.stackPointer)
        , hexViewerDataB(snapshotB.data)
        , focusedRegionsB(snapshotB.focusedRegions)
        , excludedRegionsB(snapshotB.excludedRegions)
        , stackPointerB(snapshotB.stackPointer)
        , comparingWithCurrent(false)
    {
        this->init();
        this->onTargetStateChanged(currentTargetState);

        this->setWindowTitle(
            "Comparing snapshot \"" + snapshotA.name + "\" with snapshot \"" + snapshotB.name + "\""
        );

        this->dataAPrimaryLabel->setText(snapshotA.name);
        this->dataBPrimaryLabel->setText(snapshotB.name);
        this->dataASecondaryLabel->setText(snapshotA.createdDate.toString("dd/MM/yyyy hh:mm"));
        this->dataBSecondaryLabel->setText(snapshotB.createdDate.toString("dd/MM/yyyy hh:mm"));
    }

    SnapshotDiff::SnapshotDiff(
        MemorySnapshot& snapshotA,
        Targets::TargetMemoryBuffer dataB,
        std::vector<FocusedMemoryRegion> focusedRegionsB,
        std::vector<ExcludedMemoryRegion> excludedRegionsB,
        Targets::TargetStackPointer stackPointerB,
        const Targets::TargetMemoryDescriptor& memoryDescriptor,
        Targets::TargetState currentTargetState,
        QWidget* parent
    )
        : QWidget(parent)
        , memoryDescriptor(memoryDescriptor)
        , hexViewerDataA(snapshotA.data)
        , focusedRegionsA(snapshotA.focusedRegions)
        , excludedRegionsA(snapshotA.excludedRegions)
        , stackPointerA(snapshotA.stackPointer)
        , hexViewerDataB(std::move(dataB))
        , focusedRegionsB(focusedRegionsB)
        , excludedRegionsB(excludedRegionsB)
        , stackPointerB(stackPointerB)
        , comparingWithCurrent(true)
    {
        this->init();
        this->onTargetStateChanged(currentTargetState);

        this->setWindowTitle(
            "Comparing snapshot \"" + snapshotA.name + "\" with current memory"
        );

        this->dataAPrimaryLabel->setText(snapshotA.name);
        this->dataBPrimaryLabel->setText("Current");
        this->dataASecondaryLabel->setText(snapshotA.createdDate.toString("dd/MM/yyyy hh:mm"));
        this->dataBSecondaryLabel->setVisible(false);

        this->restoreBytesAction = new ContextMenuAction(
            "Restore Selection",
            [this] (const std::set<Targets::TargetMemoryAddress>&) {
                return
                    this->memoryDescriptor.access.writeableDuringDebugSession
                    && this->targetState == Targets::TargetState::STOPPED;
            },
            this
        );

        QObject::connect(
            this->restoreBytesAction,
            &ContextMenuAction::invoked,
            this,
            [this] (const std::set<Targets::TargetMemoryAddress>& selectedByteItemAddresses) {
                this->restoreSelectedBytes(selectedByteItemAddresses, true);
            }
        );

        QObject::connect(
            this->changeListPane,
            &ChangeListPane::restoreBytesRequested,
            this,
            [this] (const std::set<Targets::TargetMemoryAddress>& addresses) {
                this->restoreSelectedBytes(addresses, true);
            }
        );
    }

    void SnapshotDiff::refreshB(
        Targets::TargetMemoryBuffer data,
        std::vector<FocusedMemoryRegion> focusedRegions,
        std::vector<ExcludedMemoryRegion> excludedRegions,
        Targets::TargetStackPointer stackPointer
    ) {
        this->hexViewerDataB = std::move(data);
        this->focusedRegionsB = focusedRegions;
        this->excludedRegionsB = excludedRegions;
        this->stackPointerB = stackPointer;

        if (this->memoryDescriptor.type == Targets::TargetMemoryType::RAM) {
            this->hexViewerWidgetB->setStackPointer(this->stackPointerB);
        }

        this->refreshDifferences();
        this->hexViewerWidgetB->refreshRegions();
        this->hexViewerWidgetB->updateValues();

        this->hexViewerWidgetA->updateValues();
    }

    void SnapshotDiff::showEvent(QShowEvent* event) {
        QWidget::showEvent(event);
    }

    void SnapshotDiff::resizeEvent(QResizeEvent* event) {
        const auto windowSize = this->size();
        this->container->setFixedSize(windowSize);

        const auto maxLeftPanelSize = static_cast<int>(windowSize.width() / 3);
        this->leftPanel->setMaximumResize(maxLeftPanelSize);
        this->leftPanel->setMinimumResize(std::min(this->leftPanel->getMinimumResize(), maxLeftPanelSize));

        QWidget::resizeEvent(event);
    }

    void SnapshotDiff::keyPressEvent(QKeyEvent* keyEvent) {
        const auto key = keyEvent->key();

        if (key == Qt::Key_Escape) {
            this->hexViewerWidgetA->clearHighlighting();
            this->hexViewerWidgetB->clearHighlighting();
        }

        QWidget::keyPressEvent(keyEvent);
    }

    void SnapshotDiff::init() {
        this->setWindowFlag(Qt::Window);
        this->setObjectName("snapshot-diff");

        auto windowUiFile = QFile(
            QString::fromStdString(Services::PathService::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane"
                + "/SnapshotManager/SnapshotDiff/UiFiles/SnapshotDiff.ui"
            )
        );

        auto stylesheetFile = QFile(
            QString::fromStdString(Services::PathService::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane"
                + "/SnapshotManager/SnapshotDiff/Stylesheets/SnapshotDiff.qss"
            )
        );

        if (!windowUiFile.open(QFile::ReadOnly)) {
            throw Exception("Failed to open SnapshotDiff UI file");
        }

        if (!stylesheetFile.open(QFile::ReadOnly)) {
            throw Exception("Failed to open SnapshotDiff stylesheet file");
        }

        // Set ideal window size
        this->setFixedSize(1600, 910);
        this->setMinimumSize(800, 600);
        this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

        auto uiLoader = UiLoader(this);
        const auto styleSheet = stylesheetFile.readAll();
        this->container = uiLoader.load(&windowUiFile, this);
        this->container->setStyleSheet(styleSheet);

        auto* toolBar = this->container->findChild<QWidget*>("tool-bar");

        this->syncHexViewerSettingsButton = toolBar->findChild<SvgToolButton*>("sync-settings-btn");
        this->syncHexViewerScrollButton = toolBar->findChild<SvgToolButton*>("sync-scroll-btn");
        this->syncHexViewerHoverButton = toolBar->findChild<SvgToolButton*>("sync-hover-btn");
        this->syncHexViewerSelectionButton = toolBar->findChild<SvgToolButton*>("sync-selection-btn");

        this->viewChangeListButton = this->container->findChild<QToolButton*>("change-list-btn");
        this->viewChangeListButton->layout()->setContentsMargins(0, 0, 0, 0);

        auto* subContainerLayout = this->container->findChild<QWidget*>(
            "sub-container"
        )->findChild<QHBoxLayout*>("sub-layout");

        this->leftPanel = new PanelWidget(PanelWidgetType::LEFT, this->leftPanelState, this);
        this->leftPanel->setObjectName("left-panel");
        this->leftPanel->setMinimumResize(200);
        this->leftPanel->setHandleSize(6);
        subContainerLayout->insertWidget(1, this->leftPanel, 0, Qt::AlignLeft);

        this->dataAContainer = this->container->findChild<QWidget*>("data-a-container");
        this->dataBContainer = this->container->findChild<QWidget*>("data-b-container");

        this->dataAPrimaryLabel = this->dataAContainer->findChild<Label*>("primary-label");
        this->dataASecondaryLabel = this->dataAContainer->findChild<Label*>("secondary-label");
        this->dataBPrimaryLabel = this->dataBContainer->findChild<Label*>("primary-label");
        this->dataBSecondaryLabel = this->dataBContainer->findChild<Label*>("secondary-label");

        auto snapshotAContainerLayout = this->dataAContainer->findChild<QVBoxLayout*>();
        auto snapshotBContainerLayout = this->dataBContainer->findChild<QVBoxLayout*>();

        this->hexViewerWidgetA = new DifferentialHexViewerWidget(
            DifferentialHexViewerWidgetType::PRIMARY,
            this->differentialHexViewerSharedState,
            this->settings,
            this->memoryDescriptor,
            this->hexViewerDataA,
            this->hexViewerWidgetSettingsA,
            this->focusedRegionsA,
            this->excludedRegionsA,
            this
        );

        this->hexViewerWidgetB = new DifferentialHexViewerWidget(
            DifferentialHexViewerWidgetType::SECONDARY,
            this->differentialHexViewerSharedState,
            this->settings,
            this->memoryDescriptor,
            this->hexViewerDataB,
            this->hexViewerWidgetSettingsB,
            this->focusedRegionsB,
            this->excludedRegionsB,
            this
        );

        this->hexViewerWidgetA->setObjectName("differential-hex-viewer-widget-a");
        this->hexViewerWidgetB->setObjectName("differential-hex-viewer-widget-b");

        this->hexViewerWidgetA->setStyleSheet(this->hexViewerWidgetA->styleSheet() + styleSheet);
        this->hexViewerWidgetB->setStyleSheet(this->hexViewerWidgetB->styleSheet() + styleSheet);

        snapshotAContainerLayout->addWidget(this->hexViewerWidgetA);
        snapshotBContainerLayout->addWidget(this->hexViewerWidgetB);

        this->changeListPane = new ChangeListPane(
            this->hexViewerWidgetA,
            this->hexViewerWidgetB,
            this->changeListPaneState,
            this->leftPanel
        );

        this->changeListPane->setRestoreEnabled(
            this->comparingWithCurrent
            && this->memoryDescriptor.access.writeableDuringDebugSession
            && this->targetState == Targets::TargetState::STOPPED
        );

        this->leftPanel->layout()->addWidget(this->changeListPane);
        this->viewChangeListButton->setChecked(this->changeListPane->state.activated);

        this->bottomBar = this->container->findChild<QWidget*>("bottom-bar");
        this->bottomBarLayout = this->bottomBar->findChild<QHBoxLayout*>();

        this->memoryCapacityLabel = this->bottomBar->findChild<Label*>("memory-capacity-label");
        this->memoryTypeLabel = this->bottomBar->findChild<Label*>("memory-type-label");
        this->diffCountLabel = this->bottomBar->findChild<Label*>("diff-count-label");

        this->memoryCapacityLabel->setText(QLocale(QLocale::English).toString(this->hexViewerDataA->size()) + " bytes");
        this->memoryTypeLabel->setText(EnumToStringMappings::targetMemoryTypes.at(
            this->memoryDescriptor.type).toUpper()
        );

        this->taskProgressIndicator = new TaskProgressIndicator(this);
        this->bottomBarLayout->insertWidget(7, this->taskProgressIndicator);

        this->setSyncHexViewerSettingsEnabled(this->settings.syncHexViewerSettings);
        this->setSyncHexViewerScrollEnabled(this->settings.syncHexViewerScroll);
        this->setSyncHexViewerHoverEnabled(this->settings.syncHexViewerHover);
        this->setSyncHexViewerSelectionEnabled(this->settings.syncHexViewerSelection);

        QObject::connect(
            this->viewChangeListButton,
            &QToolButton::clicked,
            this,
            &SnapshotDiff::toggleChangeListPane
        );

        QObject::connect(
            this->syncHexViewerSettingsButton,
            &QToolButton::clicked,
            this,
            [this] {
                this->setSyncHexViewerSettingsEnabled(!this->settings.syncHexViewerSettings);
            }
        );

        QObject::connect(
            this->syncHexViewerScrollButton,
            &QToolButton::clicked,
            this,
            [this] {
                this->setSyncHexViewerScrollEnabled(!this->settings.syncHexViewerScroll);
            }
        );

        QObject::connect(
            this->syncHexViewerHoverButton,
            &QToolButton::clicked,
            this,
            [this] {
                this->setSyncHexViewerHoverEnabled(!this->settings.syncHexViewerHover);
            }
        );

        QObject::connect(
            this->syncHexViewerSelectionButton,
            &QToolButton::clicked,
            this,
            [this] {
                this->setSyncHexViewerSelectionEnabled(!this->settings.syncHexViewerSelection);
            }
        );

        auto* insightSignals = InsightSignals::instance();

        QObject::connect(
            insightSignals,
            &InsightSignals::targetStateUpdated,
            this,
            &SnapshotDiff::onTargetStateChanged
        );

        QObject::connect(this->hexViewerWidgetA, &HexViewerWidget::ready, this, &SnapshotDiff::onHexViewerAReady);
        QObject::connect(this->hexViewerWidgetB, &HexViewerWidget::ready, this, &SnapshotDiff::onHexViewerBReady);

        this->refreshDifferences();

        this->hexViewerWidgetA->init();
        this->hexViewerWidgetB->init();

        this->move(this->parentWidget()->window()->geometry().center() - this->rect().center());
    }

    void SnapshotDiff::onHexViewerAReady() {
        this->hexViewerWidgetB->setOther(this->hexViewerWidgetA);

        if (this->restoreBytesAction != nullptr) {
            this->hexViewerWidgetA->addExternalContextMenuAction(this->restoreBytesAction);
        }

        if (this->memoryDescriptor.type == Targets::TargetMemoryType::RAM) {
            this->hexViewerWidgetA->setStackPointer(this->stackPointerA);
        }
    }

    void SnapshotDiff::onHexViewerBReady() {
        this->hexViewerWidgetA->setOther(this->hexViewerWidgetB);

        if (this->memoryDescriptor.type == Targets::TargetMemoryType::RAM) {
            this->hexViewerWidgetB->setStackPointer(this->stackPointerB);
        }
    }

    void SnapshotDiff::refreshDifferences() {
        using Targets::TargetMemoryAddressRange;

        assert(this->hexViewerDataA.has_value());
        assert(this->hexViewerDataB.has_value());

        this->differentialHexViewerSharedState.differences.clear();
        auto diffRanges = std::vector<TargetMemoryAddressRange>();

        const auto& dataA = *(this->hexViewerDataA);
        const auto& dataB = *(this->hexViewerDataB);

        const auto isAddressExcluded = [this] (Targets::TargetMemoryAddress address) {
            for (const auto& excludedRegion : this->excludedRegionsA) {
                if (excludedRegion.addressRange.contains(address)) {
                    return true;
                }
            }

            for (const auto& excludedRegion : this->excludedRegionsB) {
                if (excludedRegion.addressRange.contains(address)) {
                    return true;
                }
            }

            return false;
        };

        const auto& memoryStartAddress = this->memoryDescriptor.addressRange.startAddress;
        auto lastDiffRange = std::optional<TargetMemoryAddressRange>();

        for (Targets::TargetMemoryBuffer::size_type i = 0; i < dataA.size(); ++i) {
            const auto address = memoryStartAddress + static_cast<Targets::TargetMemoryAddress>(i);

            if (dataA[i] != dataB[i] && !isAddressExcluded(address)) {
                this->differentialHexViewerSharedState.differences.insert(address);

                if (lastDiffRange.has_value() && address > 0 && lastDiffRange->endAddress == (address - 1)) {
                    lastDiffRange->endAddress = address;

                } else {
                    if (lastDiffRange.has_value()) {
                        diffRanges.push_back(*lastDiffRange);
                    }

                    lastDiffRange = TargetMemoryAddressRange(address, address);
                }
            }
        }

        if (lastDiffRange.has_value()) {
            diffRanges.push_back(*lastDiffRange);
        }

        this->changeListPane->setDiffRanges(diffRanges);

        const auto diffCount = this->differentialHexViewerSharedState.differences.size();
        this->diffCountLabel->setText(
            diffCount == 0
                ? "Contents are identical"
                : QLocale(QLocale::English).toString(diffCount) + (diffCount == 1 ? " difference" : " differences")
        );
    }

    void SnapshotDiff::toggleChangeListPane() {
        if (!this->changeListPane->state.activated) {
            this->changeListPane->activate();
            this->viewChangeListButton->setChecked(true);
            return;
        }

        this->changeListPane->deactivate();
        this->viewChangeListButton->setChecked(false);
    }

    void SnapshotDiff::setSyncHexViewerSettingsEnabled(bool enabled) {
        this->settings.syncHexViewerSettings = enabled;
        this->syncHexViewerSettingsButton->setChecked(this->settings.syncHexViewerSettings);
    }

    void SnapshotDiff::setSyncHexViewerScrollEnabled(bool enabled) {
        this->settings.syncHexViewerScroll = enabled;
        this->syncHexViewerScrollButton->setChecked(this->settings.syncHexViewerScroll);
    }

    void SnapshotDiff::setSyncHexViewerHoverEnabled(bool enabled) {
        this->settings.syncHexViewerHover = enabled;
        this->syncHexViewerHoverButton->setChecked(this->settings.syncHexViewerHover);
    }

    void SnapshotDiff::setSyncHexViewerSelectionEnabled(bool enabled) {
        this->settings.syncHexViewerSelection = enabled;
        this->syncHexViewerSelectionButton->setChecked(this->settings.syncHexViewerSelection);
    }

    void SnapshotDiff::restoreSelectedBytes(
        std::set<Targets::TargetMemoryAddress> addresses,
        bool confirmationPromptEnabled
    ) {
        auto excludedAddresses = std::set<Targets::TargetMemoryAddress>();
        for (const auto& excludedRegion : this->excludedRegionsA) {
            const auto regionAddresses = excludedRegion.addressRange.addresses();
            excludedAddresses.insert(regionAddresses.begin(), regionAddresses.end());
        }

        std::erase_if(addresses, [excludedAddresses] (const auto& address) {
            return excludedAddresses.contains(address);
        });

        if (addresses.empty()) {
            // The user has only selected bytes that are within an excluded region - nothing to do here
            return;
        }

        if (confirmationPromptEnabled) {
            auto* confirmationDialog = new ConfirmationDialog(
                "Restore selected bytes",
                "This operation will write " + QString::number(addresses.size()) + " byte(s) to the target's "
                    + EnumToStringMappings::targetMemoryTypes.at(this->memoryDescriptor.type).toUpper()
                    + ".<br/><br/>Are you sure you want to proceed?",
                "Proceed",
                std::nullopt,
                this
            );

            QObject::connect(
                confirmationDialog,
                &ConfirmationDialog::confirmed,
                this,
                [this, addresses] {
                    this->restoreSelectedBytes(addresses, false);
                }
            );

            confirmationDialog->show();
            return;
        }

        auto writeBlocks = std::vector<WriteTargetMemory::Block>();

        Targets::TargetMemoryAddress blockStartAddress = *(addresses.begin());
        Targets::TargetMemoryAddress blockEndAddress = blockStartAddress;

        for (const auto& address : addresses) {
            if (address > (blockEndAddress + 1)) {
                // Commit the block
                const auto dataBeginOffset = blockStartAddress - this->memoryDescriptor.addressRange.startAddress;
                const auto dataEndOffset = blockEndAddress - this->memoryDescriptor.addressRange.startAddress + 1;

                writeBlocks.emplace_back(
                    blockStartAddress,
                    Targets::TargetMemoryBuffer(
                        this->hexViewerDataA->begin() + dataBeginOffset,
                        this->hexViewerDataA->begin() + dataEndOffset
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
                    this->hexViewerDataA->begin() + dataBeginOffset,
                    this->hexViewerDataA->begin() + dataEndOffset
                )
            );
        }

        const auto after = [this, writeBlocks] {
            auto& hexViewerDataB = this->hexViewerDataB.value();

            for (const auto& writeBlock : writeBlocks) {
                std::copy(
                    writeBlock.data.begin(),
                    writeBlock.data.end(),
                    hexViewerDataB.begin()
                        + (writeBlock.startAddress - this->memoryDescriptor.addressRange.startAddress)
                );
            }

            this->refreshDifferences();
            this->hexViewerWidgetA->updateValues();
            this->hexViewerWidgetB->updateValues();
        };

        const auto writeMemoryTask = QSharedPointer<WriteTargetMemory>(
            new WriteTargetMemory(this->memoryDescriptor, std::move(writeBlocks)),
            &QObject::deleteLater
        );

        QObject::connect(writeMemoryTask.get(), &WriteTargetMemory::targetMemoryWritten, this, after);

        this->taskProgressIndicator->addTask(writeMemoryTask);
        InsightWorker::queueTask(writeMemoryTask);
    }

    void SnapshotDiff::onTargetStateChanged(Targets::TargetState newState) {
        this->targetState = newState;

        this->changeListPane->setRestoreEnabled(
            this->comparingWithCurrent
            && this->memoryDescriptor.access.writeableDuringDebugSession
            && this->targetState == Targets::TargetState::STOPPED
        );
    }
}
