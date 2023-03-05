#include "TargetRegistersPaneWidget.hpp"

#include <QVBoxLayout>
#include <QMenu>
#include <QClipboard>
#include <QApplication>
#include <set>

#include "src/Insight/UserInterfaces/InsightWindow/UiLoader.hpp"
#include "src/Insight/InsightSignals.hpp"
#include "src/Insight/InsightWorker/InsightWorker.hpp"

#include "src/Services/PathService.hpp"
#include "src/Exceptions/Exception.hpp"

#include "src/Insight/InsightWorker/Tasks/ReadTargetRegisters.hpp"

namespace Bloom::Widgets
{
    using namespace Bloom::Exceptions;

    using Bloom::Targets::TargetDescriptor;
    using Bloom::Targets::TargetRegisterDescriptor;
    using Bloom::Targets::TargetRegisterDescriptors;
    using Bloom::Targets::TargetRegisterType;

    TargetRegistersPaneWidget::TargetRegistersPaneWidget(
        const TargetDescriptor& targetDescriptor,
        PaneState& paneState,
        PanelWidget* parent
    )
        : PaneWidget(paneState, parent)
        , targetDescriptor(targetDescriptor)
    {
        this->setObjectName("target-registers-side-pane");

        auto targetRegistersPaneUiFile = QFile(
            QString::fromStdString(Services::PathService::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetRegistersPane/UiFiles/"
                  "TargetRegistersSidePane.ui"
            )
        );

        if (!targetRegistersPaneUiFile.open(QFile::ReadOnly)) {
            throw Exception("Failed to open TargetRegistersSidePane UI file");
        }

        auto uiLoader = UiLoader(this);
        this->container = uiLoader.load(&targetRegistersPaneUiFile, this);
        this->container->setFixedSize(parent->width(), parent->maximumHeight());
        auto* containerLayout = this->container->findChild<QVBoxLayout*>();

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(this->container);

        this->toolBar = this->container->findChild<QWidget*>("tool-bar");
        this->collapseAllButton = this->toolBar->findChild<SvgToolButton*>("collapse-all-btn");
        this->expandAllButton = this->toolBar->findChild<SvgToolButton*>("expand-all-btn");
        this->toolBar->layout()->setContentsMargins(5, 0, 5, 0);
        this->searchInput = this->container->findChild<QLineEdit*>("search-input");

        QObject::connect(this->expandAllButton, &QToolButton::clicked, [this] {
            this->expandAllRegisterGroups();
        });

        QObject::connect(this->collapseAllButton, &QToolButton::clicked, [this] {
            this->collapseAllRegisterGroups();
        });

        QObject::connect(this->searchInput, &QLineEdit::textChanged, [this] {
            this->filterRegisters(this->searchInput->text());
        });

        const auto& registerDescriptors = targetDescriptor.registerDescriptorsByType;

        auto registerDescriptorsByGroupName = std::map<QString, std::set<TargetRegisterDescriptor>>({
            {
                "CPU General Purpose",
                std::set<TargetRegisterDescriptor>(
                    registerDescriptors.at(TargetRegisterType::GENERAL_PURPOSE_REGISTER).begin(),
                    registerDescriptors.at(TargetRegisterType::GENERAL_PURPOSE_REGISTER).end()
                )
            }
        });

        for (const auto& registerDescriptor : registerDescriptors.at(TargetRegisterType::OTHER)) {
            const auto groupName = QString::fromStdString(registerDescriptor.groupName.value_or("other")).toUpper();
            registerDescriptorsByGroupName[groupName].insert(registerDescriptor);
        }

        for (const auto& registerDescriptor : registerDescriptors.at(TargetRegisterType::PORT_REGISTER)) {
            const auto groupName = QString::fromStdString(registerDescriptor.groupName.value_or("other")).toUpper();
            registerDescriptorsByGroupName[groupName].insert(registerDescriptor);
        }

        for (const auto& [groupName, registerDescriptors] : registerDescriptorsByGroupName) {
            this->registerGroupItems.emplace_back(new RegisterGroupItem(
                groupName,
                registerDescriptors,
                this->registerItemsByDescriptor
            ));

            this->registerDescriptors.insert(registerDescriptors.begin(), registerDescriptors.end());
        }

        this->registerListView = new ListView(
            std::vector<ListItem*>(this->registerGroupItems.begin(), this->registerGroupItems.end()),
            this
        );

        this->registerListScene = this->registerListView->listScene();

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
        for (const auto& groupItem : this->registerGroupItems) {
            auto visibleItems = std::uint32_t{0};
            auto displayEntireGroup = keyword.isEmpty() || groupItem->groupName.contains(keyword, Qt::CaseInsensitive);

            for (auto& registerItem : groupItem->registerItems) {
                registerItem->excluded = !displayEntireGroup
                    && !registerItem->searchKeywords.contains(keyword, Qt::CaseInsensitive);

                if (!registerItem->excluded) {
                    ++visibleItems;
                }
            }

            groupItem->setVisible(visibleItems > 0 || keyword.isEmpty());
            groupItem->setExpanded(visibleItems > 0 && !keyword.isEmpty());
        }

        this->registerListScene->refreshGeometry();
    }

    void TargetRegistersPaneWidget::collapseAllRegisterGroups() {
        for (auto& registerGroupItem : this->registerGroupItems) {
            registerGroupItem->setExpanded(false);
        }

        this->registerListScene->refreshGeometry();
    }

    void TargetRegistersPaneWidget::expandAllRegisterGroups() {
        for (auto& registerGroupItem : this->registerGroupItems) {
            registerGroupItem->setExpanded(true);
        }

        this->registerListScene->refreshGeometry();
    }

    void TargetRegistersPaneWidget::refreshRegisterValues(
        std::optional<Targets::TargetRegisterDescriptor> registerDescriptor,
        std::optional<std::function<void(void)>> callback
    ) {
        if (!registerDescriptor.has_value() && this->registerDescriptors.empty()) {
            return;
        }

        auto* readRegisterTask = new ReadTargetRegisters(
            registerDescriptor.has_value()
                ? Targets::TargetRegisterDescriptors({*registerDescriptor})
                : this->registerDescriptors
        );

        QObject::connect(
            readRegisterTask,
            &ReadTargetRegisters::targetRegistersRead,
            this,
            &TargetRegistersPaneWidget::onRegistersRead
        );

        if (callback.has_value()) {
            QObject::connect(
                readRegisterTask,
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
        auto* registerGroupItem = dynamic_cast<RegisterGroupItem*>(clickedItem);

        if (registerGroupItem != nullptr) {
            registerGroupItem->setExpanded(!registerGroupItem->isExpanded());
            this->registerListScene->refreshGeometry();
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

        auto* menu = new QMenu(this);
        menu->addAction(this->openInspectionWindowAction);
        menu->addAction(this->refreshValueAction);
        menu->addSeparator();

        auto* copyMenu = new QMenu("Copy", this);
        copyMenu->addAction(this->copyNameAction);
        copyMenu->addSeparator();
        copyMenu->addAction(this->copyValueDecimalAction);
        copyMenu->addAction(this->copyValueHexAction);
        copyMenu->addAction(this->copyValueBinaryAction);

        menu->addMenu(copyMenu);

        this->openInspectionWindowAction->setEnabled(
            TargetRegisterInspectorWindow::registerSupported(this->contextMenuRegisterItem->registerDescriptor)
        );

        const auto targetStopped = this->targetState == Targets::TargetState::STOPPED;
        const auto targetStoppedAndValuePresent = targetStopped
            && this->currentRegisterValues.contains(this->contextMenuRegisterItem->registerDescriptor);

        this->refreshValueAction->setEnabled(targetStopped);
        this->copyValueDecimalAction->setEnabled(targetStoppedAndValuePresent);
        this->copyValueHexAction->setEnabled(targetStoppedAndValuePresent);
        this->copyValueBinaryAction->setEnabled(targetStoppedAndValuePresent);

        menu->exec(sourcePosition);
    }

    void TargetRegistersPaneWidget::onTargetStateChanged(Targets::TargetState newState) {
        if (this->targetState == newState) {
            return;
        }

        this->targetState = newState;

        if (this->targetState == Targets::TargetState::RUNNING) {
            this->clearInlineRegisterValues();
        }
    }

    void TargetRegistersPaneWidget::onRegistersRead(const Targets::TargetRegisters& registers) {
        for (const auto& targetRegister : registers) {
            const auto& descriptor = targetRegister.descriptor;
            const auto& previousValueIt = this->currentRegisterValues.find(descriptor);
            const auto& registerItemIt = this->registerItemsByDescriptor.find(descriptor);

            if (registerItemIt != this->registerItemsByDescriptor.end()) {
                auto& registerItem = registerItemIt->second;

                registerItem->setValue(targetRegister.value);
                registerItem->valueChanged = previousValueIt != this->currentRegisterValues.end()
                    ? previousValueIt->second != targetRegister.value
                    : false;
            }

            this->currentRegisterValues[descriptor] = targetRegister.value;
        }

        this->registerListScene->update();
    }

    void TargetRegistersPaneWidget::clearInlineRegisterValues() {
        for (auto& [registerDescriptor, registerItem] : this->registerItemsByDescriptor) {
            registerItem->clearValue();
        }

        this->registerListScene->update();
    }

    void TargetRegistersPaneWidget::openInspectionWindow(const TargetRegisterDescriptor& registerDescriptor) {
        if (!TargetRegisterInspectorWindow::registerSupported(registerDescriptor)) {
            return;
        }

        TargetRegisterInspectorWindow* inspectionWindow = nullptr;

        const auto& currentValueIt = this->currentRegisterValues.find(registerDescriptor);
        const auto& inspectionWindowIt = this->inspectionWindowsByDescriptor.find(registerDescriptor);

        if (inspectionWindowIt != this->inspectionWindowsByDescriptor.end()) {
            inspectionWindow = inspectionWindowIt->second;

        } else {
            inspectionWindow = new TargetRegisterInspectorWindow(
                registerDescriptor,
                this->targetState,
                this
            );

            this->inspectionWindowsByDescriptor.insert(std::pair(
                registerDescriptor,
                inspectionWindow
            ));
        }

        if (currentValueIt != this->currentRegisterValues.end()) {
            inspectionWindow->setValue(currentValueIt->second);
        }

        inspectionWindow->show();
        inspectionWindow->activateWindow();
    }

    void TargetRegistersPaneWidget::copyRegisterName(const TargetRegisterDescriptor& registerDescriptor) {
        QApplication::clipboard()->setText(QString::fromStdString(registerDescriptor.name.value_or("")).toUpper());
    }

    void TargetRegistersPaneWidget::copyRegisterValueHex(const TargetRegisterDescriptor& registerDescriptor) {
        const auto& valueIt = this->currentRegisterValues.find(registerDescriptor);

        if (valueIt == this->currentRegisterValues.end()) {
            return;
        }

        const auto& value = valueIt->second;
        const auto valueByteArray = QByteArray(
            reinterpret_cast<const char*>(value.data()),
            static_cast<qsizetype>(value.size())
        ).toHex();

        QApplication::clipboard()->setText(QString(valueByteArray).toUpper());
    }

    void TargetRegistersPaneWidget::copyRegisterValueDecimal(const TargetRegisterDescriptor& registerDescriptor) {
        const auto& valueIt = this->currentRegisterValues.find(registerDescriptor);

        if (valueIt == this->currentRegisterValues.end()) {
            return;
        }

        const auto& value = valueIt->second;
        const auto valueByteArray = QByteArray(
            reinterpret_cast<const char*>(value.data()),
            static_cast<qsizetype>(value.size())
        ).toHex();

        QApplication::clipboard()->setText(QString::number(valueByteArray.toUInt(nullptr, 16)));
    }

    void TargetRegistersPaneWidget::copyRegisterValueBinary(const TargetRegisterDescriptor& registerDescriptor) {
        const auto& valueIt = this->currentRegisterValues.find(registerDescriptor);

        if (valueIt == this->currentRegisterValues.end()) {
            return;
        }

        const auto& value = valueIt->second;
        const auto valueByteArray = QByteArray(
            reinterpret_cast<const char*>(value.data()),
            static_cast<qsizetype>(value.size())
        ).toHex();

        auto bitString = QString::number(valueByteArray.toUInt(nullptr, 16), 2);

        if (bitString.size() < (value.size() * 8)) {
            bitString = bitString.rightJustified(static_cast<qsizetype>(value.size() * 8), '0');
        }

        QApplication::clipboard()->setText(bitString);
    }
}
