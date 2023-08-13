#include "HexViewerWidget.hpp"

#include <QFile>
#include <QVBoxLayout>
#include <QLocale>

#include "src/Insight/UserInterfaces/InsightWindow/UiLoader.hpp"
#include "src/Insight/InsightSignals.hpp"

#include "src/Services/PathService.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Widgets
{
    using namespace Exceptions;

    using Targets::TargetMemoryDescriptor;

    HexViewerWidget::HexViewerWidget(
        const TargetMemoryDescriptor& targetMemoryDescriptor,
        const std::optional<Targets::TargetMemoryBuffer>& data,
        HexViewerWidgetSettings& settings,
        const std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
        const std::vector<ExcludedMemoryRegion>& excludedMemoryRegions,
        QWidget* parent
    )
        : QWidget(parent)
        , targetMemoryDescriptor(targetMemoryDescriptor)
        , data(data)
        , settings(settings)
        , focusedMemoryRegions(focusedMemoryRegions)
        , excludedMemoryRegions(excludedMemoryRegions)
    {
        this->setObjectName("hex-viewer-widget");
        this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        auto widgetUiFile = QFile(
            QString::fromStdString(Services::PathService::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget"
                + "/UiFiles/HexViewerWidget.ui"
            )
        );

        auto stylesheetFile = QFile(
            QString::fromStdString(Services::PathService::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget"
                + "/Stylesheets/HexViewerWidget.qss"
            )
        );

        if (!widgetUiFile.open(QFile::ReadOnly)) {
            throw Exception("Failed to open HexViewerWidget UI file");
        }

        if (!stylesheetFile.open(QFile::ReadOnly)) {
            throw Exception("Failed to open HexViewerWidget stylesheet file");
        }

        auto uiLoader = UiLoader(this);
        this->container = uiLoader.load(&widgetUiFile, this);
        this->setStyleSheet(stylesheetFile.readAll());
        this->container->setFixedSize(this->size());
        this->container->setContentsMargins(0, 0, 0, 0);

        this->toolBar = this->container->findChild<QWidget*>("tool-bar");
        this->bottomBar = this->container->findChild<QWidget*>("bottom-bar");

        this->groupStackMemoryButton = this->toolBar->findChild<SvgToolButton*>(
            "group-stack-memory-btn"
        );
        this->highlightHoveredRowAndColumnButton = this->toolBar->findChild<SvgToolButton*>(
            "highlight-hovered-rows-columns-btn"
        );
        this->highlightFocusedMemoryButton = this->container->findChild<SvgToolButton*>(
            "highlight-focused-memory-btn"
        );
        this->displayAnnotationsButton = this->container->findChild<SvgToolButton*>("display-annotations-btn");
        this->displayAsciiButton = this->container->findChild<SvgToolButton*>("display-ascii-btn");

        this->goToAddressInput = this->container->findChild<TextInput*>("go-to-address-input");

        this->toolBar->setContentsMargins(0, 0, 0, 0);
        this->toolBar->layout()->setContentsMargins(5, 0, 5, 1);

        this->bottomBar->setContentsMargins(0, 0, 0, 0);
        this->bottomBar->layout()->setContentsMargins(5, 0, 5, 0);

        this->hoveredAddressLabel = this->bottomBar->findChild<Label*>("byte-address-label");
        this->selectionCountLabel = this->bottomBar->findChild<Label*>("selection-count-label");

        this->loadingHexViewerLabel = this->container->findChild<Label*>("loading-hex-viewer-label");

        this->setHoveredRowAndColumnHighlightingEnabled(this->settings.highlightHoveredRowAndCol);
        this->setFocusedMemoryHighlightingEnabled(this->settings.highlightFocusedMemory);
        this->setAnnotationsEnabled(this->settings.displayAnnotations);
        this->setDisplayAsciiEnabled(this->settings.displayAsciiValues);

        if (this->targetMemoryDescriptor.type == Targets::TargetMemoryType::RAM) {
            this->groupStackMemoryButton->show();
            this->setStackMemoryGroupingEnabled(this->settings.groupStackMemory);

            QObject::connect(
                this->groupStackMemoryButton,
                &QToolButton::clicked,
                this,
                [this] {
                    this->setStackMemoryGroupingEnabled(!this->settings.groupStackMemory);
                }
            );

        } else {
            this->groupStackMemoryButton->hide();
            this->setStackMemoryGroupingEnabled(false);
        }

        QObject::connect(
            this->highlightHoveredRowAndColumnButton,
            &QToolButton::clicked,
            this,
            [this] {
                this->setHoveredRowAndColumnHighlightingEnabled(!this->settings.highlightHoveredRowAndCol);
            }
        );

        QObject::connect(
            this->highlightFocusedMemoryButton,
            &QToolButton::clicked,
            this,
            [this] {
                this->setFocusedMemoryHighlightingEnabled(!this->settings.highlightFocusedMemory);
            }
        );

        QObject::connect(
            this->displayAnnotationsButton,
            &QToolButton::clicked,
            this,
            [this] {
                this->setAnnotationsEnabled(!this->settings.displayAnnotations);
            }
        );

        QObject::connect(
            this->displayAsciiButton,
            &QToolButton::clicked,
            this,
            [this] {
                this->setDisplayAsciiEnabled(!this->settings.displayAsciiValues);
            }
        );

        QObject::connect(
            this->goToAddressInput,
            &QLineEdit::textEdited,
            this,
            &HexViewerWidget::onGoToAddressInputChanged
        );

        QObject::connect(
            this->goToAddressInput,
            &TextInput::focusChanged,
            this,
            &HexViewerWidget::onGoToAddressInputChanged
        );

        QObject::connect(
            InsightSignals::instance(),
            &InsightSignals::targetStateUpdated,
            this,
            &HexViewerWidget::onTargetStateChanged
        );

        this->show();
    }

    void HexViewerWidget::init() {
        this->byteItemGraphicsView = new ItemGraphicsView(
            this->targetMemoryDescriptor,
            this->data,
            this->focusedMemoryRegions,
            this->excludedMemoryRegions,
            this->settings,
            this->container
        );

        this->byteItemGraphicsView->hide();

        auto* containerLayout = this->container->findChild<QVBoxLayout*>("hex-viewer-layout");
        containerLayout->insertWidget(2, this->byteItemGraphicsView);

        QObject::connect(
            this->byteItemGraphicsView,
            &ItemGraphicsView::sceneReady,
            this,
            [this] {
                this->byteItemGraphicsScene = this->byteItemGraphicsView->getScene();

                QObject::connect(
                    this->byteItemGraphicsScene,
                    &ItemGraphicsScene::hoveredAddress,
                    this,
                    &HexViewerWidget::onHoveredAddress
                );

                QObject::connect(
                    this->byteItemGraphicsScene,
                    &ItemGraphicsScene::selectionChanged,
                    this,
                    &HexViewerWidget::onByteSelectionChanged
                );

                this->loadingHexViewerLabel->hide();
                this->byteItemGraphicsView->show();

                emit this->ready();
            }
        );

        this->byteItemGraphicsView->initScene();
    }

    void HexViewerWidget::updateValues() {
        if (this->byteItemGraphicsScene != nullptr) {
            this->byteItemGraphicsScene->refreshValues();
        }
    }

    void HexViewerWidget::refreshRegions() {
        if (this->byteItemGraphicsScene != nullptr) {
            this->byteItemGraphicsScene->rebuildItemHierarchy();
        }
    }

    void HexViewerWidget::setStackPointer(Targets::TargetStackPointer stackPointer) {
        if (this->byteItemGraphicsScene != nullptr) {
            this->byteItemGraphicsScene->updateStackPointer(stackPointer);
        }
    }

    void HexViewerWidget::addExternalContextMenuAction(ContextMenuAction* action) {
        assert(this->byteItemGraphicsScene != nullptr);
        this->byteItemGraphicsScene->addExternalContextMenuAction(action);
    }

    void HexViewerWidget::resizeEvent(QResizeEvent* event) {
        this->container->setFixedSize(
            this->width(),
            this->height()
        );
    }

    void HexViewerWidget::showEvent(QShowEvent* event) {
        QWidget::showEvent(event);
    }

    void HexViewerWidget::onTargetStateChanged(Targets::TargetState newState) {
        using Targets::TargetState;
        this->targetState = newState;
    }

    void HexViewerWidget::setStackMemoryGroupingEnabled(bool enabled) {
        this->groupStackMemoryButton->setChecked(enabled);
        this->settings.groupStackMemory = enabled;

        if (this->byteItemGraphicsScene != nullptr) {
            this->byteItemGraphicsScene->rebuildItemHierarchy();
        }

        emit this->settingsChanged(this->settings);
    }

    void HexViewerWidget::setHoveredRowAndColumnHighlightingEnabled(bool enabled) {
        this->highlightHoveredRowAndColumnButton->setChecked(enabled);
        this->settings.highlightHoveredRowAndCol = enabled;

        if (this->byteItemGraphicsScene != nullptr) {
            this->byteItemGraphicsScene->update();
        }

        emit this->settingsChanged(this->settings);
    }

    void HexViewerWidget::setFocusedMemoryHighlightingEnabled(bool enabled) {
        this->highlightFocusedMemoryButton->setChecked(enabled);
        this->settings.highlightFocusedMemory = enabled;

        if (this->byteItemGraphicsScene != nullptr) {
            this->byteItemGraphicsScene->update();
        }

        emit this->settingsChanged(this->settings);
    }

    void HexViewerWidget::setAnnotationsEnabled(bool enabled) {
        this->displayAnnotationsButton->setChecked(enabled);
        this->settings.displayAnnotations = enabled;

        if (this->byteItemGraphicsScene != nullptr) {
            this->byteItemGraphicsScene->adjustSize();
        }

        emit this->settingsChanged(this->settings);
    }

    void HexViewerWidget::setDisplayAsciiEnabled(bool enabled) {
        this->displayAsciiButton->setChecked(enabled);
        this->settings.displayAsciiValues = enabled;

        if (this->byteItemGraphicsScene != nullptr) {
            this->byteItemGraphicsScene->update();
        }

        emit this->settingsChanged(this->settings);
    }

    void HexViewerWidget::onGoToAddressInputChanged() {
        if (this->byteItemGraphicsScene == nullptr) {
            return;
        }

        auto addressConversionOk = false;
        const auto address = this->goToAddressInput->text().toUInt(&addressConversionOk, 16);

        const auto& memoryAddressRange = this->targetMemoryDescriptor.addressRange;

        if (addressConversionOk && memoryAddressRange.contains(address) && this->goToAddressInput->hasFocus()) {
            this->byteItemGraphicsScene->selectByteItems({address});
            this->byteItemGraphicsView->scrollToByteItemAtAddress(address);
            return;
        }

        this->byteItemGraphicsScene->selectByteItems({});
    }

    void HexViewerWidget::onHoveredAddress(const std::optional<Targets::TargetMemoryAddress>& address) {
        if (!address.has_value()) {
            this->hoveredAddressLabel->setText("Relative address / Absolute address:");
            return;
        }

        const auto addressHex = "0x" + QString::number(
            *address,
            16
        ).rightJustified(8, '0').toUpper();

        const auto relativeAddress = *address - this->targetMemoryDescriptor.addressRange.startAddress;
        const auto relativeAddressHex = "0x" + QString::number(
            relativeAddress,
            16
        ).rightJustified(8, '0').toUpper();

        this->hoveredAddressLabel->setText(
            "Relative address / Absolute address: " + relativeAddressHex + " / " + addressHex
        );
    }

    void HexViewerWidget::onByteSelectionChanged(
        const std::unordered_map<Targets::TargetMemoryAddress, ByteItem*>& selectedByteItemsByAddress
    ) {
        const auto selectionCount = selectedByteItemsByAddress.size();

        if (selectionCount == 0) {
            this->selectionCountLabel->hide();
            return;
        }

        this->selectionCountLabel->setText(
            QLocale(QLocale::English).toString(selectionCount) + " " + QString(selectionCount == 1 ? "byte" : "bytes")
                + " selected"
        );
        this->selectionCountLabel->show();
    }
}
