#include "TargetRegistersPaneWidget.hpp"

#include <QVBoxLayout>
#include <QMenu>
#include <QClipboard>
#include <QApplication>
#include <set>
#include <algorithm>

#include "src/Insight/UserInterfaces/InsightWindow/UiLoader.hpp"
#include "src/Insight/InsightSignals.hpp"
#include "src/Insight/InsightWorker/InsightWorker.hpp"

#include "src/Services/PathService.hpp"
#include "src/Exceptions/Exception.hpp"

#include "src/Insight/InsightWorker/Tasks/ReadTargetRegisters.hpp"

namespace Widgets
{
    using namespace Exceptions;

    using Targets::TargetDescriptor;
    using Targets::TargetRegisterDescriptor;
    using Targets::TargetRegisterDescriptors;
    using Targets::TargetRegisterType;

    TargetRegistersPaneWidget::TargetRegistersPaneWidget(
        const TargetDescriptor& targetDescriptor,
        const Targets::TargetState& targetState,
        PaneState& paneState,
        PanelWidget* parent
    )
        : PaneWidget(paneState, parent)
        , targetDescriptor(targetDescriptor)
        , targetState(targetState)
    {
        this->setObjectName("target-registers-side-pane");

        auto targetRegistersPaneUiFile = QFile{
            QString::fromStdString(Services::PathService::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetRegistersPane/UiFiles/"
                    "TargetRegistersSidePane.ui"
            )
        };

        if (!targetRegistersPaneUiFile.open(QFile::ReadOnly)) {
            throw Exception{"Failed to open TargetRegistersSidePane UI file"};
        }

        auto uiLoader = UiLoader{this};
        this->container = uiLoader.load(&targetRegistersPaneUiFile, this);
        this->container->setFixedSize(parent->width(), parent->maximumHeight());
        auto* containerLayout = this->container->findChild<QVBoxLayout*>();

        auto* layout = new QVBoxLayout{this};
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(this->container);

        this->toolBar = this->container->findChild<QWidget*>("tool-bar");
        this->collapseAllButton = this->toolBar->findChild<SvgToolButton*>("collapse-all-btn");
        this->expandAllButton = this->toolBar->findChild<SvgToolButton*>("expand-all-btn");
        this->toolBar->layout()->setContentsMargins(5, 0, 5, 0);
        this->searchInput = this->container->findChild<QLineEdit*>("search-input");

        this->contextMenu->addAction(this->openInspectionWindowAction);
        this->contextMenu->addAction(this->refreshValueAction);
        this->contextMenu->addSeparator();

        this->copyMenu->addAction(this->copyNameAction);
        this->copyMenu->addSeparator();
        this->copyMenu->addAction(this->copyValueDecimalAction);
        this->copyMenu->addAction(this->copyValueHexAction);
        this->copyMenu->addAction(this->copyValueBinaryAction);

        this->contextMenu->addMenu(this->copyMenu);

        QObject::connect(this->expandAllButton, &QToolButton::clicked, [this] {
            this->expandAllRegisterGroups();
        });

        QObject::connect(this->collapseAllButton, &QToolButton::clicked, [this] {
            this->collapseAllRegisterGroups();
        });

        QObject::connect(this->searchInput, &QLineEdit::textChanged, [this] {
            this->filterRegisters(this->searchInput->text());
        });

        for (const auto& [peripheralKey, peripheralDescriptor] : this->targetDescriptor.peripheralDescriptorsByKey) {
            this->peripheralItems.emplace_back(
                new PeripheralItem{
                    peripheralDescriptor,
                    this->flattenedRegisterItemsByRegisterId,
                    this->flattenedRegisterDescriptors
                }
            );
        }

        this->registerListView = new ListView{
            ListItem::ListItemSetType{this->peripheralItems.begin(), this->peripheralItems.end()},
            this
        };

        this->registerListScene = this->registerListView->listScene();
        this->registerListScene->setKeyNavigationEnabled(false);

        containerLayout->addWidget(this->registerListView);

        QObject::connect(
            this->registerListScene,
            &ListScene::itemDoubleClicked,
            this,
            &TargetRegistersPaneWidget::onItemDoubleClicked
        );

        QObject::connect(
            this->registerListScene,
            &ListScene::itemContextMenu,
            this,
            &TargetRegistersPaneWidget::onItemContextMenu
        );

        QObject::connect(
            this->openInspectionWindowAction,
            &QAction::triggered,
            this,
            [this] {
                if (this->contextMenuRegisterItem != nullptr) {
                    this->openInspectionWindow(this->contextMenuRegisterItem->registerDescriptor);
                }
            }
        );

        QObject::connect(
            this->refreshValueAction,
            &QAction::triggered,
            this,
            [this] {
                if (this->contextMenuRegisterItem != nullptr) {
                    this->refreshRegisterValues(this->contextMenuRegisterItem->registerDescriptor, std::nullopt);
                }
            }
        );

        QObject::connect(
            this->copyNameAction,
            &QAction::triggered,
            this,
            [this] {
                if (this->contextMenuRegisterItem != nullptr) {
                    this->copyRegisterName(this->contextMenuRegisterItem->registerDescriptor);
                }
            }
        );

        QObject::connect(
            this->copyValueHexAction,
            &QAction::triggered,
            this,
            [this] {
                if (this->contextMenuRegisterItem != nullptr) {
                    this->copyRegisterValueHex(this->contextMenuRegisterItem->registerDescriptor);
                }
            }
        );

        QObject::connect(
            this->copyValueDecimalAction,
            &QAction::triggered,
            this,
            [this] {
                if (this->contextMenuRegisterItem != nullptr) {
                    this->copyRegisterValueDecimal(this->contextMenuRegisterItem->registerDescriptor);
                }
            }
        );

        QObject::connect(
            this->copyValueBinaryAction,
            &QAction::triggered,
            this,
            [this] {
                if (this->contextMenuRegisterItem != nullptr) {
                    this->copyRegisterValueBinary(this->contextMenuRegisterItem->registerDescriptor);
                }
            }
        );

        auto* insightSignals = InsightSignals::instance();

        QObject::connect(
            insightSignals,
            &InsightSignals::targetStateUpdated,
            this,
            &TargetRegistersPaneWidget::onTargetStateChanged
        );

        QObject::connect(
            insightSignals,
            &InsightSignals::targetRegistersWritten,
            this,
            &TargetRegistersPaneWidget::onRegistersRead
        );

        // Restore the state
        if (this->state.activated) {
            this->activate();

        } else {
            this->deactivate();
        }

        // The register pane cannot be detached.
        this->attach();
    }

    void TargetRegistersPaneWidget::filterRegisters(const QString& keyword) {
        for (auto* peripheralItem : this->peripheralItems) {
            peripheralItem->applyFilter(keyword);
        }

        this->registerListScene->refreshGeometry();
    }

    void TargetRegistersPaneWidget::collapseAllRegisterGroups() {
        for (auto& registerGroupItem : this->peripheralItems) {
            registerGroupItem->setAllExpanded(false);
        }

        this->registerListScene->refreshGeometry();
    }

    void TargetRegistersPaneWidget::expandAllRegisterGroups() {
        for (auto& registerGroupItem : this->peripheralItems) {
            registerGroupItem->setAllExpanded(true);
        }

        this->registerListScene->refreshGeometry();
    }

    void TargetRegistersPaneWidget::refreshRegisterValues(
        std::optional<std::reference_wrapper<const Targets::TargetRegisterDescriptor>> registerDescriptor,
        std::optional<std::function<void(void)>> callback
    ) {
        if (!registerDescriptor.has_value() && this->flattenedRegisterDescriptors.empty()) {
            return;
        }

        const auto readRegisterTask = QSharedPointer<ReadTargetRegisters>{
            new ReadTargetRegisters{
                registerDescriptor.has_value()
                    ? Targets::TargetRegisterDescriptors{&(registerDescriptor->get())}
                    : this->flattenedRegisterDescriptors
            },
            &QObject::deleteLater
        };

        QObject::connect(
            readRegisterTask.get(),
            &ReadTargetRegisters::targetRegistersRead,
            this,
            &TargetRegistersPaneWidget::onRegistersRead
        );

        if (callback.has_value()) {
            QObject::connect(
                readRegisterTask.get(),
                &InsightWorkerTask::completed,
                this,
                callback.value()
            );
        }

        InsightWorker::queueTask(readRegisterTask);
    }

    void TargetRegistersPaneWidget::resizeEvent(QResizeEvent* event) {
        const auto parentSize = this->parentPanel->size();
        const auto width = parentSize.width() - 1;
        this->container->setFixedWidth(width);
        this->searchInput->setFixedWidth(width - 20);

        /*
         * In order to avoid the panel resize handle overlapping the scroll bar handle, we reduce the size of
         * the scroll area.
         */
        this->registerListView->setFixedWidth(width - this->parentPanel->getHandleSize());

        PaneWidget::resizeEvent(event);
    }

    void TargetRegistersPaneWidget::onItemDoubleClicked(ListItem* clickedItem) {
        auto* peripheralItem = dynamic_cast<PeripheralItem*>(clickedItem);

        if (peripheralItem != nullptr) {
            peripheralItem->setExpanded(!peripheralItem->isExpanded());
            this->registerListScene->refreshGeometry();
            return;
        }

        auto* registerGroupItem = dynamic_cast<RegisterGroupItem*>(clickedItem);

        if (registerGroupItem != nullptr) {
            registerGroupItem->setExpanded(!registerGroupItem->isExpanded());
            this->registerListScene->refreshGeometry();
            return;
        }

        auto* registerItem = dynamic_cast<RegisterItem*>(clickedItem);

        if (registerItem != nullptr) {
            this->openInspectionWindow(registerItem->registerDescriptor);
        }
    }

    void TargetRegistersPaneWidget::onItemContextMenu(ListItem* item, QPoint sourcePosition) {
        auto* registerItem = dynamic_cast<RegisterItem*>(item);

        if (registerItem == nullptr) {
            return;
        }

        this->contextMenuRegisterItem = registerItem;

        this->openInspectionWindowAction->setEnabled(
            TargetRegisterInspectorWindow::registerSupported(this->contextMenuRegisterItem->registerDescriptor)
        );

        const auto targetStopped = this->targetState.executionState == Targets::TargetExecutionState::STOPPED;
        const auto valuePresent = this->currentRegisterValuesByRegisterId.contains(
            this->contextMenuRegisterItem->registerDescriptor.id
        );

        this->refreshValueAction->setEnabled(targetStopped);
        this->copyValueDecimalAction->setEnabled(targetStopped && valuePresent);
        this->copyValueHexAction->setEnabled(targetStopped && valuePresent);
        this->copyValueBinaryAction->setEnabled(targetStopped && valuePresent);

        this->contextMenu->exec(sourcePosition);
    }

    void TargetRegistersPaneWidget::onTargetStateChanged(
        const Targets::TargetState& newState,
        const Targets::TargetState& previousState
    ) {
        if (previousState.executionState == newState.executionState) {
            return;
        }

        if (this->targetState.executionState != Targets::TargetExecutionState ::STOPPED) {
            this->clearInlineRegisterValues();
        }
    }

    void TargetRegistersPaneWidget::onRegistersRead(
        const Targets::TargetRegisterDescriptorAndValuePairs& registerPairs
    ) {
        for (const auto& [descriptor, value] : registerPairs) {
            const auto& previousValueIt = this->currentRegisterValuesByRegisterId.find(descriptor.id);
            const auto& registerItemIt = this->flattenedRegisterItemsByRegisterId.find(descriptor.id);

            if (registerItemIt != this->flattenedRegisterItemsByRegisterId.end()) {
                auto& registerItem = registerItemIt->second;

                registerItem->setValue(value);
                registerItem->valueChanged = previousValueIt != this->currentRegisterValuesByRegisterId.end()
                    && previousValueIt->second != value;
            }

            this->currentRegisterValuesByRegisterId[descriptor.id] = value;
        }

        this->registerListScene->update();
    }

    void TargetRegistersPaneWidget::clearInlineRegisterValues() {
        for (auto& [registerDescriptorId, registerItem] : this->flattenedRegisterItemsByRegisterId) {
            registerItem->clearValue();
        }

        this->registerListScene->update();
    }

    void TargetRegistersPaneWidget::openInspectionWindow(const TargetRegisterDescriptor& registerDescriptor) {
        if (!TargetRegisterInspectorWindow::registerSupported(registerDescriptor)) {
            return;
        }

        TargetRegisterInspectorWindow* inspectionWindow = nullptr;

        const auto& inspectionWindowIt = this->inspectionWindowsByRegisterId.find(registerDescriptor.id);
        if (inspectionWindowIt != this->inspectionWindowsByRegisterId.end()) {
            inspectionWindow = inspectionWindowIt->second;

        } else {
            inspectionWindow = new TargetRegisterInspectorWindow{
                registerDescriptor,
                this->targetState,
                this
            };

            this->inspectionWindowsByRegisterId.emplace(registerDescriptor.id, inspectionWindow);
        }

        const auto& currentValueIt = this->currentRegisterValuesByRegisterId.find(registerDescriptor.id);
        if (currentValueIt != this->currentRegisterValuesByRegisterId.end()) {
            inspectionWindow->setValue(currentValueIt->second);
        }

        inspectionWindow->show();
        inspectionWindow->activateWindow();
    }

    void TargetRegistersPaneWidget::copyRegisterName(const TargetRegisterDescriptor& registerDescriptor) {
        QApplication::clipboard()->setText(QString::fromStdString(registerDescriptor.name).toUpper());
    }

    void TargetRegistersPaneWidget::copyRegisterValueHex(const TargetRegisterDescriptor& registerDescriptor) {
        const auto& valueIt = this->currentRegisterValuesByRegisterId.find(registerDescriptor.id);
        if (valueIt == this->currentRegisterValuesByRegisterId.end()) {
            return;
        }

        const auto& value = valueIt->second;
        const auto valueByteArray = QByteArray{
            reinterpret_cast<const char *>(value.data()),
            static_cast<qsizetype>(value.size())
        }.toHex();

        QApplication::clipboard()->setText(QString{valueByteArray}.toUpper());
    }

    void TargetRegistersPaneWidget::copyRegisterValueDecimal(const TargetRegisterDescriptor& registerDescriptor) {
        const auto& valueIt = this->currentRegisterValuesByRegisterId.find(registerDescriptor.id);
        if (valueIt == this->currentRegisterValuesByRegisterId.end()) {
            return;
        }

        const auto& value = valueIt->second;
        const auto valueByteArray = QByteArray{
            reinterpret_cast<const char *>(value.data()),
            static_cast<qsizetype>(value.size())
        }.toHex();

        QApplication::clipboard()->setText(QString::number(valueByteArray.toUInt(nullptr, 16)));
    }

    void TargetRegistersPaneWidget::copyRegisterValueBinary(const TargetRegisterDescriptor& registerDescriptor) {
        const auto& valueIt = this->currentRegisterValuesByRegisterId.find(registerDescriptor.id);
        if (valueIt == this->currentRegisterValuesByRegisterId.end()) {
            return;
        }

        const auto& value = valueIt->second;
        const auto valueByteArray = QByteArray{
            reinterpret_cast<const char *>(value.data()),
            static_cast<qsizetype>(value.size())
        }.toHex();

        auto bitString = QString::number(valueByteArray.toUInt(nullptr, 16), 2);

        if (bitString.size() < (value.size() * 8)) {
            bitString = bitString.rightJustified(static_cast<qsizetype>(value.size() * 8), '0');
        }

        QApplication::clipboard()->setText(bitString);
    }
}
