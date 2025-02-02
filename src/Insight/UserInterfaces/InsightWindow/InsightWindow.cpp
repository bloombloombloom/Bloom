#include "InsightWindow.hpp"

#include <QtSvg/QtSvg>
#include <QDesktopServices>
#include <utility>

#include "UiLoader.hpp"
#include "Widgets/RotatableLabel.hpp"

#include "Widgets/TargetWidgets/DIP/DualInlinePackageWidget.hpp"
#include "Widgets/TargetWidgets/QFP/QuadFlatPackageWidget.hpp"

#include "Widgets/TargetMemoryInspectionPane/ToolButton.hpp"

#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"
#include "src/Services/PathService.hpp"

#include "src/Insight/InsightSignals.hpp"
#include "src/Insight/InsightWorker/InsightWorker.hpp"

using namespace Exceptions;
using namespace Widgets;

using Targets::TargetDescriptor;
using Targets::TargetState;
using Targets::TargetExecutionState;
using Targets::TargetPinDescriptor;

InsightWindow::InsightWindow(
    InsightProjectSettings& insightProjectSettings,
    const InsightConfig& insightConfig,
    const EnvironmentConfig& environmentConfig,
    const TargetDescriptor& targetDescriptor,
    const TargetState& targetState
)
    : QMainWindow(nullptr)
    , insightProjectSettings(insightProjectSettings)
    , insightConfig(insightConfig)
    , environmentConfig(environmentConfig)
    , targetConfig(environmentConfig.targetConfig)
    , targetDescriptor(targetDescriptor)
    , targetState(targetState)
{
    this->setObjectName("main-window");
    this->setAttribute(Qt::WA_DeleteOnClose, true);
    this->setWindowTitle("Bloom Insight");

    constexpr auto defaultWindowSize = QSize{1000, 500};

    const auto windowSize = this->insightProjectSettings.mainWindowSize.has_value()
        ? QSize{
            std::max(this->insightProjectSettings.mainWindowSize->width(), defaultWindowSize.width()),
            std::max(this->insightProjectSettings.mainWindowSize->height(), defaultWindowSize.height())
        }
        : defaultWindowSize;

    this->setFixedSize(windowSize);
    this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    this->setMinimumSize(0, 0);

    auto mainWindowUiFile = QFile{
        QString::fromStdString(Services::PathService::compiledResourcesPath()
            + "/src/Insight/UserInterfaces/InsightWindow/UiFiles/InsightWindow.ui"
        )
    };
    auto mainWindowStylesheet = QFile{
        QString::fromStdString(Services::PathService::compiledResourcesPath()
            + "/src/Insight/UserInterfaces/InsightWindow/Stylesheets/InsightWindow.qss"
        )
    };

    if (!mainWindowUiFile.open(QFile::ReadOnly)) {
        throw Exception{"Failed to open InsightWindow UI file"};
    }

    if (!mainWindowStylesheet.open(QFile::ReadOnly)) {
        throw Exception{"Failed to open InsightWindow stylesheet file"};
    }

    auto uiLoader = UiLoader{this};
    this->windowContainer = uiLoader.load(&mainWindowUiFile, this);
    this->windowContainer->setStyleSheet(mainWindowStylesheet.readAll());

    mainWindowUiFile.close();
    mainWindowStylesheet.close();

    QApplication::setWindowIcon(QIcon{
        QString::fromStdString(Services::PathService::compiledResourcesPath()
            + "/src/Insight/UserInterfaces/InsightWindow/Images/bloom-icon.svg"
        )
    });

    this->layoutContainer = this->windowContainer->findChild<QWidget*>("layout-container");
    this->mainMenuBar = this->windowContainer->findChild<QMenuBar*>("menu-bar");
    this->layoutContainer->layout()->setMenuBar(this->mainMenuBar);
    this->container = this->layoutContainer->findChild<QWidget*>("container");
    this->ioContainerWidget = this->windowContainer->findChild<InsightTargetWidgets::TargetPackageWidgetContainer*>(
        "io-container"
    );

    auto* horizontalContentLayout = this->container->findChild<QHBoxLayout*>("horizontal-content-layout");
    auto* verticalContentLayout = this->container->findChild<QVBoxLayout*>("vertical-content-layout");

    auto* fileMenu = this->mainMenuBar->findChild<QMenu*>("file-menu");
    auto* helpMenu = this->mainMenuBar->findChild<QMenu*>("help-menu");
    auto* quitAction = fileMenu->findChild<QAction*>("close-insight");
    auto* openReportIssuesUrlAction = helpMenu->findChild<QAction*>("open-report-issues-url");
    auto* openGettingStartedUrlAction = helpMenu->findChild<QAction*>("open-getting-started-url");
    auto* openAboutWindowAction = helpMenu->findChild<QAction*>("open-about-dialogue");

    this->header = this->windowContainer->findChild<QWidget*>("header");
    this->refreshIoInspectionButton = this->header->findChild<SvgToolButton*>("refresh-io-inspection-btn");

    // Create panel states
    if (!this->insightProjectSettings.leftPanelState.has_value()) {
        this->insightProjectSettings.leftPanelState = PanelState{};
    }

    if (!this->insightProjectSettings.bottomPanelState.has_value()) {
        this->insightProjectSettings.bottomPanelState = PanelState{};
    }

    this->leftMenuBar = this->container->findChild<QWidget*>("left-side-menu-bar");
    this->leftPanel = new PanelWidget{
        PanelWidgetType::LEFT,
        this->insightProjectSettings.leftPanelState.value(),
        this->container
    };
    this->leftPanel->setObjectName("left-panel");
    this->leftPanel->setHandleSize(6);
    this->leftPanel->setMinimumResize(300);
    horizontalContentLayout->insertWidget(0, this->leftPanel);

    this->targetRegistersButton = this->container->findChild<QToolButton*>("target-registers-btn");
    auto* targetRegisterButtonLayout = this->targetRegistersButton->findChild<QVBoxLayout*>();
    auto* registersBtnLabel = new RotatableLabel{270, "Registers", this->targetRegistersButton};
    registersBtnLabel->setObjectName("target-registers-btn-label");
    registersBtnLabel->setContentsMargins(4, 4, 10, 2);
    targetRegisterButtonLayout->insertWidget(0, registersBtnLabel, 0, Qt::AlignTop);

    this->bottomMenuBar = this->container->findChild<QWidget*>("bottom-menu-bar");
    this->bottomMenuBarLayout = this->bottomMenuBar->findChild<QHBoxLayout*>();
    this->bottomPanel = new PanelWidget{
        PanelWidgetType::BOTTOM,
        this->insightProjectSettings.bottomPanelState.value(),
        this->container
    };
    this->bottomPanel->setObjectName("bottom-panel");
    this->bottomPanel->setHandleSize(10);
    verticalContentLayout->insertWidget(1, this->bottomPanel);

    this->footer = this->windowContainer->findChild<QWidget*>("footer");
    this->targetStatusLabel = this->footer->findChild<Label*>("target-state");
    this->programCounterValueLabel = this->footer->findChild<Label*>("target-program-counter-value");
    this->targetNameLabel = this->footer->findChild<Label*>("target-name");
    this->targetIdLabel = this->footer->findChild<Label*>("target-id");
    this->variantMenu = this->footer->findChild<QMenu*>("target-variant-menu");

    this->taskIndicator = new TaskIndicator{this};
    auto* footerLayout = this->footer->findChild<QHBoxLayout*>();
    footerLayout->insertWidget(2, this->taskIndicator);

    this->windowContainer->setFixedSize(windowSize);
    this->layoutContainer->setFixedSize(windowSize);

    // InsightSignal connections
    auto* insightSignals = InsightSignals::instance();

    QObject::connect(
        insightSignals,
        &InsightSignals::targetStateUpdated,
        this,
        &InsightWindow::onTargetStateUpdate
    );

    QObject::connect(
        insightSignals,
        &InsightSignals::programmingModeEnabled,
        this,
        &InsightWindow::onProgrammingModeEnabled
    );
    QObject::connect(
        insightSignals,
        &InsightSignals::programmingModeDisabled,
        this,
        &InsightWindow::onProgrammingModeDisabled
    );

    this->targetNameLabel->setText(QString::fromStdString(this->targetDescriptor.name));
    this->targetIdLabel->setText(QString::fromStdString(this->targetDescriptor.marketId));

    this->populateVariantMenu();
    this->variantMenu->setEnabled(true);
    this->selectDefaultVariant();

    this->createPanes();
    this->setUiDisabled(true);

    // Main menu connections
    QObject::connect(
        quitAction,
        &QAction::triggered,
        this,
        &InsightWindow::close
    );
    QObject::connect(
        openReportIssuesUrlAction,
        &QAction::triggered,
        this,
        &InsightWindow::openReportIssuesUrl
    );
    QObject::connect(
        openGettingStartedUrlAction,
        &QAction::triggered,
        this,
        &InsightWindow::openGettingStartedUrl
    );
    QObject::connect(
        openAboutWindowAction,
        &QAction::triggered,
        this,
        &InsightWindow::openAboutWindow
    );

    // Toolbar button connections
    QObject::connect(
        this->refreshIoInspectionButton,
        &QToolButton::clicked,
        this,
        &InsightWindow::refresh
    );

    // Panel connections
    QObject::connect(
        this->leftPanel,
        &PanelWidget::opened,
        this,
        &InsightWindow::adjustMinimumSize
    );
    QObject::connect(
        this->leftPanel,
        &PanelWidget::closed,
        this,
        &InsightWindow::adjustMinimumSize
    );
    QObject::connect(
        this->bottomPanel,
        &PanelWidget::opened,
        this,
        &InsightWindow::adjustMinimumSize
    );
    QObject::connect(
        this->bottomPanel,
        &PanelWidget::closed,
        this,
        &InsightWindow::adjustMinimumSize
    );

    // Panel button connections
    QObject::connect(
        this->targetRegistersButton,
        &QToolButton::clicked,
        this,
        &InsightWindow::toggleTargetRegistersPane
    );

    this->onTargetStateUpdate(this->targetState, {});
}

void InsightWindow::resizeEvent(QResizeEvent* event) {
    const auto windowSize = this->size();

    this->windowContainer->setFixedSize(windowSize);
    this->layoutContainer->setFixedSize(windowSize);

    this->adjustPanels();

    this->insightProjectSettings.mainWindowSize = windowSize;
}

void InsightWindow::showEvent(QShowEvent* event) {
    this->adjustPanels();
    this->adjustMinimumSize();
}

void InsightWindow::closeEvent(QCloseEvent* event) {
    return QMainWindow::closeEvent(event);
}

bool InsightWindow::isPinoutSupported(const Targets::TargetPinoutDescriptor& pinoutDescriptor) {
    using Targets::TargetPinoutType;

    const auto pinCount = pinoutDescriptor.pinDescriptors.size();

    if (pinCount > 100) {
        return false;
    }

    if (
        pinoutDescriptor.type != TargetPinoutType::DIP
        && pinoutDescriptor.type != TargetPinoutType::SOIC
        && pinoutDescriptor.type != TargetPinoutType::SSOP
        && pinoutDescriptor.type != TargetPinoutType::QFP
        && pinoutDescriptor.type != TargetPinoutType::QFN
    ) {
        return false;
    }

    if (
        (
            pinoutDescriptor.type == TargetPinoutType::DIP
            || pinoutDescriptor.type == TargetPinoutType::SOIC
            || pinoutDescriptor.type == TargetPinoutType::SSOP
        )
        && pinCount % 2 != 0
    ) {
        return false;
    }

    if (
        (pinoutDescriptor.type == TargetPinoutType::QFP || pinoutDescriptor.type == TargetPinoutType::QFN)
        && (pinCount % 4 != 0 || pinCount <= 4)
    ) {
        return false;
    }

    return true;
}

void InsightWindow::setUiDisabled(bool disable) {
    this->uiDisabled = disable;

    if (this->refreshIoInspectionButton != nullptr) {
        this->refreshIoInspectionButton->setDisabled(disable);
        this->refreshIoInspectionButton->repaint();
    }
}

void InsightWindow::populateVariantMenu() {
    for (const auto& [variantKey, variantDescriptor] : this->targetDescriptor.variantDescriptorsByKey) {
        const auto& pinoutDescriptor = this->targetDescriptor.getPinoutDescriptor(variantDescriptor.pinoutKey);

        auto* variantAction = new QAction{this->variantMenu};
        variantAction->setText(
            QString::fromStdString(variantDescriptor.name + " (" + pinoutDescriptor.name + ")")
        );

        if (InsightWindow::isPinoutSupported(pinoutDescriptor)) {
            QObject::connect(
                variantAction,
                &QAction::triggered,
                this,
                [this, &variantDescriptor] {
                    this->selectVariant(&variantDescriptor);

                    if (this->targetState.executionState == TargetExecutionState::STOPPED) {
                        this->refreshPadStates();
                    }
                }
            );

        } else {
            variantAction->setEnabled(false);
            variantAction->setText(variantAction->text() + " (unsupported)");
        }

        this->variantMenu->addAction(variantAction);
    }
}

void InsightWindow::selectDefaultVariant() {
    if (this->targetDescriptor.variantDescriptorsByKey.empty()) {
        return;
    }

    // Try the user provided variant key, if provided
    if (this->insightConfig.defaultVariantKey.has_value()) {
        const auto variantDescriptor = this->targetDescriptor.tryGetVariantDescriptor(
            *(this->insightConfig.defaultVariantKey)
        );

        if (variantDescriptor.has_value()) {
            const auto& descriptor = variantDescriptor->get();
            const auto& pinoutDescriptor = this->targetDescriptor.getPinoutDescriptor(descriptor.pinoutKey);

            if (InsightWindow::isPinoutSupported(pinoutDescriptor)) {
                this->selectVariant(&descriptor);
                return;

            } else {
                Logger::error(
                    "Unsupported target variant (\"" + descriptor.name
                        + "\") provided via 'defaultVariantKey' parameter"
                );
            }

        } else {
            Logger::error(
                "Invalid target variant key `" + *(this->insightConfig.defaultVariantKey)
                    + "` - no such variant with the given key was found"
            );
        }
    }

    // Try the previously selected variant
    if (this->insightProjectSettings.selectedVariantKey.has_value()) {
        const auto variantDescriptor = this->targetDescriptor.tryGetVariantDescriptor(
            *(this->insightProjectSettings.selectedVariantKey)
        );

        if (variantDescriptor.has_value()) {
            const auto& descriptor = variantDescriptor->get();
            const auto& pinoutDescriptor = this->targetDescriptor.getPinoutDescriptor(descriptor.pinoutKey);

            if (InsightWindow::isPinoutSupported(pinoutDescriptor)) {
                this->selectVariant(&descriptor);
                return;
            }
        }
    }

    /*
     * Given that we haven't been able to select a variant at this point, we will just fall back to the first
     * one that is available.
     */
    for (const auto& [variantKey, variantDescriptor] : this->targetDescriptor.variantDescriptorsByKey) {
        const auto& pinoutDescriptor = this->targetDescriptor.getPinoutDescriptor(variantDescriptor.pinoutKey);

        if (InsightWindow::isPinoutSupported(pinoutDescriptor)) {
            this->selectVariant(&variantDescriptor);
            return;
        }
    }

    Logger::error("Failed to select a target variant - no supported variants");
}

void InsightWindow::selectVariant(const Targets::TargetVariantDescriptor* variantDescriptor) {
    using Targets::TargetPinoutType;

    if (this->selectedVariantDescriptor != nullptr && this->selectedVariantDescriptor == variantDescriptor) {
        // The variant is already selected.
        return;
    }

    const auto& pinoutDescriptor = this->targetDescriptor.getPinoutDescriptor(variantDescriptor->pinoutKey);
    if (!InsightWindow::isPinoutSupported(pinoutDescriptor)) {
        Logger::error("Attempted to select unsupported target variant.");
        return;
    }

    if (this->targetPackageWidget != nullptr) {
        this->targetPackageWidget->hide();
        this->targetPackageWidget->deleteLater();
        this->targetPackageWidget = nullptr;
        this->ioContainerWidget->setPackageWidget(this->targetPackageWidget);
    }

    this->selectedVariantDescriptor = variantDescriptor;
    this->insightProjectSettings.selectedVariantKey = variantDescriptor->key;
    this->variantMenu->setTitle(QString::fromStdString(variantDescriptor->name + " (" + pinoutDescriptor.name + ")"));

    if (
        pinoutDescriptor.type == TargetPinoutType::DIP
        || pinoutDescriptor.type == TargetPinoutType::SOIC
        || pinoutDescriptor.type == TargetPinoutType::SSOP
    ) {
        this->targetPackageWidget = new InsightTargetWidgets::Dip::DualInlinePackageWidget{
            *variantDescriptor,
            pinoutDescriptor,
            this->targetDescriptor,
            this->targetState,
            this->ioContainerWidget
        };

    } else if (pinoutDescriptor.type == TargetPinoutType::QFP || pinoutDescriptor.type == TargetPinoutType::QFN) {
        this->targetPackageWidget = new InsightTargetWidgets::Qfp::QuadFlatPackageWidget{
            *variantDescriptor,
            pinoutDescriptor,
            this->targetDescriptor,
            this->targetState,
            this->ioContainerWidget
        };
    }

    if (this->targetPackageWidget != nullptr) {
        this->ioContainerWidget->setPackageWidget(this->targetPackageWidget);

        this->adjustPanels();
        this->adjustMinimumSize();
        this->targetPackageWidget->show();
    }
}

void InsightWindow::createPanes() {
    // Target registers pane
    if (!this->insightProjectSettings.registersPaneState.has_value()) {
        this->insightProjectSettings.registersPaneState = PaneState{false, true, std::nullopt};
    }

    auto* leftPanelLayout = this->leftPanel->layout();
    this->targetRegistersSidePane = new TargetRegistersPaneWidget{
        this->targetDescriptor,
        this->targetState,
        *(this->insightProjectSettings.registersPaneState),
        this->leftPanel
    };
    leftPanelLayout->addWidget(this->targetRegistersSidePane);

    QObject::connect(
        this->targetRegistersSidePane,
        &PaneWidget::paneActivated,
        this,
        &InsightWindow::onRegistersPaneStateChanged
    );
    QObject::connect(
        this->targetRegistersSidePane,
        &PaneWidget::paneDeactivated,
        this,
        &InsightWindow::onRegistersPaneStateChanged
    );

    this->targetRegistersButton->setDisabled(false);
    this->onRegistersPaneStateChanged();

    // Target memory inspection panes
    auto* bottomPanelLayout = this->bottomPanel->layout();

    for (const auto& [addressSpaceKey, addressSpaceDescriptor] : this->targetDescriptor.addressSpaceDescriptorsByKey) {
        for (const auto& [segmentKey, segmentDescriptor] : addressSpaceDescriptor.segmentDescriptorsByKey) {
            if (!segmentDescriptor.inspectionEnabled) {
                continue;
            }

            auto* inspectionPane = new TargetMemoryInspectionPane{
                addressSpaceDescriptor,
                segmentDescriptor,
                this->targetDescriptor,
                this->targetState,
                this->insightProjectSettings.findOrCreateMemoryInspectionPaneSettings(
                    QString::fromStdString(addressSpaceDescriptor.key),
                    QString::fromStdString(segmentDescriptor.key)
                ),
                this->insightProjectSettings.findOrCreateMemoryInspectionPaneState(
                    QString::fromStdString(addressSpaceDescriptor.key),
                    QString::fromStdString(segmentDescriptor.key)
                ),
                this->bottomPanel
            };

            this->memoryInspectionPaneWidgets.push_back(inspectionPane);
            bottomPanelLayout->addWidget(inspectionPane);

            auto* inspectionPaneBtn = new ToolButton{
                QString::fromStdString(segmentDescriptor.name),
                this->bottomMenuBar
            };

            QObject::connect(
                inspectionPaneBtn,
                &QToolButton::clicked,
                this,
                [this, inspectionPane] {
                    this->toggleMemoryInspectionPane(inspectionPane);
                }
            );

            const auto onPaneStateChanged = [this, inspectionPane, inspectionPaneBtn] {
                this->onMemoryInspectionPaneStateChanged(inspectionPane, inspectionPaneBtn);
            };

            QObject::connect(
                inspectionPane,
                &PaneWidget::paneActivated,
                this,
                onPaneStateChanged
            );
            QObject::connect(
                inspectionPane,
                &PaneWidget::paneDeactivated,
                this,
                onPaneStateChanged
            );
            QObject::connect(
                inspectionPane,
                &PaneWidget::paneAttached,
                this,
                onPaneStateChanged
            );

            this->bottomMenuBarLayout->insertWidget(1, inspectionPaneBtn);
            this->onMemoryInspectionPaneStateChanged(inspectionPane, inspectionPaneBtn);
        }
    }

    this->bottomPanel->updateVisibility();
}

void InsightWindow::adjustPanels() {
    const auto targetPackageWidgetSize = (this->targetPackageWidget != nullptr)
        ? this->targetPackageWidget->size() : QSize{};
    const auto containerSize = this->size();

    if (!this->isVisible()) {
        return;
    }

    /*
     * The purpose of the -20 is to ensure there is some padding between the panel borders and the
     * target package widget. Looks nicer with the padding.
     */
    this->leftPanel->setMaximumResize(
        std::max(
            this->leftPanel->getMinimumResize(),
            containerSize.width() - targetPackageWidgetSize.width() - this->leftMenuBar->width() - 20
        )
    );

    /*
     * Allow the bottom panel to overlap the target package widget (because the target package widget can
     * occupy a lot of space and become an annoyance if the bottom panel is restricted too much).
     */
    this->bottomPanel->setMaximumResize(
        std::max(
            this->bottomPanel->getMinimumResize(),
            (containerSize.height() / 2) - this->mainMenuBar->height() - this->bottomMenuBar->height() - 20
        )
    );

    this->bottomPanel->setMinimumResize(static_cast<int>(containerSize.height() * 0.25));
}

void InsightWindow::adjustMinimumSize() {
    static const auto absoluteMinimum = QSize{900, 400};

    /*
     * On X11, the QScreen::availableGeometry() function may return the full geometry of the screen, without
     * accounting for reserved areas for window managers and other decorations.
     *
     * Because of this, we always use QScreen::geometry() and account for reserved areas ourselves. It's near
     * impossible to do this accurately, so we just subtract 200 from the width and height, and hope that it's
     * enough.
     */
    const auto screenSize = this->screen()->availableGeometry().size();
    const auto absoluteMaximum = QSize{screenSize.width() - 200, screenSize.height() - 200};

    auto minSize = QSize{};

    if (this->targetPackageWidget != nullptr) {
        minSize.setWidth(this->targetPackageWidget->width() + 250);
        minSize.setHeight(this->targetPackageWidget->height() + 150);
    }

    if (this->leftPanel->isVisible()) {
        minSize.setWidth(minSize.width() + this->leftPanel->getMinimumResize());
    }

    if (this->bottomPanel->isVisible()) {
        minSize.setHeight(minSize.height() + this->bottomPanel->getMinimumResize());
    }

    this->setMinimumSize(
        std::min(std::max(minSize.width(), absoluteMinimum.width()), absoluteMaximum.width()),
        std::min(std::max(minSize.height(), absoluteMinimum.height()), absoluteMaximum.height())
    );
}

void InsightWindow::onTargetStateUpdate(TargetState newState, Targets::TargetState previousState) {
    const auto targetStopped = newState.executionState == TargetExecutionState::STOPPED;
    this->setUiDisabled(!targetStopped);

    if (this->targetPackageWidget != nullptr) {
        this->targetPackageWidget->setDisabled(!targetStopped);
    }

    switch (newState.executionState) {
        case TargetExecutionState::STOPPED: {
            this->targetStatusLabel->setText("Stopped");
            break;
        }
        case TargetExecutionState::RUNNING:
        case TargetExecutionState::STEPPING: {
            this->targetStatusLabel->setText("Running");
            break;
        }
        default: {
            this->targetStatusLabel->setText("Unknown");
            break;
        }
    }

    const auto pc = this->targetState.programCounter.load();
    this->programCounterValueLabel->setText(
        pc.has_value()
            ? "0x" + QString::number(*pc, 16).toUpper() + " (" + QString::number(*pc) + ")"
            : "-"
    );
}


void InsightWindow::refresh() {
    if (this->targetState.executionState != TargetExecutionState::STOPPED) {
        return;
    }

    this->refreshIoInspectionButton->startSpin();
    this->refreshIoInspectionButton->setDisabled(true);

    if (this->targetPackageWidget != nullptr) {
        this->refreshPadStates();
    }

    const auto callback = [this] {
        this->refreshIoInspectionButton->stopSpin();
        this->refreshIoInspectionButton->setDisabled(
                this->targetState.executionState != TargetExecutionState::STOPPED
        );
    };

    if (this->targetRegistersSidePane != nullptr && this->targetRegistersSidePane->state.activated) {
        this->targetRegistersSidePane->refreshRegisterValues(std::nullopt, callback);

    } else {
        callback();
    }
}

void InsightWindow::refreshPadStates() {
    this->targetPackageWidget->setDisabled(true);

    this->targetPackageWidget->refreshPadStates([this] {
        if (this->targetState.executionState == TargetExecutionState::STOPPED) {
            this->targetPackageWidget->setDisabled(false);
        }
    });
}

void InsightWindow::openReportIssuesUrl() {
    auto url = QUrl{QString::fromStdString(Services::PathService::homeDomainName() + "/report-issue")};
    /*
     * The https://bloom.oscillate.io/report-issue URL just redirects to the Bloom GitHub issue page.
     *
     * We can use query parameters in the URL to pre-fill the body of the issue. We use this to include some
     * target information.
     */
    auto urlQuery = QUrlQuery{};
    auto issueBody = QString{
        "Issue reported via Bloom Insight.\nTarget name: " + QString::fromStdString(this->targetDescriptor.name) + "\n"
    };

    if (this->selectedVariantDescriptor != nullptr) {
        issueBody += "Target variant: " + QString::fromStdString(this->selectedVariantDescriptor->name) + "\n";
    }

    issueBody += "\nPlease describe your issue below. Include as much detail as possible.";
    urlQuery.addQueryItem("body", issueBody);
    url.setQuery(urlQuery);

    QDesktopServices::openUrl(url);
}

void InsightWindow::openGettingStartedUrl() {
    QDesktopServices::openUrl(
        QUrl{QString::fromStdString(Services::PathService::homeDomainName() + "/docs/getting-started")}
    );
}

void InsightWindow::openAboutWindow() {
    if (this->aboutWindowWidget == nullptr) {
        this->aboutWindowWidget = new AboutWindow{this};
    }

    this->aboutWindowWidget->show();
}

void InsightWindow::toggleTargetRegistersPane() {
    if (this->targetRegistersSidePane->state.activated) {
        this->targetRegistersSidePane->deactivate();

    } else {
        this->targetRegistersSidePane->activate();
    }
}

void InsightWindow::toggleMemoryInspectionPane(Widgets::TargetMemoryInspectionPane* pane) {
    if (pane->state.activated) {
        if (!pane->state.attached) {
            pane->activateWindow();

            return;
        }

        pane->deactivate();
        return;
    }

    pane->activate();
}

void InsightWindow::onRegistersPaneStateChanged() {
    this->targetRegistersButton->setChecked(this->targetRegistersSidePane->state.activated);

    if (
        this->targetState.executionState == TargetExecutionState::STOPPED
        && this->targetRegistersSidePane->state.activated
    ) {
        this->targetRegistersSidePane->refreshRegisterValues();
    }
}

void InsightWindow::onMemoryInspectionPaneStateChanged(
    Widgets::TargetMemoryInspectionPane* pane,
    Widgets::ToolButton* toolBtn
) {
    toolBtn->setChecked(pane->state.activated);

    if (pane->state.activated && pane->state.attached) {
        // Deactivate other attached memory inspection panes
        for (auto* otherPane : this->memoryInspectionPaneWidgets) {
            if (otherPane == pane) {
                continue;
            }

            if (otherPane->state.activated && otherPane->state.attached) {
                otherPane->deactivate();
            }
        }
    }
}

void InsightWindow::onProgrammingModeEnabled() {
    this->targetStatusLabel->setText("Programming Mode Enabled");
    this->programCounterValueLabel->setText("-");
}

void InsightWindow::onProgrammingModeDisabled() {
    this->onTargetStateUpdate(this->targetState, this->targetState);
}
