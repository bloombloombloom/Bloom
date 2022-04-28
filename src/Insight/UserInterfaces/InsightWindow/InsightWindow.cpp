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
#include "src/Helpers/Paths.hpp"
#include "src/Targets/TargetDescriptor.hpp"

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
        InsightWorker& insightWorker,
        const EnvironmentConfig& environmentConfig,
        const InsightConfig& insightConfig,
        InsightProjectSettings& insightProjectSettings
    )
        : QMainWindow(nullptr)
        , insightWorker(insightWorker)
        , environmentConfig(environmentConfig)
        , targetConfig(environmentConfig.targetConfig)
        , insightConfig(insightConfig)
        , insightProjectSettings(insightProjectSettings)
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
            QString::fromStdString(Paths::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/UiFiles/InsightWindow.ui"
            )
        );
        auto mainWindowStylesheet = QFile(
            QString::fromStdString(Paths::compiledResourcesPath()
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
            QString::fromStdString(Paths::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Images/BloomIcon.svg"
            )
        ));

        this->layoutContainer = this->windowContainer->findChild<QWidget*>("layout-container");
        this->mainMenuBar = this->windowContainer->findChild<QMenuBar*>("menu-bar");
        this->layoutContainer->layout()->setMenuBar(this->mainMenuBar);
        this->container = this->layoutContainer->findChild<QWidget*>("container");
        this->ioContainerWidget = this->windowContainer->findChild<InsightTargetWidgets::TargetPackageWidgetContainer*>(
            "io-container"
        );
        this->ioUnavailableWidget = this->windowContainer->findChild<QLabel*>("io-inspection-unavailable");

        auto* fileMenu = this->mainMenuBar->findChild<QMenu*>("file-menu");
        auto* helpMenu = this->mainMenuBar->findChild<QMenu*>("help-menu");
        auto* quitAction = fileMenu->findChild<QAction*>("close-insight");
        auto* openReportIssuesUrlAction = helpMenu->findChild<QAction*>("open-report-issues-url");
        auto* openGettingStartedUrlAction = helpMenu->findChild<QAction*>("open-getting-started-url");
        auto* openAboutWindowAction = helpMenu->findChild<QAction*>("open-about-dialogue");

        this->header = this->windowContainer->findChild<QWidget*>("header");
        this->refreshIoInspectionButton = this->header->findChild<SvgToolButton*>("refresh-io-inspection-btn");

        this->leftMenuBar = this->container->findChild<QWidget*>("left-side-menu-bar");
        this->leftPanel = this->container->findChild<PanelWidget*>("left-panel");

        this->targetRegistersButton = this->container->findChild<QToolButton*>("target-registers-btn");
        auto* targetRegisterButtonLayout = this->targetRegistersButton->findChild<QVBoxLayout*>();
        auto* registersBtnLabel = new RotatableLabel(270, "Registers", this->targetRegistersButton);
        registersBtnLabel->setObjectName("target-registers-btn-label");
        registersBtnLabel->setContentsMargins(4, 3, 10, 0);
        targetRegisterButtonLayout->insertWidget(0, registersBtnLabel, 0, Qt::AlignTop);

        this->bottomMenuBar = this->container->findChild<QWidget*>("bottom-menu-bar");
        this->bottomPanel = this->container->findChild<PanelWidget*>("bottom-panel");

        this->ramInspectionButton = this->container->findChild<QToolButton*>("ram-inspection-btn");
        this->eepromInspectionButton = this->container->findChild<QToolButton*>("eeprom-inspection-btn");

        this->footer = this->windowContainer->findChild<QWidget*>("footer");
        this->targetStatusLabel = this->footer->findChild<QLabel*>("target-state");
        this->programCounterValueLabel = this->footer->findChild<QLabel*>("target-program-counter-value");
        this->targetNameLabel = this->footer->findChild<QLabel*>("target-name");
        this->targetIdLabel = this->footer->findChild<QLabel*>("target-id");
        this->variantMenu = this->footer->findChild<QMenu*>("target-variant-menu");

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

        // Tool bar button connections
        QObject::connect(
            this->refreshIoInspectionButton,
            &QToolButton::clicked,
            this,
            &InsightWindow::refresh
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

        // InsightWorker connections
        QObject::connect(
            &(this->insightWorker),
            &InsightWorker::targetControllerSuspended,
            this,
            &InsightWindow::onTargetControllerSuspended
        );
        QObject::connect(
            &(this->insightWorker),
            &InsightWorker::targetControllerResumed,
            this,
            &InsightWindow::onTargetControllerResumed
        );
        QObject::connect(
            &(this->insightWorker),
            &InsightWorker::targetStateUpdated,
            this,
            &InsightWindow::onTargetStateUpdate
        );
        QObject::connect(
            &(this->insightWorker),
            &InsightWorker::targetProgramCounterUpdated,
            this,
            &InsightWindow::onTargetProgramCounterUpdate
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
    }

    void InsightWindow::showEvent(QShowEvent* event) {
        this->adjustPanels();
        this->adjustMinimumSize();
    }

    void InsightWindow::closeEvent(QCloseEvent* event) {
        this->recordInsightSettings();

        return QMainWindow::closeEvent(event);
    }

    bool InsightWindow::isVariantSupported(const TargetVariant& variant) {
        const auto pinCount = variant.pinDescriptorsByNumber.size();

        /*
         * Because the size of the pin body widget is fixed, for all of our target package widgets, we run out of screen
         * estate for target variants with more than 100 pins.
         *
         * This will be addressed at some point, but for now, we just won't support variants with more than 100 pins.
         */
        if (pinCount > 100) {
            return false;
        }

        if (variant.package == TargetPackage::DIP
            || variant.package == TargetPackage::SOIC
            || variant.package == TargetPackage::SSOP
        ) {
            // All DIP, SOIC and SSOP variants must have a pin count that is a multiple of two
            if (pinCount % 2 == 0) {
                return true;
            }
        }

        if (variant.package == TargetPackage::QFP || variant.package == TargetPackage::QFN) {
            /*
             * All QFP and QFN variants must have a pin count that is a multiple of four. And there must be
             * more than one pin per side.
             */
            if (pinCount % 4 == 0 && pinCount > 4) {
                return true;
            }
        }

        return false;
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

        const auto& lastLeftPanelState = this->insightProjectSettings.previousLeftPanelState;
        const auto& lastBottomPanelState = this->insightProjectSettings.previousBottomPanelState;

        if (lastLeftPanelState.has_value() && this->leftPanel != nullptr) {
            this->leftPanel->setSize(lastLeftPanelState->size);

            if (lastLeftPanelState->open && this->targetRegistersSidePane != nullptr
                && this->insightProjectSettings.previousRegistersPaneState.has_value()
                && this->insightProjectSettings.previousRegistersPaneState->activated
            ) {
                this->toggleTargetRegistersPane();
            }
        }

        if (lastBottomPanelState.has_value()) {
            this->bottomPanel->setSize(lastBottomPanelState->size);

            if (this->ramInspectionPane != nullptr
                && this->insightProjectSettings.previousRamInspectionPaneState.has_value()
                && this->insightProjectSettings.previousRamInspectionPaneState->activated
            ) {
                this->toggleRamInspectionPane();

            } else if (this->eepromInspectionPane != nullptr
                && this->insightProjectSettings.previousEepromInspectionPaneState.has_value()
                && this->insightProjectSettings.previousEepromInspectionPaneState->activated
            ) {
                this->toggleEepromInspectionPane();
            }
        }

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

        std::optional<QString> previouslySelectedVariantName = (this->previouslySelectedVariant.has_value()) ?
            std::optional(QString::fromStdString(this->previouslySelectedVariant->name).toLower())
            : std::nullopt;

        if (previouslySelectedVariantName.has_value()
            && this->supportedVariantsByName.contains(previouslySelectedVariantName.value())
        ) {
            this->selectVariant(&(this->supportedVariantsByName.at(previouslySelectedVariantName.value())));

        } else if (this->targetConfig.variantName.has_value()) {
            auto selectedVariantName = QString::fromStdString(this->targetConfig.variantName.value());
            if (this->supportedVariantsByName.contains(selectedVariantName)) {
                // The user has specified a valid variant name in their config file, so use that as the default
                this->selectVariant(&(this->supportedVariantsByName.at(selectedVariantName)));

            } else {
                Logger::error("Invalid target variant name \"" + this->targetConfig.variantName.value()
                    + "\" - no such variant with the given name was found.");
            }
        }

        if (this->selectedVariant == nullptr) {
            /*
             * Given that we haven't been able to select a variant at this point, we will just fallback to the first
             * one that is available.
             */
            this->selectVariant(&(this->supportedVariantsByName.begin()->second));
        }
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

        if (variant->package == TargetPackage::DIP
            || variant->package == TargetPackage::SOIC
            || variant->package == TargetPackage::SSOP
        ) {
            this->targetPackageWidget = new InsightTargetWidgets::Dip::DualInlinePackageWidget(
                *variant,
                this->insightWorker,
                this->ioContainerWidget
            );

        } else if (variant->package == TargetPackage::QFP || variant->package == TargetPackage::QFN) {
            this->targetPackageWidget = new InsightTargetWidgets::Qfp::QuadFlatPackageWidget(
                *variant,
                this->insightWorker,
                this->ioContainerWidget
            );
        }

        if (this->targetPackageWidget != nullptr) {
            this->ioContainerWidget->setPackageWidget(this->targetPackageWidget);
            this->targetPackageWidget->setTargetState(this->targetState);

            if (this->targetState == TargetState::STOPPED) {
                this->targetPackageWidget->refreshPinStates([this] {
                    if (this->targetState == TargetState::STOPPED) {
                        this->targetPackageWidget->setDisabled(false);
                    }
                });
            }

            this->adjustPanels();
            this->adjustMinimumSize();
            this->targetPackageWidget->show();
        }
    }

    void InsightWindow::createPanes() {
        // Target registers pane
        auto* leftPanelLayout = this->leftPanel->layout();
        this->targetRegistersSidePane = new TargetRegistersPaneWidget(
            this->targetDescriptor,
            this->insightWorker,
            this->leftPanel
        );
        leftPanelLayout->addWidget(this->targetRegistersSidePane);
        this->targetRegistersButton->setChecked(false);
        this->targetRegistersButton->setDisabled(false);

        auto& memoryInspectionPaneSettingsByMemoryType =
            this->insightProjectSettings.memoryInspectionPaneSettingsByMemoryType;

        // Target memory inspection panes
        auto* bottomPanelLayout = this->bottomPanel->layout();
        if (this->targetDescriptor.memoryDescriptorsByType.contains(TargetMemoryType::RAM)) {
            auto& ramDescriptor = this->targetDescriptor.memoryDescriptorsByType.at(TargetMemoryType::RAM);

            if (!memoryInspectionPaneSettingsByMemoryType.contains(TargetMemoryType::RAM)) {
                memoryInspectionPaneSettingsByMemoryType[TargetMemoryType::RAM] = TargetMemoryInspectionPaneSettings();
            }

            this->ramInspectionPane = new TargetMemoryInspectionPane(
                ramDescriptor,
                memoryInspectionPaneSettingsByMemoryType[TargetMemoryType::RAM],
                this->insightWorker,
                this->bottomPanel
            );

            bottomPanelLayout->addWidget(this->ramInspectionPane);
            this->ramInspectionPane->deactivate();
            this->ramInspectionButton->setChecked(false);
            this->ramInspectionButton->setDisabled(false);
        }

        if (this->targetDescriptor.memoryDescriptorsByType.contains(TargetMemoryType::EEPROM)) {
            auto& eepromDescriptor = this->targetDescriptor.memoryDescriptorsByType.at(TargetMemoryType::EEPROM);

            if (!memoryInspectionPaneSettingsByMemoryType.contains(TargetMemoryType::EEPROM)) {
                memoryInspectionPaneSettingsByMemoryType[TargetMemoryType::EEPROM] = TargetMemoryInspectionPaneSettings();
            }

            this->eepromInspectionPane = new TargetMemoryInspectionPane(
                eepromDescriptor,
                memoryInspectionPaneSettingsByMemoryType[TargetMemoryType::EEPROM],
                this->insightWorker,
                this->bottomPanel
            );

            bottomPanelLayout->addWidget(this->eepromInspectionPane);
            this->eepromInspectionPane->deactivate();
            this->eepromInspectionButton->setChecked(false);
            this->eepromInspectionButton->setDisabled(false);
        }
    }

    void InsightWindow::destroyPanes() {
        if (this->targetRegistersSidePane != nullptr) {
            this->targetRegistersSidePane->deactivate();
            this->targetRegistersSidePane->deleteLater();
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
    }

    void InsightWindow::deactivate() {
        this->recordInsightSettings();

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

        this->ioUnavailableWidget->setText(
            "Insight deactivated - Bloom has been disconnected from the target.\n\n"
            "Bloom will attempt to reconnect upon the start of a new debug session."
        );
        this->ioUnavailableWidget->show();

        this->targetStatusLabel->setText("Unknown");
        this->programCounterValueLabel->setText("-");

        this->variantMenu->clear();
        this->variantMenu->setEnabled(false);

        this->supportedVariantsByName.clear();

        this->setUiDisabled(true);
        this->activated = false;
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

    void InsightWindow::onTargetControllerSuspended() {
        if (this->activated) {
            this->deactivate();
        }
    }

    void InsightWindow::onTargetControllerResumed(const TargetDescriptor& targetDescriptor) {
        if (!this->activated) {
            this->targetDescriptor = targetDescriptor;
            this->activate();
        }
    }

    void InsightWindow::onTargetStateUpdate(TargetState newState) {
        this->targetState = newState;

        if (newState == TargetState::RUNNING) {
            this->targetStatusLabel->setText("Running");
            this->programCounterValueLabel->setText("-");
            this->setUiDisabled(true);

        } else if (newState == TargetState::STOPPED) {
            this->targetStatusLabel->setText("Stopped");
            this->setUiDisabled(false);

        } else {
            this->targetStatusLabel->setText("Unknown");
        }
    }

    void InsightWindow::onTargetProgramCounterUpdate(quint32 programCounter) {
        this->programCounterValueLabel->setText(
            "0x" + QString::number(programCounter, 16).toUpper() + " (" + QString::number(programCounter) + ")"
        );
    }

    void InsightWindow::refresh() {
        if (this->targetState != TargetState::STOPPED || this->selectedVariant == nullptr) {
            return;
        }

        this->setUiDisabled(true);
        this->refreshIoInspectionButton->startSpin();

        if (this->targetPackageWidget != nullptr) {
            this->targetPackageWidget->setDisabled(true);
            this->targetPackageWidget->refreshPinStates([this] {
                if (this->targetState == TargetState::STOPPED) {
                    this->targetPackageWidget->setDisabled(false);

                    if (this->targetRegistersSidePane == nullptr || !this->targetRegistersSidePane->activated) {
                        this->refreshIoInspectionButton->stopSpin();
                        this->setUiDisabled(false);
                    }
                }
            });
        }

        if (this->targetRegistersSidePane != nullptr && this->targetRegistersSidePane->activated) {
            this->targetRegistersSidePane->refreshRegisterValues([this] {
                this->refreshIoInspectionButton->stopSpin();
                this->setUiDisabled(false);
            });
        }
    }

    void InsightWindow::openReportIssuesUrl() {
        auto url = QUrl(QString::fromStdString(Paths::homeDomainName() + "/report-issue"));
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
            QUrl(QString::fromStdString(Paths::homeDomainName() + "/docs/getting-started"))
        );
    }

    void InsightWindow::openAboutWindow() {
        if (this->aboutWindowWidget == nullptr) {
            this->aboutWindowWidget = new AboutWindow(this);
        }

        this->aboutWindowWidget->show();
    }

    void InsightWindow::toggleTargetRegistersPane() {
        if (this->targetRegistersSidePane->activated) {
            this->targetRegistersSidePane->deactivate();
            this->targetRegistersButton->setChecked(false);

            /*
             * Given that the target registers side pane is currently the only pane in the left panel, the panel
             * will be empty so no need to leave it visible.
             */
            this->leftPanel->setVisible(false);

        } else {
            this->targetRegistersSidePane->activate();
            this->targetRegistersButton->setChecked(true);
            this->leftPanel->setVisible(true);
        }

        this->adjustMinimumSize();
    }

    void InsightWindow::toggleRamInspectionPane() {
        if (this->ramInspectionPane->activated) {
            this->ramInspectionPane->deactivate();
            this->bottomPanel->hide();
            this->ramInspectionButton->setChecked(false);

        } else {
            if (this->eepromInspectionPane != nullptr && this->eepromInspectionPane->activated) {
                this->toggleEepromInspectionPane();
            }

            this->ramInspectionPane->activate();
            this->bottomPanel->show();
            this->ramInspectionButton->setChecked(true);
        }

        this->adjustMinimumSize();
    }

    void InsightWindow::toggleEepromInspectionPane() {
        if (this->eepromInspectionPane->activated) {
            this->eepromInspectionPane->deactivate();
            this->bottomPanel->hide();
            this->eepromInspectionButton->setChecked(false);

        } else {
            if (this->ramInspectionPane != nullptr && this->ramInspectionPane->activated) {
                this->toggleRamInspectionPane();
            }

            this->eepromInspectionPane->activate();
            this->bottomPanel->show();
            this->eepromInspectionButton->setChecked(true);
        }

        this->adjustMinimumSize();
    }

    void InsightWindow::recordInsightSettings() {
        auto& projectSettings = this->insightProjectSettings;

        projectSettings.mainWindowSize = this->size();

        if (this->activated) {
            if (this->leftPanel != nullptr) {
                projectSettings.previousLeftPanelState = this->leftPanel->getCurrentState();

                if (this->targetRegistersSidePane != nullptr) {
                    projectSettings.previousRegistersPaneState = this->targetRegistersSidePane->getCurrentState();
                }
            }

            if (this->bottomPanel != nullptr) {
                projectSettings.previousBottomPanelState = this->bottomPanel->getCurrentState();

                if (this->ramInspectionPane != nullptr) {
                    projectSettings.previousRamInspectionPaneState = this->ramInspectionPane->getCurrentState();
                }

                if (this->eepromInspectionPane != nullptr) {
                    projectSettings.previousEepromInspectionPaneState = this->eepromInspectionPane->getCurrentState();
                }
            }
        }
    }
}
