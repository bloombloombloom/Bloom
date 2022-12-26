#include "MemoryRegionManagerWindow.hpp"

#include <QFile>
#include <QSize>
#include <QDesktopServices>

#include "src/Insight/UserInterfaces/InsightWindow/UiLoader.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ErrorDialogue/ErrorDialogue.hpp"

#include "src/Services/PathService.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Bloom::Widgets
{
    using Bloom::Exceptions::Exception;

    MemoryRegionManagerWindow::MemoryRegionManagerWindow(
        const Targets::TargetMemoryDescriptor& memoryDescriptor,
        std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
        std::vector<ExcludedMemoryRegion>& excludedMemoryRegions,
        QWidget* parent
    )
        : QWidget(parent)
        , memoryDescriptor(memoryDescriptor)
        , focusedMemoryRegions(focusedMemoryRegions)
        , excludedMemoryRegions(excludedMemoryRegions)
    {
        this->setWindowFlag(Qt::Window);
        this->setObjectName("memory-region-manager-window");
        this->setWindowTitle(
            "Memory Regions - "
                + QString(this->memoryDescriptor.type == Targets::TargetMemoryType::EEPROM ? "EEPROM" : "RAM")
        );

        auto windowUiFile = QFile(
            QString::fromStdString(Services::PathService::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane"
                + "/MemoryRegionManager/UiFiles/MemoryRegionManagerWindow.ui"
            )
        );

        auto windowStylesheet = QFile(
            QString::fromStdString(Services::PathService::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane"
                + "/MemoryRegionManager/Stylesheets/MemoryRegionManagerWindow.qss"
            )
        );

        if (!windowUiFile.open(QFile::ReadOnly)) {
            throw Exception("Failed to open MemoryRegionManagerWindow UI file");
        }

        if (!windowStylesheet.open(QFile::ReadOnly)) {
            throw Exception("Failed to open MemoryRegionManagerWindow stylesheet file");
        }

        this->setStyleSheet(windowStylesheet.readAll());
        this->setFixedSize(QSize(970, 540));

        auto uiLoader = UiLoader(this);
        this->container = uiLoader.load(&windowUiFile, this);

        this->container->setFixedSize(this->size());
        this->container->setContentsMargins(0, 0, 0, 0);

        this->regionSelector = this->container->findChild<QWidget*>("region-selector");
        auto* regionSelectorToolBar = this->regionSelector->findChild<QWidget*>("region-selector-tool-bar");
        this->addRegionButton = this->regionSelector->findChild<SvgToolButton*>("add-region-btn");
        this->removeRegionButton = this->regionSelector->findChild<SvgToolButton*>("remove-region-btn");
        this->addFocusedRegionMenuAction = this->addRegionButton->findChild<QAction*>("add-focused-region");
        this->addExcludedRegionMenuAction = this->addRegionButton->findChild<QAction*>("add-excluded-region");
        this->regionItemScrollArea = this->regionSelector->findChild<QScrollArea*>("region-item-scroll-area");
        this->regionItemScrollAreaViewport = this->regionItemScrollArea->findChild<QWidget*>("item-container");
        this->regionItemScrollAreaViewportLayout = this->regionItemScrollAreaViewport->findChild<QVBoxLayout*>(
            "item-container-layout"
        );

        this->stackedFormLayout = this->container->findChild<QStackedLayout*>("stacked-form-layout");

        this->applyButton = this->container->findChild<PushButton*>("apply-btn");
        this->helpButton = this->container->findChild<PushButton*>("help-btn");
        this->closeButton = this->container->findChild<PushButton*>("close-btn");

        regionSelectorToolBar->setContentsMargins(0, 0, 0, 0);
        this->regionItemScrollArea->setContentsMargins(0, 0, 0, 0);
        this->regionItemScrollAreaViewport->setContentsMargins(0, 0, 0, 0);
        this->regionItemScrollAreaViewportLayout->setContentsMargins(0, 0, 0, 0);
        this->regionItemScrollAreaViewportLayout->setDirection(QBoxLayout::Direction::TopToBottom);

        QObject::connect(
            this->addFocusedRegionMenuAction,
            &QAction::triggered,
            this,
            &MemoryRegionManagerWindow::onNewFocusedRegionTrigger
        );

        QObject::connect(
            this->addExcludedRegionMenuAction,
            &QAction::triggered,
            this,
            &MemoryRegionManagerWindow::onNewExcludedRegionTrigger
        );

        QObject::connect(
            this->removeRegionButton,
            &QToolButton::clicked,
            this,
            &MemoryRegionManagerWindow::onDeleteRegionTrigger
        );

        QObject::connect(this->closeButton, &QPushButton::clicked, this, &QWidget::close);
        QObject::connect(this->applyButton, &QPushButton::clicked, this, &MemoryRegionManagerWindow::applyChanges);
        QObject::connect(this->helpButton, &QPushButton::clicked, this, &MemoryRegionManagerWindow::openHelpPage);

        this->refreshRegions();

        // Position the inspection window at the center of the main Insight window
        this->move(parent->window()->geometry().center() - this->rect().center());
        this->show();
    }

    void MemoryRegionManagerWindow::refreshRegions() {
        this->clearRegions();

        for (const auto& focusedRegion: this->focusedMemoryRegions) {
            this->addFocusedRegion(focusedRegion);
        }

        for (const auto& excludedRegion: this->excludedMemoryRegions) {
            this->addExcludedRegion(excludedRegion);
        }

        this->sortRegionItems();
    }

    void MemoryRegionManagerWindow::showEvent(QShowEvent* event) {
        if (this->selectedRegion == nullptr && this->regionItemScrollAreaViewportLayout->count() > 0) {
            auto* firstRegionItem = qobject_cast<RegionItem*>(
                this->regionItemScrollAreaViewportLayout->itemAt(0)->widget()
            );

            if (firstRegionItem != nullptr) {
                firstRegionItem->setSelected(true);
            }
        }
    }

    void MemoryRegionManagerWindow::clearRegions() {
        this->selectedRegion = nullptr;

        for (auto* focusedRegionItem: this->focusedRegionItems) {
            this->regionItemScrollAreaViewportLayout->removeWidget(focusedRegionItem);

            focusedRegionItem->getFormWidget()->deleteLater();
            focusedRegionItem->deleteLater();
        }

        for (auto* excludedRegionItem: this->excludedRegionItems) {
            this->regionItemScrollAreaViewportLayout->removeWidget(excludedRegionItem);

            excludedRegionItem->getFormWidget()->deleteLater();
            excludedRegionItem->deleteLater();
        }

        this->focusedRegionItems.clear();
        this->excludedRegionItems.clear();
    }

    void MemoryRegionManagerWindow::sortRegionItems() {
        /*
         * This isn't very pretty.
         *
         * Because the insertion order is persisted in QBoxLayouts, sorting the items requires removing them from the
         * layout, and then re-inserting them in the correct order.
         */
        auto regionItemCompare = [] (RegionItem* itemA, RegionItem* itemB) {
            return itemA->getMemoryRegion().createdDate < itemB->getMemoryRegion().createdDate;
        };
        auto sortedRegionItems = std::set<RegionItem*, decltype(regionItemCompare)>(regionItemCompare);

        QLayoutItem* layoutItem = nullptr;
        while ((layoutItem = this->regionItemScrollAreaViewportLayout->takeAt(0)) != nullptr) {
            auto* regionItem = qobject_cast<RegionItem*>(layoutItem->widget());
            if (regionItem != nullptr) {
                sortedRegionItems.insert(regionItem);
            }

            delete layoutItem;
        }

        for (auto* regionItem: sortedRegionItems) {
            this->regionItemScrollAreaViewportLayout->addWidget(regionItem);
        }
    }

    FocusedRegionItem* MemoryRegionManagerWindow::addFocusedRegion(const FocusedMemoryRegion& region) {
        auto* focusedRegionItem = new FocusedRegionItem(
            region,
            this->memoryDescriptor,
            this->regionItemScrollAreaViewport
        );
        this->focusedRegionItems.insert(focusedRegionItem);

        this->regionItemScrollAreaViewportLayout->addWidget(focusedRegionItem);
        this->stackedFormLayout->addWidget(focusedRegionItem->getFormWidget());

        QObject::connect(
            focusedRegionItem,
            &RegionItem::selected,
            this,
            &MemoryRegionManagerWindow::onRegionSelected
        );

        return focusedRegionItem;
    }

    ExcludedRegionItem* MemoryRegionManagerWindow::addExcludedRegion(const ExcludedMemoryRegion& region) {
        auto* excludedRegionItem = new ExcludedRegionItem(
            region,
            this->memoryDescriptor,
            this->regionItemScrollAreaViewport
        );
        this->excludedRegionItems.insert(excludedRegionItem);

        this->regionItemScrollAreaViewportLayout->addWidget(excludedRegionItem);
        this->stackedFormLayout->addWidget(excludedRegionItem->getFormWidget());

        QObject::connect(
            excludedRegionItem,
            &RegionItem::selected,
            this,
            &MemoryRegionManagerWindow::onRegionSelected
        );

        return excludedRegionItem;
    }

    void MemoryRegionManagerWindow::onRegionSelected(RegionItem* selectedRegion) {
        if (this->selectedRegion != nullptr && this->selectedRegion != selectedRegion) {
            this->selectedRegion->setSelected(false);
        }

        this->selectedRegion = selectedRegion;
        this->stackedFormLayout->setCurrentWidget(this->selectedRegion->getFormWidget());
    }

    void MemoryRegionManagerWindow::onNewFocusedRegionTrigger() {
        using Targets::TargetMemoryAddressRange;

        auto* region = this->addFocusedRegion(FocusedMemoryRegion(
            "Untitled Region",
            this->memoryDescriptor.type,
            TargetMemoryAddressRange(
                this->memoryDescriptor.addressRange.startAddress,
                this->memoryDescriptor.addressRange.startAddress + 10
            )
        ));

        region->setSelected(true);
    }

    void MemoryRegionManagerWindow::onNewExcludedRegionTrigger() {
        using Targets::TargetMemoryAddressRange;

        auto* region = this->addExcludedRegion(ExcludedMemoryRegion(
            "Untitled Region",
            this->memoryDescriptor.type,
            TargetMemoryAddressRange(
                this->memoryDescriptor.addressRange.startAddress,
                this->memoryDescriptor.addressRange.startAddress + 10
            )
        ));

        region->setSelected(true);
    }

    void MemoryRegionManagerWindow::onDeleteRegionTrigger() {
        if (this->selectedRegion == nullptr) {
            return;
        }

        auto* regionItem = this->selectedRegion;
        const auto& region = regionItem->getMemoryRegion();

        if (region.type == MemoryRegionType::FOCUSED) {
            auto* focusedRegionItem = qobject_cast<FocusedRegionItem*>(regionItem);

            if (focusedRegionItem != nullptr) {
                this->focusedRegionItems.erase(focusedRegionItem);
            }

        } else {
            auto* excludedRegionItem = qobject_cast<ExcludedRegionItem*>(regionItem);

            if (excludedRegionItem != nullptr) {
                this->excludedRegionItems.erase(excludedRegionItem);
            }
        }

        regionItem->getFormWidget()->deleteLater();
        this->regionItemScrollAreaViewportLayout->removeWidget(regionItem);
        regionItem->deleteLater();

        this->selectedRegion = nullptr;

        if (this->regionItemScrollAreaViewportLayout->count() > 0) {
            auto* regionItem = qobject_cast<RegionItem*>(
                this->regionItemScrollAreaViewportLayout->itemAt(0)->widget()
            );

            if (regionItem != nullptr) {
                regionItem->setSelected(true);
            }
        }
    }

    void MemoryRegionManagerWindow::applyChanges() {
        auto processedFocusedMemoryRegions = std::vector<FocusedMemoryRegion>();
        auto processedExcludedMemoryRegions = std::vector<ExcludedMemoryRegion>();

        for (auto* focusedRegionItem : this->focusedRegionItems) {
            const auto validationFailures = focusedRegionItem->getValidationFailures();

            if (!validationFailures.empty()) {
                auto* errorDialogue = new ErrorDialogue(
                    "Invalid Memory Region",
                    "Invalid memory region \"" + focusedRegionItem->getRegionNameInputValue() + "\""
                        + "\n\n- " + validationFailures.join("\n- "),
                    this
                );
                errorDialogue->show();
                return;
            }

            focusedRegionItem->applyChanges();
            const auto& focusedRegion = focusedRegionItem->getMemoryRegion();
            for (const auto& processedFocusedRegion : processedFocusedMemoryRegions) {
                if (processedFocusedRegion.intersectsWith(focusedRegion)) {
                    auto* errorDialogue = new ErrorDialogue(
                        "Intersecting Region Found",
                        "Region \"" + focusedRegionItem->getRegionNameInputValue()
                            + "\" intersects with region \"" + processedFocusedRegion.name + "\". "
                            + "Regions cannot intersect. Please review the relevant address ranges.",
                        this
                    );
                    errorDialogue->show();
                    return;
                }
            }

            processedFocusedMemoryRegions.emplace_back(focusedRegion);
        }

        for (auto* excludedRegionItem : this->excludedRegionItems) {
            const auto validationFailures = excludedRegionItem->getValidationFailures();

            if (!validationFailures.empty()) {
                auto* errorDialogue = new ErrorDialogue(
                    "Invalid Memory Region",
                    "Invalid memory region \"" + excludedRegionItem->getRegionNameInputValue() + "\""
                        + "<br/><br/>- " + validationFailures.join("<br/>- "),
                    this
                );
                errorDialogue->show();
                return;
            }

            excludedRegionItem->applyChanges();
            auto excludedRegion = excludedRegionItem->getMemoryRegion();
            for (const auto& processedFocusedRegion : processedFocusedMemoryRegions) {
                if (processedFocusedRegion.intersectsWith(excludedRegion)) {
                    auto* errorDialogue = new ErrorDialogue(
                        "Intersecting Region Found",
                        "Region \"" + excludedRegionItem->getRegionNameInputValue()
                            + "\" intersects with region \"" + processedFocusedRegion.name + "\". "
                            + "Regions cannot intersect. Please review the relevant address ranges.",
                        this
                    );
                    errorDialogue->show();
                    return;
                }
            }

            for (const auto& processedExcludedRegion : processedExcludedMemoryRegions) {
                if (processedExcludedRegion.intersectsWith(excludedRegion)) {
                    auto* errorDialogue = new ErrorDialogue(
                        "Intersecting Region Found",
                        "Region \"" + excludedRegionItem->getRegionNameInputValue()
                            + "\" intersects with region \"" + processedExcludedRegion.name + "\". "
                            + "Regions cannot intersect. Please review the relevant address ranges.",
                        this
                    );
                    errorDialogue->show();
                    return;
                }
            }

            processedExcludedMemoryRegions.emplace_back(excludedRegion);
        }

        this->focusedMemoryRegions = std::move(processedFocusedMemoryRegions);
        this->excludedMemoryRegions = std::move(processedExcludedMemoryRegions);
        this->close();
        emit this->changesApplied();
    }

    void MemoryRegionManagerWindow::openHelpPage() {
        QDesktopServices::openUrl(
            QUrl(QString::fromStdString(Services::PathService::homeDomainName() + "/docs/manage-memory-regions"))
        );
    }
}
