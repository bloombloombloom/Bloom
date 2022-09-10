#include "HexViewerWidget.hpp"

#include <QFile>

#include "src/Insight/UserInterfaces/InsightWindow/UiLoader.hpp"
#include "src/Insight/InsightSignals.hpp"

#include "src/Helpers/Paths.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Bloom::Widgets
{
    using namespace Bloom::Exceptions;

    using Bloom::Targets::TargetMemoryDescriptor;

    HexViewerWidget::HexViewerWidget(
        const TargetMemoryDescriptor& targetMemoryDescriptor,
        HexViewerWidgetSettings& settings,
        std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
        std::vector<ExcludedMemoryRegion>& excludedMemoryRegions,
        QWidget* parent
    )
        : QWidget(parent)
        , targetMemoryDescriptor(targetMemoryDescriptor)
        , settings(settings)
        , focusedMemoryRegions(focusedMemoryRegions)
        , excludedMemoryRegions(excludedMemoryRegions)
    {
        this->setObjectName("hex-viewer-widget");
        this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        auto widgetUiFile = QFile(
            QString::fromStdString(Paths::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget"
                + "/UiFiles/HexViewerWidget.ui"
            )
        );

        if (!widgetUiFile.open(QFile::ReadOnly)) {
            throw Exception("Failed to open HexViewerWidget UI file");
        }

        auto uiLoader = UiLoader(this);
        this->container = uiLoader.load(&widgetUiFile, this);
        this->container->setFixedSize(this->size());
        this->container->setContentsMargins(0, 0, 0, 0);

        this->toolBar = this->container->findChild<QWidget*>("tool-bar");
        this->bottomBar = this->container->findChild<QWidget*>("bottom-bar");

        this->highlightStackMemoryButton = this->toolBar->findChild<SvgToolButton*>(
            "highlight-stack-memory-btn"
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

        this->loadingHexViewerLabel = this->container->findChild<Label*>("loading-hex-viewer-label");
        this->byteItemGraphicsViewContainer = this->container->findChild<QWidget*>("graphics-view-container");

        this->setHoveredRowAndColumnHighlightingEnabled(this->settings.highlightHoveredRowAndCol);
        this->setFocusedMemoryHighlightingEnabled(this->settings.highlightFocusedMemory);
        this->setAnnotationsEnabled(this->settings.displayAnnotations);
        this->setDisplayAsciiEnabled(this->settings.displayAsciiValues);

        if (this->targetMemoryDescriptor.type == Targets::TargetMemoryType::RAM) {
            this->highlightStackMemoryButton->show();
            this->setStackMemoryHighlightingEnabled(this->settings.highlightStackMemory);

            QObject::connect(
                this->highlightStackMemoryButton,
                &QToolButton::clicked,
                this,
                [this] {
                    this->setStackMemoryHighlightingEnabled(!this->settings.highlightStackMemory);
                }
            );

        } else {
            this->highlightStackMemoryButton->hide();
            this->setStackMemoryHighlightingEnabled(false);
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
        this->byteItemGraphicsView = new ByteItemContainerGraphicsView(this->byteItemGraphicsViewContainer);

        QObject::connect(
            this->byteItemGraphicsView,
            &ByteItemContainerGraphicsView::ready,
            this,
            [this] {
                this->byteItemGraphicsScene = this->byteItemGraphicsView->getScene();
                this->loadingHexViewerLabel->hide();
                this->byteItemGraphicsViewContainer->show();
                this->byteItemGraphicsView->setFixedSize(this->byteItemGraphicsViewContainer->size());

                emit this->ready();
            }
        );

        this->byteItemGraphicsView->initScene(
            this->targetMemoryDescriptor,
            this->focusedMemoryRegions,
            this->excludedMemoryRegions,
            this->settings,
            this->hoveredAddressLabel
        );
    }

    void HexViewerWidget::updateValues(const Targets::TargetMemoryBuffer& buffer) {
        if (this->byteItemGraphicsScene != nullptr) {
            this->byteItemGraphicsScene->updateValues(buffer);
        }
    }

    void HexViewerWidget::refreshRegions() {
        if (this->byteItemGraphicsScene != nullptr) {
            this->byteItemGraphicsScene->refreshRegions();
        }
    }

    void HexViewerWidget::setStackPointer(Targets::TargetStackPointer stackPointer) {
        if (this->byteItemGraphicsScene != nullptr) {
            this->byteItemGraphicsScene->updateStackPointer(stackPointer);
        }
    }

    void HexViewerWidget::resizeEvent(QResizeEvent* event) {
        this->container->setFixedSize(
            this->width(),
            this->height()
        );

        this->byteItemGraphicsView->setFixedSize(this->byteItemGraphicsViewContainer->size());
    }

    void HexViewerWidget::showEvent(QShowEvent* event) {
        this->byteItemGraphicsView->setFixedSize(this->byteItemGraphicsViewContainer->size());
    }

    void HexViewerWidget::onTargetStateChanged(Targets::TargetState newState) {
        using Targets::TargetState;
        this->targetState = newState;
    }

    void HexViewerWidget::setStackMemoryHighlightingEnabled(bool enabled) {
        this->highlightStackMemoryButton->setChecked(enabled);
        this->settings.highlightStackMemory = enabled;

        if (this->byteItemGraphicsScene != nullptr) {
            this->byteItemGraphicsScene->invalidateChildItemCaches();
        }
    }

    void HexViewerWidget::setHoveredRowAndColumnHighlightingEnabled(bool enabled) {
        this->highlightHoveredRowAndColumnButton->setChecked(enabled);
        this->settings.highlightHoveredRowAndCol = enabled;

        if (this->byteItemGraphicsScene != nullptr) {
            this->byteItemGraphicsScene->invalidateChildItemCaches();
        }
    }

    void HexViewerWidget::setFocusedMemoryHighlightingEnabled(bool enabled) {
        this->highlightFocusedMemoryButton->setChecked(enabled);
        this->settings.highlightFocusedMemory = enabled;

        if (this->byteItemGraphicsScene != nullptr) {
            this->byteItemGraphicsScene->invalidateChildItemCaches();
        }
    }

    void HexViewerWidget::setAnnotationsEnabled(bool enabled) {
        this->displayAnnotationsButton->setChecked(enabled);
        this->settings.displayAnnotations = enabled;

        if (this->byteItemGraphicsScene != nullptr) {
            this->byteItemGraphicsScene->adjustSize(true);
        }
    }

    void HexViewerWidget::setDisplayAsciiEnabled(bool enabled) {
        this->displayAsciiButton->setChecked(enabled);
        this->settings.displayAsciiValues = enabled;

        if (this->byteItemGraphicsScene != nullptr) {
            this->byteItemGraphicsScene->invalidateChildItemCaches();
        }
    }

    void HexViewerWidget::onGoToAddressInputChanged() {
        if (this->byteItemGraphicsScene == nullptr) {
            return;
        }

        auto addressConversionOk = false;
        const auto address = this->goToAddressInput->text().toUInt(&addressConversionOk, 16);

        const auto& memoryAddressRange = this->targetMemoryDescriptor.addressRange;

        if (addressConversionOk && memoryAddressRange.contains(address) && this->goToAddressInput->hasFocus()) {
            this->byteItemGraphicsScene->setHighlightedAddresses({address});
            this->byteItemGraphicsView->scrollToByteItemAtAddress(address);
            return;
        }

        this->byteItemGraphicsScene->setHighlightedAddresses({});
    }
}
