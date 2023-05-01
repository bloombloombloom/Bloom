#include "SnapshotDiff.hpp"

#include <QFile>
#include <QVBoxLayout>
#include <algorithm>

#include "src/Insight/InsightWorker/Tasks/WriteTargetMemory.hpp"
#include "src/Insight/InsightWorker/InsightWorker.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/UiLoader.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ConfirmationDialog.hpp"

#include "src/Services/PathService.hpp"
#include "src/Exceptions/Exception.hpp"

#include "src/Insight/InsightSignals.hpp"
#include "src/Logger/Logger.hpp"

namespace Bloom::Widgets
{
    using Bloom::Exceptions::Exception;

    SnapshotDiff::SnapshotDiff(
        MemorySnapshot& snapshotA,
        MemorySnapshot& snapshotB,
        const Targets::TargetMemoryDescriptor& memoryDescriptor,
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
    {
        this->init();

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
        QWidget* parent
    )
        : QWidget(parent)
        , memoryDescriptor(memoryDescriptor)
        , hexViewerDataA(snapshotA.data)
        , focusedRegionsA(snapshotA.focusedRegions)
        , excludedRegionsA(snapshotA.excludedRegions)
        , stackPointerA(snapshotA.stackPointer)
        , hexViewerDataB(dataB)
        , focusedRegionsB(focusedRegionsB)
        , excludedRegionsB(excludedRegionsB)
        , stackPointerB(stackPointerB)
    {
        this->init();

        this->setWindowTitle(
            "Comparing snapshot \"" + snapshotA.name + "\" with current memory"
        );

        this->dataAPrimaryLabel->setText(snapshotA.name);
        this->dataBPrimaryLabel->setText("Current");
        this->dataASecondaryLabel->setText(snapshotA.createdDate.toString("dd/MM/yyyy hh:mm"));
        this->dataBSecondaryLabel->setVisible(false);
    }

    void SnapshotDiff::showEvent(QShowEvent* event) {
        QWidget::showEvent(event);
    }

    void SnapshotDiff::resizeEvent(QResizeEvent* event) {
        this->container->setFixedSize(this->size());

        QWidget::resizeEvent(event);
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
        this->setMinimumSize(700, 600);
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

        this->restoreBytesAction = new ContextMenuAction("Restore Selection", std::nullopt, this);

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

        this->bottomBar = this->container->findChild<QWidget*>("bottom-bar");
        this->bottomBarLayout = this->bottomBar->findChild<QHBoxLayout*>();

        this->setSyncHexViewerSettingsEnabled(this->settings.syncHexViewerSettings);
        this->setSyncHexViewerScrollEnabled(this->settings.syncHexViewerScroll);
        this->setSyncHexViewerHoverEnabled(this->settings.syncHexViewerHover);
        this->setSyncHexViewerSelectionEnabled(this->settings.syncHexViewerSelection);

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

        QObject::connect(this->hexViewerWidgetA, &HexViewerWidget::ready, this, &SnapshotDiff::onHexViewerAReady);
        QObject::connect(this->hexViewerWidgetB, &HexViewerWidget::ready, this, &SnapshotDiff::onHexViewerBReady);

        this->hexViewerWidgetA->init();
        this->hexViewerWidgetB->init();

        this->move(this->parentWidget()->window()->geometry().center() - this->rect().center());
    }

    void SnapshotDiff::onHexViewerAReady() {
        this->hexViewerWidgetB->setOther(this->hexViewerWidgetA);

//        this->hexViewerWidgetA->addExternalContextMenuAction(this->restoreBytesAction);
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
}
