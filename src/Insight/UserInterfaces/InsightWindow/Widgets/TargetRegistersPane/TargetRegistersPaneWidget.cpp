#include "TargetRegistersPaneWidget.hpp"

#include <QVBoxLayout>
#include <set>

#include "src/Insight/UserInterfaces/InsightWindow/UiLoader.hpp"
#include "src/Insight/InsightSignals.hpp"
#include "src/Insight/InsightWorker/InsightWorker.hpp"
#include "RegisterGroupWidget.hpp"
#include "RegisterWidget.hpp"

#include "src/Helpers/Paths.hpp"
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
            QString::fromStdString(Paths::compiledResourcesPath()
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

        this->itemScrollArea = this->container->findChild<QScrollArea*>("item-scroll-area");

        this->itemContainer = this->container->findChild<QWidget*>("item-container");
        auto itemLayout = this->itemContainer->findChild<QVBoxLayout*>();

        auto& registerDescriptors = targetDescriptor.registerDescriptorsByType;
        this->renderedDescriptors = std::set<TargetRegisterDescriptor>(
            registerDescriptors.at(TargetRegisterType::GENERAL_PURPOSE_REGISTER).begin(),
            registerDescriptors.at(TargetRegisterType::GENERAL_PURPOSE_REGISTER).end()
        );

        auto* generalPurposeRegisterGroupWidget = new RegisterGroupWidget(
            "CPU General Purpose",
            this->renderedDescriptors,
            this
        );

        itemLayout->addWidget(generalPurposeRegisterGroupWidget, 0, Qt::AlignTop);
        this->registerGroupWidgets.insert(generalPurposeRegisterGroupWidget);

        auto registerDescriptorsByGroupName = std::map<std::string, std::set<TargetRegisterDescriptor>>();
        for (const auto& registerDescriptor : registerDescriptors.at(TargetRegisterType::OTHER)) {
            registerDescriptorsByGroupName[registerDescriptor.groupName.value_or("other")].insert(registerDescriptor);
        }

        for (const auto& registerDescriptor : registerDescriptors.at(TargetRegisterType::PORT_REGISTER)) {
            registerDescriptorsByGroupName[registerDescriptor.groupName.value_or("other")].insert(registerDescriptor);
        }

        for (const auto& [groupName, registerDescriptors] : registerDescriptorsByGroupName) {
            auto* registerGroupWidget = new RegisterGroupWidget(
                QString::fromStdString(groupName).toUpper(),
                registerDescriptors,
                this
            );

            itemLayout->addWidget(registerGroupWidget, 0, Qt::AlignTop);
            this->registerGroupWidgets.insert(registerGroupWidget);
            this->renderedDescriptors.insert(registerDescriptors.begin(), registerDescriptors.end());
        }

        itemLayout->addStretch(1);

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
        for (const auto& registerGroupWidget : this->registerGroupWidgets) {
            // If the group name matches the keyword, then don't bother iterating through all the register widgets
            if (keyword.isEmpty() || registerGroupWidget->name.contains(keyword, Qt::CaseInsensitive)) {
                registerGroupWidget->setVisible(true);
                registerGroupWidget->setAllRegistersVisible(true);

                if (!keyword.isEmpty()) {
                    registerGroupWidget->expand();

                } else {
                    registerGroupWidget->collapse();
                }

            } else {
                registerGroupWidget->filterRegisters(keyword);
            }
        }
    }

    void TargetRegistersPaneWidget::collapseAllRegisterGroups() {
        for (const auto& registerGroupWidget : this->registerGroupWidgets) {
            registerGroupWidget->collapse();
        }
    }

    void TargetRegistersPaneWidget::expandAllRegisterGroups() {
        for (const auto& registerGroupWidget : this->registerGroupWidgets) {
            registerGroupWidget->expand();
        }
    }

    void TargetRegistersPaneWidget::refreshRegisterValues(std::optional<std::function<void(void)>> callback) {
        auto& descriptors = this->renderedDescriptors;

        if (descriptors.empty()) {
            return;
        }

        auto* readRegisterTask = new ReadTargetRegisters(descriptors);
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

    void TargetRegistersPaneWidget::onItemSelectionChange(ItemWidget* newlySelectedWidget) {
        // Only one item in the target registers pane can be selected at any given time.
        if (this->selectedItemWidget != newlySelectedWidget) {
            if (this->selectedItemWidget != nullptr) {
                this->selectedItemWidget->setSelected(false);
            }

            this->selectedItemWidget = newlySelectedWidget;
        }
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
        this->itemScrollArea->setFixedWidth(width - this->parentPanel->getHandleSize());

        PaneWidget::resizeEvent(event);
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

            for (const auto& registerGroupWidget : this->registerGroupWidgets) {
                if (registerGroupWidget->registerWidgetsMappedByDescriptor.contains(descriptor)) {
                    registerGroupWidget->registerWidgetsMappedByDescriptor.at(
                        descriptor
                    )->setRegisterValue(targetRegister);
                    break;
                }
            }
        }
    }

    void TargetRegistersPaneWidget::clearInlineRegisterValues() {
        for (const auto& registerGroupWidget : this->registerGroupWidgets) {
            for (auto* registerWidget : registerGroupWidget->registerWidgets) {
                registerWidget->clearInlineValue();
            }
        }
    }
}
