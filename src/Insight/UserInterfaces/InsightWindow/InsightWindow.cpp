#include "InsightWindow.hpp"

#include <QtSvg/QtSvg>
#include <QDesktopServices>
#include <utility>

#include "UiLoader.hpp"
#include "Widgets/RotatableLabel.hpp"

#include "Widgets/TargetWidgets/DIP/DualInlinePackageWidget.hpp"
#include "Widgets/TargetWidgets/QFP/QuadFlatPackageWidget.hpp"

#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"
#include "src/Services/PathService.hpp"

#include "src/Targets/TargetMemory.hpp"

#include "src/Insight/InsightSignals.hpp"
#include "src/Insight/InsightWorker/InsightWorker.hpp"
#include "src/Insight/InsightWorker/Tasks/ReadProgramCounter.hpp"

namespace Bloom
{
    using namespace Bloom::Exceptions;
    using namespace Bloom::Widgets;

    using Bloom::Targets::TargetDescriptor;
    using Bloom::Targets::TargetState;
    using Bloom::Targets::TargetPinState;
    using Bloom::Targets::TargetVariant;
    using Bloom::Targets::TargetPackage;
    using Bloom::Targets::TargetPinDescriptor;
    using Bloom::Targets::TargetMemoryType;

    InsightWindow::InsightWindow(
        const EnvironmentConfig& environmentConfig,
        const InsightConfig& insightConfig,
        InsightProjectSettings& insightProjectSettings,
        const Targets::TargetDescriptor& targetDescriptor
    )
        : QMainWindow(nullptr)
        , environmentConfig(environmentConfig)
        , targetConfig(environmentConfig.targetConfig)
        , insightConfig(insightConfig)
        , insightProjectSettings(insightProjectSettings)
        , targetDescriptor(targetDescriptor)
    {
        this->setObjectName("main-window");
        this->setWindowTitle("Bloom Insight");

        const auto defaultMinimumSize = QSize(1000, 500);

        if (this->insightProjectSettings.mainWindowSize.has_value()) {
            this->setMinimumSize(
                std::max(this->insightProjectSettings.mainWindowSize->width(), defaultMinimumSize.width()),
                std::max(this->insightProjectSettings.mainWindowSize->height(), defaultMinimumSize.height())
            );

        } else {
            this->setMinimumSize(defaultMinimumSize);
        }

        auto mainWindowUiFile = QFile(
            QString::fromStdString(Services::PathService::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/UiFiles/InsightWindow.ui"
            )
        );
        auto mainWindowStylesheet = QFile(
            QString::fromStdString(Services::PathService::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Stylesheets/InsightWindow.qss"
            )
        );

        if (!mainWindowUiFile.open(QFile::ReadOnly)) {
            throw Exception("Failed to open InsightWindow UI file");
        }

        if (!mainWindowStylesheet.open(QFile::ReadOnly)) {
            throw Exception("Failed to open InsightWindow stylesheet file");
        }

        auto uiLoader = UiLoader(this);
        this->windowContainer = uiLoader.load(&mainWindowUiFile, this);
        this->windowContainer->setStyleSheet(mainWindowStylesheet.readAll());

        mainWindowUiFile.close();
        mainWindowStylesheet.close();

        QApplication::setWindowIcon(QIcon(
            QString::fromStdString(Services::PathService::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Images/bloom-icon.svg"
            )
        ));

        this->layoutContainer = this->windowContainer->findChild<QWidget*>("layout-container");
        this->mainMenuBar = this->windowContainer->findChild<QMenuBar*>("menu-bar");
        this->layoutContainer->layout()->setMenuBar(this->mainMenuBar);
        this->container = this->layoutContainer->findChild<QWidget*>("container");
        this->ioContainerWidget = this->windowContainer->findChild<InsightTargetWidgets::TargetPackageWidgetContainer*>(
            "io-container"
        );
        this->ioUnavailableWidget = this->windowContainer->findChild<Label*>("io-inspection-unavailable");

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
            this->insightProjectSettings.leftPanelState = PanelState();
        }

        if (!this->insightProjectSettings.bottomPanelState.has_value()) {
            this->insightProjectSettings.bottomPanelState = PanelState();
        }

        this->leftMenuBar = this->container->findChild<QWidget*>("left-side-menu-bar");
        this->leftPanel = new PanelWidget(
            PanelWidgetType::LEFT,
            this->insightProjectSettings.leftPanelState.value(),
            this->container
        );
        this->leftPanel->setObjectName("left-panel");
        this->leftPanel->setHandleSize(6);
        this->leftPanel->setMinimumResize(300);
        horizontalContentLayout->insertWidget(0, this->leftPanel);

        this->targetRegistersButton = this->container->findChild<QToolButton*>("target-registers-btn");
        auto* targetRegisterButtonLayout = this->targetRegistersButton->findChild<QVBoxLayout*>();
        auto* registersBtnLabel = new RotatableLabel(270, "Registers", this->targetRegistersButton);
        registersBtnLabel->setObjectName("target-registers-btn-label");
        registersBtnLabel->setContentsMargins(4, 4, 10, 2);
        targetRegisterButtonLayout->insertWidget(0, registersBtnLabel, 0, Qt::AlignTop);

        this->bottomMenuBar = this->container->findChild<QWidget*>("bottom-menu-bar");
        this->bottomPanel = new PanelWidget(
            PanelWidgetType::BOTTOM,
            this->insightProjectSettings.bottomPanelState.value(),
            this->container
        );
        this->bottomPanel->setObjectName("bottom-panel");
        this->bottomPanel->setHandleSize(10);
        verticalContentLayout->insertWidget(1, this->bottomPanel);

        this->ramInspectionButton = this->container->findChild<QToolButton*>("ram-inspection-btn");
        this->eepromInspectionButton = this->container->findChild<QToolButton*>("eeprom-inspection-btn");
        this->flashInspectionButton = this->container->findChild<QToolButton*>("flash-inspection-btn");

        this->footer = this->windowContainer->findChild<QWidget*>("footer");
        this->targetStatusLabel = this->footer->findChild<Label*>("target-state");
        this->programCounterValueLabel = this->footer->findChild<Label*>("target-program-counter-value");
        this->targetNameLabel = this->footer->findChild<Label*>("target-name");
        this->targetIdLabel = this->footer->findChild<Label*>("target-id");
        this->variantMenu = this->footer->findChild<QMenu*>("target-variant-menu");

        this->taskIndicator = new TaskIndicator(this);
        auto* footerLayout = this->footer->findChild<QHBoxLayout*>();
        footerLayout->insertWidget(2, this->taskIndicator);

        const auto windowSize = this->size();
        this->windowContainer->setFixedSize(windowSize);
        this->layoutContainer->setFixedSize(windowSize);

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
        QObject::connect(
            this->ramInspectionButton,
            &QToolButton::clicked,
            this,
            &InsightWindow::toggleRamInspectionPane
        );
        QObject::connect(
            this->eepromInspectionButton,
            &QToolButton::clicked,
            this,
            &InsightWindow::toggleEepromInspectionPane
        );
        QObject::connect(
            this->flashInspectionButton,
            &QToolButton::clicked,
            this,
            &InsightWindow::toggleFlashInspectionPane
        );

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
            &InsightSignals::targetReset,
            this,
            [this] {
                this->refreshProgramCounter();
            }
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
    }

    void InsightWindow::init(TargetDescriptor targetDescriptor) {
        this->targetDescriptor = std::move(targetDescriptor);
        this->activate();
    }

    void InsightWindow::resizeEvent(QResizeEvent* event) {
        const auto windowSize = this->size();

        this->windowContainer->setFixedSize(windowSize);
        this->layoutContainer->setFixedSize(windowSize);

        this->adjustPanels();

        this->insightProjectSettings.mainWindowSize = windowSize;
    }

    void InsightWindow::showEvent(QShowEvent* event) {
        if (!this->activated) {
            this->activate();
        }

        this->adjustPanels();
        this->adjustMinimumSize();
    }

    void InsightWindow::closeEvent(QCloseEvent* event) {
        if (this->activated) {
            this->deactivate();
        }

        return QMainWindow::closeEvent(event);
    }

    bool InsightWindow::isVariantSupported(const TargetVariant& variant) {
        const auto pinCount = variant.pinDescriptorsByNumber.size();

        /*
         * Because the size of the pin body widget is fixed, for all of our target package widgets, we run out of screen
         * estate for target variants with more than 100 pins.
         *
         * This will be addressed at some point, but for now, we just won't support variants with more than 100 pins.
         *
         * I don't think there are any AVR targets with variants with more than 100 pins, so this isn't a problem anyway.
         */
        if (pinCount > 100) {
            return false;
        }

        if (
            variant.package != TargetPackage::DIP
            && variant.package != TargetPackage::SOIC
            && variant.package != TargetPackage::SSOP
            && variant.package != TargetPackage::QFP
            && variant.package != TargetPackage::QFN
        ) {
            return false;
        }

        if (
            (
                variant.package == TargetPackage::DIP
                || variant.package == TargetPackage::SOIC
                || variant.package == TargetPackage::SSOP
            )
            && pinCount % 2 != 0
        ) {
            // All DIP, SOIC and SSOP variants must have a pin count that is a multiple of two
            return false;
        }

        if (
            (variant.package == TargetPackage::QFP || variant.package == TargetPackage::QFN)
            && (pinCount % 4 != 0 || pinCount <= 4)
        ) {
            /*
             * All QFP and QFN variants must have a pin count that is a multiple of four. And there must be
             * more than one pin per side.
             */
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

    void InsightWindow::activate() {
        this->targetNameLabel->setText(QString::fromStdString(this->targetDescriptor.name));
        this->targetIdLabel->setText("0x" + QString::fromStdString(this->targetDescriptor.id).remove("0x").toUpper());

        this->ioUnavailableWidget->hide();

        this->populateVariantMenu();
        this->variantMenu->setEnabled(true);

        Logger::debug("Number of target variants supported by Insight: "
            + std::to_string(supportedVariantsByName.size()));

        if (this->supportedVariantsByName.empty()) {
            if (this->targetDescriptor.variants.empty()) {
                this->variantMenu->parentWidget()->hide();
            }

            this->ioUnavailableWidget->setText(
                "GPIO inspection is not available for this target. "
                "Please report this to Bloom developers by clicking Help -> Report An Issue"
            );
            this->ioUnavailableWidget->show();

        } else {
            this->selectDefaultVariant();
        }

        this->createPanes();

        this->setUiDisabled(this->targetState != TargetState::STOPPED);
        this->activated = true;
        emit this->activatedSignal();
    }

    void InsightWindow::populateVariantMenu() {
        /*
         * We don't want to present the user with duplicate target variants.
         *
         * In the context of the Insight window, two variants are considered to be duplicates if they do not differ in
         * package type and pinout configuration.
         */
        auto processedVariants = std::vector<TargetVariant>();
        const auto isDuplicateVariant = [&processedVariants] (const TargetVariant& variantA) {
            return std::ranges::any_of(
                processedVariants.begin(),
                processedVariants.end(),
                [&variantA, &processedVariants] (const TargetVariant& variantB) {
                    if (variantA.package != variantB.package) {
                        return false;
                    }

                    if (variantA.pinDescriptorsByNumber.size() != variantB.pinDescriptorsByNumber.size()) {
                        return false;
                    }

                    if (variantA.pinDescriptorsByNumber != variantB.pinDescriptorsByNumber) {
                        return false;
                    }

                    return true;
                }
            );
        };

        for (const auto& targetVariant: this->targetDescriptor.variants) {
            if (isDuplicateVariant(targetVariant)) {
                continue;
            }

            auto* variantAction = new QAction(this->variantMenu);
            variantAction->setText(
                QString::fromStdString(targetVariant.name + " (" + targetVariant.packageName + ")")
            );

            if (InsightWindow::isVariantSupported(targetVariant)) {
                auto* supportedVariantPtr = &(this->supportedVariantsByName.insert(
                    std::pair(QString::fromStdString(targetVariant.name).toLower(), targetVariant)
                ).first->second);

                QObject::connect(
                    variantAction,
                    &QAction::triggered,
                    this,
                    [this, supportedVariantPtr] {
                        this->selectVariant(supportedVariantPtr);
                    }
                );

            } else {
                variantAction->setEnabled(false);
                variantAction->setText(variantAction->text() + " (unsupported)");
            }

            this->variantMenu->addAction(variantAction);
            processedVariants.push_back(targetVariant);
        }
    }

    void InsightWindow::selectDefaultVariant() {
        if (this->supportedVariantsByName.empty()) {
            return;
        }

        std::optional<QString> previouslySelectedVariantName = (this->previouslySelectedVariant.has_value())
            ? std::optional(QString::fromStdString(this->previouslySelectedVariant->name).toLower())
            : std::nullopt;

        if (previouslySelectedVariantName.has_value()) {
            const auto previouslySelectedVariantIt = this->supportedVariantsByName.find(*previouslySelectedVariantName);

            if (previouslySelectedVariantIt != this->supportedVariantsByName.end()) {
                this->selectVariant(&(previouslySelectedVariantIt->second));
                return;
            }
        }

        if (this->targetConfig.variantName.has_value()) {
            const auto variantIt = this->supportedVariantsByName.find(
                QString::fromStdString(*this->targetConfig.variantName)
            );

            if (variantIt != this->supportedVariantsByName.end()) {
                // The user has specified a valid variant name in their config file, so use that as the default
                this->selectVariant(&(variantIt->second));

            } else {
                Logger::error(
                    "Invalid target variant name \"" + this->targetConfig.variantName.value()
                        + "\" - no such variant with the given name was found."
                );
            }
        }

        /*
         * Given that we haven't been able to select a variant at this point, we will just fall back to the first
         * one that is available.
         */
        this->selectVariant(&(this->supportedVariantsByName.begin()->second));
    }

    void InsightWindow::selectVariant(const TargetVariant* variant) {
        if (!this->isVariantSupported(*variant)) {
            Logger::error("Attempted to select unsupported target variant.");
            return;
        }

        if (this->selectedVariant != nullptr && this->selectedVariant->id == variant->id) {
            // The variant is already selected.
            return;
        }

        if (this->targetPackageWidget != nullptr) {
            this->targetPackageWidget->hide();
            this->targetPackageWidget->deleteLater();
            this->targetPackageWidget = nullptr;
            this->ioContainerWidget->setPackageWidget(this->targetPackageWidget);
        }

        this->selectedVariant = variant;
        this->variantMenu->setTitle(QString::fromStdString(variant->name + " (" + variant->packageName + ")"));

        if (
            variant->package == TargetPackage::DIP
            || variant->package == TargetPackage::SOIC
            || variant->package == TargetPackage::SSOP
        ) {
            this->targetPackageWidget = new InsightTargetWidgets::Dip::DualInlinePackageWidget(
                *variant,
                this->ioContainerWidget
            );

        } else if (variant->package == TargetPackage::QFP || variant->package == TargetPackage::QFN) {
            this->targetPackageWidget = new InsightTargetWidgets::Qfp::QuadFlatPackageWidget(
                *variant,
                this->ioContainerWidget
            );
        }

        if (this->targetPackageWidget != nullptr) {
            this->ioContainerWidget->setPackageWidget(this->targetPackageWidget);
            this->targetPackageWidget->setTargetState(this->targetState);

            if (this->targetState == TargetState::STOPPED) {
                this->refreshPinStates();
            }

            this->adjustPanels();
            this->adjustMinimumSize();
            this->targetPackageWidget->show();
        }
    }

    void InsightWindow::createPanes() {
        // Target registers pane
        if (!this->insightProjectSettings.registersPaneState.has_value()) {
            this->insightProjectSettings.registersPaneState = PaneState(false, true, std::nullopt);
        }

        auto* leftPanelLayout = this->leftPanel->layout();
        this->targetRegistersSidePane = new TargetRegistersPaneWidget(
            this->targetDescriptor,
            *(this->insightProjectSettings.registersPaneState),
            this->leftPanel
        );
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

        auto& memoryInspectionPaneSettingsByMemoryType =
            this->insightProjectSettings.memoryInspectionPaneSettingsByMemoryType;

        // Target memory inspection panes
        auto* bottomPanelLayout = this->bottomPanel->layout();

        const auto ramDescriptorIt = this->targetDescriptor.memoryDescriptorsByType.find(TargetMemoryType::RAM);
        const auto eepromDescriptorIt = this->targetDescriptor.memoryDescriptorsByType.find(TargetMemoryType::EEPROM);
        const auto flashDescriptorIt = this->targetDescriptor.memoryDescriptorsByType.find(TargetMemoryType::FLASH);

        if (ramDescriptorIt != this->targetDescriptor.memoryDescriptorsByType.end()) {
            if (!this->insightProjectSettings.ramInspectionPaneState.has_value()) {
                this->insightProjectSettings.ramInspectionPaneState = PaneState(false, true, std::nullopt);
            }

            if (!memoryInspectionPaneSettingsByMemoryType.contains(TargetMemoryType::RAM)) {
                memoryInspectionPaneSettingsByMemoryType[TargetMemoryType::RAM] = TargetMemoryInspectionPaneSettings();
            }

            this->ramInspectionPane = new TargetMemoryInspectionPane(
                ramDescriptorIt->second,
                memoryInspectionPaneSettingsByMemoryType[TargetMemoryType::RAM],
                *(this->insightProjectSettings.ramInspectionPaneState),
                this->bottomPanel
            );

            bottomPanelLayout->addWidget(this->ramInspectionPane);

            QObject::connect(
                this->ramInspectionPane,
                &PaneWidget::paneActivated,
                this,
                &InsightWindow::onRamInspectionPaneStateChanged
            );
            QObject::connect(
                this->ramInspectionPane,
                &PaneWidget::paneDeactivated,
                this,
                &InsightWindow::onRamInspectionPaneStateChanged
            );
            QObject::connect(
                this->ramInspectionPane,
                &PaneWidget::paneAttached,
                this,
                &InsightWindow::onRamInspectionPaneStateChanged
            );

            this->ramInspectionButton->setDisabled(false);
            this->onRamInspectionPaneStateChanged();
        }

        if (eepromDescriptorIt != this->targetDescriptor.memoryDescriptorsByType.end()) {
            if (!this->insightProjectSettings.eepromInspectionPaneState.has_value()) {
                this->insightProjectSettings.eepromInspectionPaneState = PaneState(false, true, std::nullopt);
            }

            if (!memoryInspectionPaneSettingsByMemoryType.contains(TargetMemoryType::EEPROM)) {
                memoryInspectionPaneSettingsByMemoryType[TargetMemoryType::EEPROM] = TargetMemoryInspectionPaneSettings();
            }

            this->eepromInspectionPane = new TargetMemoryInspectionPane(
                eepromDescriptorIt->second,
                memoryInspectionPaneSettingsByMemoryType[TargetMemoryType::EEPROM],
                *(this->insightProjectSettings.eepromInspectionPaneState),
                this->bottomPanel
            );

            bottomPanelLayout->addWidget(this->eepromInspectionPane);

            QObject::connect(
                this->eepromInspectionPane,
                &PaneWidget::paneActivated,
                this,
                &InsightWindow::onEepromInspectionPaneStateChanged
            );
            QObject::connect(
                this->eepromInspectionPane,
                &PaneWidget::paneDeactivated,
                this,
                &InsightWindow::onEepromInspectionPaneStateChanged
            );
            QObject::connect(
                this->eepromInspectionPane,
                &PaneWidget::paneAttached,
                this,
                &InsightWindow::onEepromInspectionPaneStateChanged
            );

            this->eepromInspectionButton->setDisabled(false);
            this->onEepromInspectionPaneStateChanged();
        }

        if (flashDescriptorIt != this->targetDescriptor.memoryDescriptorsByType.end()) {
            if (!this->insightProjectSettings.flashInspectionPaneState.has_value()) {
                this->insightProjectSettings.flashInspectionPaneState = PaneState(false, true, std::nullopt);
            }

            if (!memoryInspectionPaneSettingsByMemoryType.contains(TargetMemoryType::FLASH)) {
                memoryInspectionPaneSettingsByMemoryType[TargetMemoryType::FLASH] = TargetMemoryInspectionPaneSettings();
            }

            this->flashInspectionPane = new TargetMemoryInspectionPane(
                flashDescriptorIt->second,
                memoryInspectionPaneSettingsByMemoryType[TargetMemoryType::FLASH],
                *(this->insightProjectSettings.flashInspectionPaneState),
                this->bottomPanel
            );

            bottomPanelLayout->addWidget(this->flashInspectionPane);

            QObject::connect(
                this->flashInspectionPane,
                &PaneWidget::paneActivated,
                this,
                &InsightWindow::onFlashInspectionPaneStateChanged
            );
            QObject::connect(
                this->flashInspectionPane,
                &PaneWidget::paneDeactivated,
                this,
                &InsightWindow::onFlashInspectionPaneStateChanged
            );
            QObject::connect(
                this->flashInspectionPane,
                &PaneWidget::paneAttached,
                this,
                &InsightWindow::onFlashInspectionPaneStateChanged
            );

            this->flashInspectionButton->setDisabled(false);
            this->onFlashInspectionPaneStateChanged();
        }
    }

    void InsightWindow::destroyPanes() {
        if (this->targetRegistersSidePane != nullptr) {
            this->targetRegistersSidePane->deactivate();
            this->targetRegistersSidePane->deleteLater();
            this->targetRegistersSidePane = nullptr;

            this->leftPanel->setVisible(false);
            this->targetRegistersButton->setChecked(false);
            this->targetRegistersButton->setDisabled(true);
        }

        /*
         * Before we destroy the memory inspection pane widgets, we take a copy of their current settings (memory
         * regions, hex viewer settings, etc), in order to persist them through debug sessions.
         */
        if (this->ramInspectionPane != nullptr) {
            this->ramInspectionPane->deactivate();
            this->ramInspectionPane->deleteLater();
            this->ramInspectionPane = nullptr;

            this->bottomPanel->setVisible(false);
            this->ramInspectionButton->setChecked(false);
            this->ramInspectionButton->setDisabled(true);
        }

        if (this->eepromInspectionPane != nullptr) {
            this->eepromInspectionPane->deactivate();
            this->eepromInspectionPane->deleteLater();
            this->eepromInspectionPane = nullptr;

            this->bottomPanel->setVisible(false);
            this->eepromInspectionButton->setChecked(false);
            this->eepromInspectionButton->setDisabled(true);
        }

        if (this->flashInspectionPane != nullptr) {
            this->flashInspectionPane->deactivate();
            this->flashInspectionPane->deleteLater();
            this->flashInspectionPane = nullptr;

            this->bottomPanel->setVisible(false);
            this->flashInspectionButton->setChecked(false);
            this->flashInspectionButton->setDisabled(true);
        }
    }

    void InsightWindow::deactivate() {
        const auto insightProjectSettings = this->insightProjectSettings;
        const auto childWidgets = this->findChildren<QWidget*>();

        for (auto* widget : childWidgets) {
            if (widget->windowFlags() & Qt::Window) {
                widget->close();
            }
        }

        if (this->selectedVariant != nullptr) {
            this->previouslySelectedVariant = *(this->selectedVariant);
            this->selectedVariant = nullptr;
        }

        if (this->targetPackageWidget != nullptr) {
            this->targetPackageWidget->hide();
            this->targetPackageWidget->deleteLater();
            this->targetPackageWidget = nullptr;
            this->ioContainerWidget->setPackageWidget(this->targetPackageWidget);
        }

        this->destroyPanes();

        this->ioUnavailableWidget->setText("Insight deactivated");
        this->ioUnavailableWidget->show();

        this->targetStatusLabel->setText("Unknown");
        this->programCounterValueLabel->setText("-");

        this->variantMenu->clear();
        this->variantMenu->setEnabled(false);

        this->supportedVariantsByName.clear();

        this->setUiDisabled(true);
        this->activated = false;
        this->insightProjectSettings = insightProjectSettings;
    }

    void InsightWindow::adjustPanels() {
        const auto targetPackageWidgetSize = (this->targetPackageWidget != nullptr)
            ? this->targetPackageWidget->size() : QSize();
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
        static const auto absoluteMinimum = QSize(900, 400);

        /*
         * On X11, the QScreen::availableGeometry() function may return the full geometry of the screen, without
         * accounting for reserved areas for window managers and other decorations.
         *
         * Because of this, we always use QScreen::geometry() and account for reserved areas ourselves. It's near
         * impossible to do this accurately, so we just subtract 200 from the width and height, and hope that it's
         * enough.
         */
        const auto screenSize = this->screen()->availableGeometry().size();
        const auto absoluteMaximum = QSize(screenSize.width() - 200, screenSize.height() - 200);

        auto minSize = QSize();

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

    void InsightWindow::onTargetStateUpdate(TargetState newState) {
        this->targetState = newState;

        if (newState == TargetState::RUNNING) {
            this->targetStatusLabel->setText("Running");
            this->programCounterValueLabel->setText("-");
            this->setUiDisabled(true);

            if (this->targetPackageWidget != nullptr) {
                this->targetPackageWidget->setDisabled(true);
            }

        } else if (newState == TargetState::STOPPED) {
            this->targetStatusLabel->setText("Stopped");
            this->refresh();

        } else {
            this->targetStatusLabel->setText("Unknown");
        }
    }


    void InsightWindow::refresh() {
        if (this->targetState != TargetState::STOPPED || this->selectedVariant == nullptr) {
            return;
        }

        this->refreshIoInspectionButton->startSpin();
        this->refreshIoInspectionButton->setDisabled(true);

        if (this->targetPackageWidget != nullptr) {
            this->refreshPinStates();
        }

        if (this->targetRegistersSidePane != nullptr && this->targetRegistersSidePane->state.activated) {
            this->targetRegistersSidePane->refreshRegisterValues();
        }

        this->refreshProgramCounter([this] {
            this->refreshIoInspectionButton->stopSpin();

            if (this->targetState == TargetState::STOPPED) {
                this->refreshIoInspectionButton->setDisabled(false);
            }
        });
    }

    void InsightWindow::refreshPinStates() {
        this->targetPackageWidget->setDisabled(true);

        this->targetPackageWidget->refreshPinStates([this] {
            if (this->targetState == TargetState::STOPPED) {
                this->targetPackageWidget->setDisabled(false);
            }
        });
    }

    void InsightWindow::refreshProgramCounter(std::optional<std::function<void(void)>> callback) {
        const auto readProgramCounterTask = QSharedPointer<ReadProgramCounter>(
            new ReadProgramCounter(),
            &QObject::deleteLater
        );

        QObject::connect(
            readProgramCounterTask.get(),
            &ReadProgramCounter::programCounterRead,
            this,
            [this] (Targets::TargetProgramCounter programCounter) {
                this->programCounterValueLabel->setText(
                    "0x" + QString::number(programCounter, 16).toUpper() + " (" + QString::number(programCounter) + ")"
                );
            }
        );

        if (callback.has_value()) {
            QObject::connect(
                readProgramCounterTask.get(),
                &ReadProgramCounter::finished,
                this,
                callback.value()
            );
        }

        InsightWorker::queueTask(readProgramCounterTask);
    }

    void InsightWindow::openReportIssuesUrl() {
        auto url = QUrl(QString::fromStdString(Services::PathService::homeDomainName() + "/report-issue"));
        /*
         * The https://bloom.oscillate.io/report-issue URL just redirects to the Bloom GitHub issue page.
         *
         * We can use query parameters in the URL to pre-fill the body of the issue. We use this to include some
         * target information.
         */
        auto urlQuery = QUrlQuery();
        auto issueBody = QString("Issue reported via Bloom Insight.\nTarget name: "
            + QString::fromStdString(this->targetDescriptor.name) + "\n"
            + "Target ID: " + QString::fromStdString(this->targetDescriptor.id) + "\n"
        );

        if (this->selectedVariant != nullptr) {
            issueBody += "Target variant: " + QString::fromStdString(this->selectedVariant->name) + "\n";
        }

        issueBody += "\nPlease describe your issue below. Include as much detail as possible.";
        urlQuery.addQueryItem("body", issueBody);
        url.setQuery(urlQuery);

        QDesktopServices::openUrl(url);
    }

    void InsightWindow::openGettingStartedUrl() {
        QDesktopServices::openUrl(
            QUrl(QString::fromStdString(Services::PathService::homeDomainName() + "/docs/getting-started"))
        );
    }

    void InsightWindow::openAboutWindow() {
        if (this->aboutWindowWidget == nullptr) {
            this->aboutWindowWidget = new AboutWindow(this);
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

    void InsightWindow::toggleRamInspectionPane() {
        if (this->ramInspectionPane->state.activated) {
            if (!this->ramInspectionPane->state.attached) {
                this->ramInspectionPane->activateWindow();
                this->ramInspectionButton->setChecked(true);

                return;
            }

            this->ramInspectionPane->deactivate();
            return;
        }

        if (this->ramInspectionPane->state.attached) {
            if (
                this->eepromInspectionPane != nullptr
                && this->eepromInspectionPane->state.activated
                && this->eepromInspectionPane->state.attached
            ) {
                this->eepromInspectionPane->deactivate();
            }

            if (
                this->flashInspectionPane != nullptr
                && this->flashInspectionPane->state.activated
                && this->flashInspectionPane->state.attached
            ) {
                this->flashInspectionPane->deactivate();
            }
        }

        this->ramInspectionPane->activate();
    }

    void InsightWindow::toggleEepromInspectionPane() {
        if (this->eepromInspectionPane->state.activated) {
            if (!this->eepromInspectionPane->state.attached) {
                this->eepromInspectionPane->activateWindow();
                this->eepromInspectionButton->setChecked(true);

                return;
            }

            this->eepromInspectionPane->deactivate();
            return;
        }

        if (this->eepromInspectionPane->state.attached) {
            if (
                this->ramInspectionPane != nullptr
                && this->ramInspectionPane->state.activated
                && this->ramInspectionPane->state.attached
            ) {
                this->ramInspectionPane->deactivate();
            }

            if (
                this->flashInspectionPane != nullptr
                && this->flashInspectionPane->state.activated
                && this->flashInspectionPane->state.attached
            ) {
                this->flashInspectionPane->deactivate();
            }
        }

        this->eepromInspectionPane->activate();
    }

    void InsightWindow::toggleFlashInspectionPane() {
        if (this->flashInspectionPane->state.activated) {
            if (!this->flashInspectionPane->state.attached) {
                this->flashInspectionPane->activateWindow();
                this->flashInspectionButton->setChecked(true);

                return;
            }

            this->flashInspectionPane->deactivate();
            return;
        }

        if (this->flashInspectionPane->state.attached) {
            if (
                this->ramInspectionPane != nullptr
                && this->ramInspectionPane->state.activated
                && this->ramInspectionPane->state.attached
            ) {
                this->ramInspectionPane->deactivate();
            }

            if (
                this->eepromInspectionPane != nullptr
                && this->eepromInspectionPane->state.activated
                && this->eepromInspectionPane->state.attached
            ) {
                this->eepromInspectionPane->deactivate();
            }
        }

        this->flashInspectionPane->activate();
    }

    void InsightWindow::onRegistersPaneStateChanged() {
        this->targetRegistersButton->setChecked(this->targetRegistersSidePane->state.activated);

        if (this->targetState == Targets::TargetState::STOPPED && this->targetRegistersSidePane->state.activated) {
            this->targetRegistersSidePane->refreshRegisterValues();
        }
    }

    void InsightWindow::onRamInspectionPaneStateChanged() {
        this->ramInspectionButton->setChecked(this->ramInspectionPane->state.activated);

        if (this->ramInspectionPane->state.activated && this->ramInspectionPane->state.attached) {
            if (
                this->eepromInspectionPane != nullptr
                && this->eepromInspectionPane->state.activated
                && this->eepromInspectionPane->state.attached
            ) {
                this->eepromInspectionPane->deactivate();
            }

            if (
                this->flashInspectionPane != nullptr
                && this->flashInspectionPane->state.activated
                && this->flashInspectionPane->state.attached
            ) {
                this->flashInspectionPane->deactivate();
            }
        }
    }

    void InsightWindow::onEepromInspectionPaneStateChanged() {
        this->eepromInspectionButton->setChecked(this->eepromInspectionPane->state.activated);

        if (this->eepromInspectionPane->state.activated && this->eepromInspectionPane->state.attached) {
            if (
                this->ramInspectionPane != nullptr
                && this->ramInspectionPane->state.activated
                && this->ramInspectionPane->state.attached
            ) {
                this->ramInspectionPane->deactivate();
            }

            if (
                this->flashInspectionPane != nullptr
                && this->flashInspectionPane->state.activated
                && this->flashInspectionPane->state.attached
            ) {
                this->flashInspectionPane->deactivate();
            }
        }
    }

    void InsightWindow::onFlashInspectionPaneStateChanged() {
        this->flashInspectionButton->setChecked(this->flashInspectionPane->state.activated);

        if (this->flashInspectionPane->state.activated && this->flashInspectionPane->state.attached) {
            if (
                this->ramInspectionPane != nullptr
                && this->ramInspectionPane->state.activated
                && this->ramInspectionPane->state.attached
            ) {
                this->ramInspectionPane->deactivate();
            }

            if (
                this->eepromInspectionPane != nullptr
                && this->eepromInspectionPane->state.activated
                && this->eepromInspectionPane->state.attached
            ) {
                this->eepromInspectionPane->deactivate();
            }

        }
    }

    void InsightWindow::onProgrammingModeEnabled() {
        this->targetStatusLabel->setText("Programming Mode Enabled");
        this->programCounterValueLabel->setText("-");
    }

    void InsightWindow::onProgrammingModeDisabled() {
        this->onTargetStateUpdate(this->targetState);
    }
}
