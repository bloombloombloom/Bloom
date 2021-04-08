#include <QtUiTools>
#include <QtSvg/QtSvg>

#include "InsightWindow.hpp"
#include "AboutWindow.hpp"
#include "TargetWidgets/DIP/DualInlinePackageWidget.hpp"
#include "TargetWidgets/QFP/QuadFlatPackageWidget.hpp"
#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"
#include "src/Targets/TargetDescriptor.hpp"

using namespace Bloom;
using namespace Exceptions;
using Targets::TargetDescriptor;
using Targets::TargetVariant;
using Targets::TargetPackage;


void InsightWindow::init(
    QApplication& application,
    TargetDescriptor targetDescriptor,
    const InsightConfig& config,
    const TargetConfig& targetConfig
) {
    this->targetDescriptor = targetDescriptor;

    auto mainWindowUiFile = QFile(":/compiled/Insight/UserInterfaces/InsightWindow/UiFiles/InsightWindow.ui");
    auto mainWindowStylesheet = QFile(":/compiled/Insight/UserInterfaces/InsightWindow/Stylesheets/InsightWindow.qss");

    if (!mainWindowUiFile.open(QFile::ReadOnly)) {
        throw Exception("Failed to open InsightWindow UI file");
    }

    if (!mainWindowStylesheet.open(QFile::ReadOnly)) {
        throw Exception("Failed to open InsightWindow stylesheet file");
    }

    auto uiLoader = QUiLoader(this);
    this->mainWindowWidget = uiLoader.load(&mainWindowUiFile);
    this->mainWindowWidget->setStyleSheet(mainWindowStylesheet.readAll());

    application.setWindowIcon(QIcon(":/compiled/Insight/UserInterfaces/InsightWindow/Images/BloomIcon.svg"));
    this->ioContainerWidget = this->mainWindowWidget->findChild<QWidget*>("io-container");
    this->ioUnavailableWidget = this->mainWindowWidget->findChild<QWidget*>("io-inspection-unavailable");
    this->mainMenuBar = this->mainWindowWidget->findChild<QMenuBar*>("menu-bar");

    auto fileMenu = this->mainMenuBar->findChild<QMenu*>("file-menu");
    auto helpMenu = this->mainMenuBar->findChild<QMenu*>("help-menu");
    auto quitAction = fileMenu->findChild<QAction*>("close-insight");
    auto openReportIssuesUrlAction = helpMenu->findChild<QAction*>("open-report-issues-url");
    auto openAboutWindowAction = helpMenu->findChild<QAction*>("open-about-dialogue");

    connect(quitAction, &QAction::triggered, this, &InsightWindow::close);
    connect(openReportIssuesUrlAction, &QAction::triggered, this, &InsightWindow::openReportIssuesUrl);
    connect(openAboutWindowAction, &QAction::triggered, this, &InsightWindow::openAboutWindow);

    this->header = this->mainWindowWidget->findChild<QWidget*>("header");
    this->refreshIoInspectionButton = this->header->findChild<QToolButton*>("refresh-io-inspection-btn");

    connect(this->refreshIoInspectionButton, &QToolButton::clicked, this, [this] {
        if (this->targetState == TargetState::STOPPED && this->selectedVariant != nullptr) {
            this->toggleUi(true);
            emit this->refreshTargetPinStates(this->selectedVariant->id);
        }
    });

    this->footer = this->mainWindowWidget->findChild<QWidget*>("footer");
    auto targetNameLabel = this->footer->findChild<QLabel*>("target-name");
    auto targetIdLabel = this->footer->findChild<QLabel*>("target-id");
    this->targetStatusLabel = this->footer->findChild<QLabel*>("target-state");
    this->programCounterValueLabel = this->footer->findChild<QLabel*>("target-program-counter-value");
    targetNameLabel->setText(QString::fromStdString(this->targetDescriptor.name));
    targetIdLabel->setText("0x" + QString::fromStdString(this->targetDescriptor.id).remove("0x").toUpper());

    this->variantMenu = this->footer->findChild<QMenu*>("target-variant-menu");

    for (const auto& targetVariant: this->targetDescriptor.variants) {
        auto variantAction = new QAction(this->variantMenu);
        variantAction->setText(
            QString::fromStdString(targetVariant.name + " (" + targetVariant.packageName + ")")
        );

        if (this->isVariantSupported(targetVariant)) {
            connect(
                variantAction,
                &QAction::triggered,
                this,
                [this, &targetVariant] {
                    this->selectVariant(&targetVariant);
                }
            );

        } else {
            variantAction->setEnabled(false);
            variantAction->setText(variantAction->text() + " (unsupported)");

        };

        this->variantMenu->addAction(variantAction);
    }

    std::copy_if(
        this->targetDescriptor.variants.begin(),
        this->targetDescriptor.variants.end(),
        std::back_inserter(this->supportedVariants),
        [this](auto variant) {
            return this->isVariantSupported(variant);
        }
    );

    Logger::debug("Number of target variants supported by Insight: " + std::to_string(supportedVariants.size()));

    if (!supportedVariants.empty()) {
        auto selectedVariant = std::find_if(
            supportedVariants.begin(),
            supportedVariants.end(),
            [&targetConfig](const TargetVariant& variant) {
                auto variantName = QString::fromStdString(variant.name).toLower().toStdString();
                return !targetConfig.variantName.empty() && targetConfig.variantName == variantName;
            }
        );

        /*
         * If ths user specified a valid variant name in their config file, use that as the default, otherwise just
         * use the first supported variant.
         */
        this->selectVariant((selectedVariant != supportedVariants.end()) ? &(*selectedVariant)
            : &supportedVariants.front());

    } else {
        if (this->targetDescriptor.variants.empty()) {
            this->variantMenu->parentWidget()->hide();
        }

        this->ioUnavailableWidget->show();
    }
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
    }

    this->selectedVariant = variant;
    this->variantMenu->setTitle(QString::fromStdString(variant->name + " (" + variant->packageName + ")"));

    if (variant->package == TargetPackage::DIP) {
        this->targetPackageWidget = new InsightTargetWidgets::Dip::DualInlinePackageWidget(
            *variant,
            this,
            this->ioContainerWidget
        );

    } else if (variant->package == TargetPackage::QFP) {
        this->targetPackageWidget = new InsightTargetWidgets::Qfp::QuadFlatPackageWidget(
            *variant,
            this,
            this->ioContainerWidget
        );
    }

    if (this->targetPackageWidget != nullptr) {
        if (this->targetState == TargetState::STOPPED) {
            this->toggleUi(true);
            emit this->refreshTargetPinStates(variant->id);
        }

        this->targetPackageWidget->show();
    }
}

void InsightWindow::show() {
    this->mainWindowWidget->activateWindow();
    this->mainWindowWidget->show();
}

void InsightWindow::close() {
    if (this->mainWindowWidget != nullptr) {
        this->mainWindowWidget->close();
    }
}

bool InsightWindow::isVariantSupported(const TargetVariant& variant) {
    if (variant.package == TargetPackage::DIP) {
        // All DIP variants must have a pin count that is a multiple of two
        if (variant.pinDescriptorsByNumber.size() % 2 == 0) {
            return true;
        }
    }

    if (variant.package == TargetPackage::QFP) {
        // All QFP variants must have a pin count that is a multiple of four
        if (variant.pinDescriptorsByNumber.size() % 4 == 0) {
            return true;
        }
    }

    return false;
}

void InsightWindow::openReportIssuesUrl() {
    QDesktopServices::openUrl(QUrl("https://bloom.oscillate.io/report-issue"));
}

void InsightWindow::openAboutWindow() {
    if (this->aboutWindowWidget == nullptr) {
        this->aboutWindowWidget = new AboutWindow(this->mainWindowWidget);
    }

    this->aboutWindowWidget->show();
}

void InsightWindow::onTargetStateUpdate(TargetState newState) {
    this->targetState = newState;

    if (newState == TargetState::RUNNING) {
        this->targetStatusLabel->setText("Running");
        this->programCounterValueLabel->setText("-");
        this->toggleUi(true);

    } else if (newState == TargetState::STOPPED) {
        this->targetStatusLabel->setText("Stopped");

        if (this->selectedVariant != nullptr) {
            emit this->refreshTargetPinStates(this->selectedVariant->id);
        }

    } else {
        this->targetStatusLabel->setText("Unknown");
    }
}

void InsightWindow::onTargetProgramCounterUpdate(quint32 programCounter) {
    this->programCounterValueLabel->setText(
        "0x" + QString::number(programCounter, 16).toUpper() + " (" + QString::number(programCounter) + ")"
    );
}

void InsightWindow::onTargetIoPortsUpdate() {
    if (this->targetState == TargetState::STOPPED && this->selectedVariant != nullptr) {
        emit this->refreshTargetPinStates(this->selectedVariant->id);
    }
}

void InsightWindow::onTargetPinStatesUpdate(int variantId, Bloom::Targets::TargetPinStateMappingType pinStatesByNumber) {
    if (this->targetPackageWidget != nullptr
        && this->selectedVariant != nullptr
        && this->selectedVariant->id == variantId
    ) {
        this->targetPackageWidget->updatePinStates(pinStatesByNumber);
        if (this->uiDisabled) {
            this->toggleUi(false);

        } else {
            this->targetPackageWidget->repaint();
        }
    }
}

void InsightWindow::togglePinIoState(TargetPinWidget* pinWidget) {
    auto pinState = pinWidget->getPinState();

    // Currently, we only allow users to toggle the IO state of output pins
    if (pinState.has_value()
        && pinState.value().ioDirection == TargetPinState::IoDirection::OUTPUT
        && this->selectedVariant != nullptr
    ) {
        auto& pinDescriptor = pinWidget->getPinDescriptor();
        pinState.value().ioState = (pinState.value().ioState == TargetPinState::IoState::HIGH) ?
            TargetPinState::IoState::LOW : TargetPinState::IoState::HIGH;
        emit this->setTargetPinState(this->selectedVariant->id, pinDescriptor, pinState.value());
    }
}