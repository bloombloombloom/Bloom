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

#include "AboutWindow.hpp"

using namespace Bloom;
using namespace Bloom::Exceptions;
using namespace Bloom::Widgets;

using Bloom::Targets::TargetDescriptor;
using Bloom::Targets::TargetState;
using Bloom::Targets::TargetPinState;
using Bloom::Targets::TargetVariant;
using Bloom::Targets::TargetPackage;
using Bloom::Targets::TargetPinDescriptor;
using Bloom::Targets::TargetMemoryType;

InsightWindow::InsightWindow(InsightWorker& insightWorker): QMainWindow(nullptr), insightWorker(insightWorker) {
    this->setObjectName("main-window");
    this->setWindowTitle("Bloom Insight");
    this->setMinimumSize(1000, 500);

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

    auto fileMenu = this->mainMenuBar->findChild<QMenu*>("file-menu");
    auto helpMenu = this->mainMenuBar->findChild<QMenu*>("help-menu");
    auto quitAction = fileMenu->findChild<QAction*>("close-insight");
    auto openReportIssuesUrlAction = helpMenu->findChild<QAction*>("open-report-issues-url");
    auto openGettingStartedUrlAction = helpMenu->findChild<QAction*>("open-getting-started-url");
    auto openAboutWindowAction = helpMenu->findChild<QAction*>("open-about-dialogue");

    connect(quitAction, &QAction::triggered, this, &InsightWindow::close);
    connect(openReportIssuesUrlAction, &QAction::triggered, this, &InsightWindow::openReportIssuesUrl);
    connect(openGettingStartedUrlAction, &QAction::triggered, this, &InsightWindow::openGettingStartedUrl);
    connect(openAboutWindowAction, &QAction::triggered, this, &InsightWindow::openAboutWindow);

    this->header = this->windowContainer->findChild<QWidget*>("header");
    this->refreshIoInspectionButton = this->header->findChild<QToolButton*>("refresh-io-inspection-btn");

    connect(this->refreshIoInspectionButton, &QToolButton::clicked, this, [this] {
        // TODO: Move this into a member function - getting too big for a lambda
        if (this->targetState == TargetState::STOPPED && this->selectedVariant != nullptr) {
            this->toggleUi(true);
            if (this->targetPackageWidget != nullptr) {
                this->targetPackageWidget->setDisabled(true);
                this->targetPackageWidget->refreshPinStates([this] {
                    if (this->targetState == TargetState::STOPPED) {
                        this->targetPackageWidget->setDisabled(false);

                        if (this->targetRegistersSidePane == nullptr || !this->targetRegistersSidePane->activated) {
                            this->toggleUi(false);
                        }
                    }
                });
            }

            if (this->targetRegistersSidePane != nullptr && this->targetRegistersSidePane->activated) {
                this->targetRegistersSidePane->refreshRegisterValues([this] {
                    this->toggleUi(false);
                });
            }
        }
    });

    this->leftMenuBar = this->container->findChild<QWidget*>("left-side-menu-bar");
    this->leftPanel = this->container->findChild<PanelWidget*>("left-panel");

    this->targetRegistersButton = this->container->findChild<QToolButton*>("target-registers-btn");
    auto targetRegisterButtonLayout = this->targetRegistersButton->findChild<QVBoxLayout*>();
    auto* registersBtnLabel = new RotatableLabel(270, "Registers", this->targetRegistersButton);
    registersBtnLabel->setObjectName("target-registers-btn-label");
    registersBtnLabel->setContentsMargins(5,0,9,0);
    targetRegisterButtonLayout->insertWidget(0, registersBtnLabel, 0, Qt::AlignTop);

    connect(this->targetRegistersButton, &QToolButton::clicked, this, &InsightWindow::toggleTargetRegistersPane);

    this->bottomMenuBar = this->container->findChild<QWidget*>("bottom-menu-bar");
    this->bottomPanel = this->container->findChild<PanelWidget*>("bottom-panel");

    this->ramInspectionButton = this->container->findChild<QToolButton*>("ram-inspection-btn");
    this->eepromInspectionButton = this->container->findChild<QToolButton*>("eeprom-inspection-btn");

    connect(this->ramInspectionButton, &QToolButton::clicked, this, &InsightWindow::toggleRamInspectionPane);
    connect(this->eepromInspectionButton, &QToolButton::clicked, this, &InsightWindow::toggleEepromInspectionPane);

    this->footer = this->windowContainer->findChild<QWidget*>("footer");
    this->targetStatusLabel = this->footer->findChild<QLabel*>("target-state");
    this->programCounterValueLabel = this->footer->findChild<QLabel*>("target-program-counter-value");

    const auto windowSize = this->size();
    this->windowContainer->setFixedSize(windowSize);
    this->layoutContainer->setFixedSize(windowSize);
}

void InsightWindow::init(TargetDescriptor targetDescriptor) {
    this->targetDescriptor = std::move(targetDescriptor);
    this->activate();
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

    } else if (newState == TargetState::STOPPED) {
        this->targetStatusLabel->setText("Stopped");
        this->toggleUi(false);

    } else {
        this->targetStatusLabel->setText("Unknown");
    }
}

void InsightWindow::onTargetProgramCounterUpdate(quint32 programCounter) {
    this->programCounterValueLabel->setText(
        "0x" + QString::number(programCounter, 16).toUpper() + " (" + QString::number(programCounter) + ")"
    );
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
    QDesktopServices::openUrl(QUrl(QString::fromStdString(Paths::homeDomainName() + "/docs/getting-started")));
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
         * Given that the target registers side pane is currently the only pane in the left panel, the panel will be
         * empty so no need to leave it visible.
         */
        this->leftPanel->setVisible(false);

    } else {
        this->targetRegistersSidePane->activate();
        this->targetRegistersButton->setChecked(true);
        this->leftPanel->setVisible(true);
    }
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
}

void InsightWindow::resizeEvent(QResizeEvent* event) {
    const auto windowSize = this->size();

    this->windowContainer->setFixedSize(windowSize);
    this->layoutContainer->setFixedSize(windowSize);

    this->adjustPanels();
}

void InsightWindow::showEvent(QShowEvent* event) {
    this->adjustPanels();
}

bool InsightWindow::isVariantSupported(const TargetVariant& variant) {
    /*
     * Because the size of the pin body widget is fixed, for all of our target package widgets, we run out of screen
     * estate for target variants with more than 100 pins.
     *
     * This will be addressed at some point, but for now, we just won't support variants with more than 100 pins.
     */
    if (variant.pinDescriptorsByNumber.size() > 100) {
        return false;
    }

    if (variant.package == TargetPackage::DIP
        || variant.package == TargetPackage::SOIC
        || variant.package == TargetPackage::SSOP
    ) {
        // All DIP, SOIC and SSOP variants must have a pin count that is a multiple of two
        if (variant.pinDescriptorsByNumber.size() % 2 == 0) {
            return true;
        }
    }

    if (variant.package == TargetPackage::QFP || variant.package == TargetPackage::QFN) {
        // All QFP and QFN variants must have a pin count that is a multiple of four
        if (variant.pinDescriptorsByNumber.size() % 4 == 0) {
            return true;
        }
    }

    return false;
}

void InsightWindow::selectVariant(const TargetVariant* variant) {
    if (!this->isVariantSupported(*variant)) {
        Logger::error("Attempted to select unsupported target variant.");
        return;
    }

    if (this->selectedVariant != nullptr && this->selectedVariant->id == variant->id) {
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

        this->setMinimumSize(
            this->targetPackageWidget->width() + 700,
            this->targetPackageWidget->height() + 450
        );

        this->adjustPanels();
        this->targetPackageWidget->show();
    }
}

void InsightWindow::toggleUi(bool disable) {
    this->uiDisabled = disable;

    if (this->refreshIoInspectionButton != nullptr) {
        this->refreshIoInspectionButton->setDisabled(disable);
        this->refreshIoInspectionButton->repaint();
    }
}

void InsightWindow::activate() {
    auto targetNameLabel = this->footer->findChild<QLabel*>("target-name");
    auto targetIdLabel = this->footer->findChild<QLabel*>("target-id");
    targetNameLabel->setText(QString::fromStdString(this->targetDescriptor.name));
    targetIdLabel->setText("0x" + QString::fromStdString(this->targetDescriptor.id).remove("0x").toUpper());
    this->variantMenu = this->footer->findChild<QMenu*>("target-variant-menu");

    this->ioUnavailableWidget->hide();

    std::optional<QString> previouslySelectedVariantName;
    if (this->selectedVariant != nullptr) {
        previouslySelectedVariantName = QString::fromStdString(this->selectedVariant->name).toLower();
        this->selectedVariant = nullptr;
    }

    this->supportedVariantsByName.clear();

    /*
     * We don't want to present the user with duplicate target variants.
     *
     * In the context of Insight, a variant that doesn't differ in package type or pinout configuration is
     * considered a duplicate.
     */
    auto processedVariants = std::vector<TargetVariant>();
    auto isDuplicateVariant = [&processedVariants] (const TargetVariant& variantA) {
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
            auto supportedVariantPtr = &(this->supportedVariantsByName.insert(
                std::pair(QString::fromStdString(targetVariant.name).toLower(), targetVariant)
            ).first->second);

            connect(
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

    this->variantMenu->setEnabled(true);

    Logger::debug("Number of target variants supported by Insight: "
        + std::to_string(supportedVariantsByName.size()));

    if (!this->supportedVariantsByName.empty()) {
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

    } else {
        if (this->targetDescriptor.variants.empty()) {
            this->variantMenu->parentWidget()->hide();
        }

        this->ioUnavailableWidget->setText(
            "GPIO inspection is not available for this target. "
            "Please report this to Bloom developers by clicking Help -> Report An Issue"
        );
        this->ioUnavailableWidget->show();
    }

    auto leftPanelLayout = this->leftPanel->layout();
    this->targetRegistersSidePane = new TargetRegistersPaneWidget(
        this->targetDescriptor,
        this->insightWorker,
        this->leftPanel
    );
    leftPanelLayout->addWidget(this->targetRegistersSidePane);
    this->targetRegistersButton->setChecked(false);
    this->targetRegistersButton->setDisabled(false);

    auto bottomPanelLayout = this->bottomPanel->layout();
    if (this->targetDescriptor.memoryDescriptorsByType.contains(TargetMemoryType::RAM)) {
        auto& ramDescriptor = this->targetDescriptor.memoryDescriptorsByType.at(TargetMemoryType::RAM);
        this->ramInspectionPane = new TargetMemoryInspectionPane(
            ramDescriptor,
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
        this->eepromInspectionPane = new TargetMemoryInspectionPane(
            eepromDescriptor,
            this->insightWorker,
            this->bottomPanel
        );
        bottomPanelLayout->addWidget(this->eepromInspectionPane);
        this->eepromInspectionPane->deactivate();
        this->eepromInspectionButton->setChecked(false);
        this->eepromInspectionButton->setDisabled(false);
    }

    this->toggleUi(this->targetState != TargetState::STOPPED);
    this->activated = true;
}

void InsightWindow::deactivate() {
    if (this->targetPackageWidget != nullptr) {
        this->targetPackageWidget->hide();
        this->targetPackageWidget->deleteLater();
        this->targetPackageWidget = nullptr;
    }

    if (this->targetRegistersSidePane != nullptr) {
        this->targetRegistersSidePane->deactivate();
        this->targetRegistersSidePane->deleteLater();
        this->leftPanel->setVisible(false);
        this->targetRegistersButton->setChecked(false);
        this->targetRegistersButton->setDisabled(true);
    }

    this->ioUnavailableWidget->setText(
        "Insight deactivated - Bloom has been disconnected from the target.\n\n"
        "Bloom will attempt to reconnect upon the start of a new debug session."
    );
    this->ioUnavailableWidget->show();

    this->targetStatusLabel->setText("Unknown");
    this->programCounterValueLabel->setText("-");

    this->variantMenu->clear();
    this->variantMenu->setEnabled(false);

    this->toggleUi(true);
    this->activated = false;
}

void InsightWindow::adjustPanels() {
    const auto targetPackageWidgetSize = (this->targetPackageWidget != nullptr)
        ? this->targetPackageWidget->size() : QSize();
    const auto containerSize = this->container->size();

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
    this->bottomPanel->setMaximumResize(
        std::max(
            this->bottomPanel->getMinimumResize(),
            containerSize.height() - targetPackageWidgetSize.height() - this->bottomMenuBar->height() - 20
        )
    );
}
