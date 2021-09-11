#include "TargetRegistersPaneWidget.hpp"

#include <QVBoxLayout>
#include <QScrollArea>
#include <set>

#include "../../UiLoader.hpp"
#include "../ExpandingWidget.hpp"
#include "RegisterGroupWidget.hpp"
#include "RegisterWidget.hpp"

#include "src/Helpers/Paths.hpp"
#include "src/Exceptions/Exception.hpp"

#include "src/Insight/InsightWorker/Tasks/ReadTargetRegisters.hpp"

using namespace Bloom::Widgets;
using namespace Bloom::Exceptions;

using Bloom::Targets::TargetDescriptor;
using Bloom::Targets::TargetRegisterDescriptor;
using Bloom::Targets::TargetRegisterDescriptors;
using Bloom::Targets::TargetRegisterType;

TargetRegistersPaneWidget::TargetRegistersPaneWidget(
    const TargetDescriptor& targetDescriptor,
    InsightWorker& insightWorker,
    QWidget* parent
): QWidget(parent), parent(parent), targetDescriptor(targetDescriptor), insightWorker(insightWorker) {
    this->setObjectName("target-registers-side-pane");

    auto targetRegistersPaneUiFile = QFile(
        QString::fromStdString(Paths::compiledResourcesPath()
            + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetRegistersPane/UiFiles/TargetRegistersSidePane.ui"
        )
    );

    if (!targetRegistersPaneUiFile.open(QFile::ReadOnly)) {
        throw Exception("Failed to open TargetRegistersSidePane UI file");
    }

    auto uiLoader = UiLoader(this);
    this->container = uiLoader.load(&targetRegistersPaneUiFile, this);
    this->container->setFixedSize(parent->width(), parent->maximumHeight());

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(this->container);

    this->toolBar = this->container->findChild<QWidget*>("tool-bar");
    this->collapseAllButton = this->toolBar->findChild<SvgToolButton*>("collapse-all-button");
    this->expandAllButton = this->toolBar->findChild<SvgToolButton*>("expand-all-button");
    this->toolBar->layout()->setContentsMargins(5, 0, 5, 0);
    this->searchInput = this->container->findChild<QLineEdit*>("search-input");

    this->connect(this->expandAllButton, &QToolButton::clicked, [this] {
        this->expandAllRegisterGroups();
    });

    this->connect(this->collapseAllButton, &QToolButton::clicked, [this] {
        this->collapseAllRegisterGroups();
    });

    this->connect(this->searchInput, &QLineEdit::textChanged, [this] {
        this->filterRegisters(this->searchInput->text());
    });

    this->itemContainer = this->container->findChild<QWidget*>("item-container");
    auto itemLayout = this->itemContainer->findChild<QVBoxLayout*>();

    auto& registerDescriptors = targetDescriptor.registerDescriptorsByType;
    this->renderedDescriptors = std::set<TargetRegisterDescriptor>(
        registerDescriptors.at(TargetRegisterType::GENERAL_PURPOSE_REGISTER).begin(),
        registerDescriptors.at(TargetRegisterType::GENERAL_PURPOSE_REGISTER).end()
    );

    auto generalPurposeRegisterGroupWidget = new RegisterGroupWidget(
        "CPU General Purpose",
        this->renderedDescriptors,
        insightWorker,
        this
    );

    itemLayout->addWidget(generalPurposeRegisterGroupWidget, 0, Qt::AlignTop);
    this->registerGroupWidgets.insert(generalPurposeRegisterGroupWidget);

    auto registerDescriptorsByGroupName = std::map<std::string, std::set<TargetRegisterDescriptor>>();
    for (const auto& registerDescriptor : registerDescriptors.at(TargetRegisterType::OTHER)) {
        registerDescriptorsByGroupName[registerDescriptor.groupName.value_or("other")].insert(registerDescriptor);
        auto groupName = registerDescriptor.groupName.value_or("other");

    }

    for (const auto& [groupName, registerDescriptors] : registerDescriptorsByGroupName) {
        auto registerGroupWidget = new RegisterGroupWidget(
            QString::fromStdString(groupName).toUpper(),
            registerDescriptors,
            insightWorker,
            this
        );

        itemLayout->addWidget(registerGroupWidget, 0, Qt::AlignTop);
        this->registerGroupWidgets.insert(registerGroupWidget);
        this->renderedDescriptors.insert(registerDescriptors.begin(), registerDescriptors.end());
    }

    itemLayout->addStretch(1);

    this->connect(
        &insightWorker,
        &InsightWorker::targetStateUpdated,
        this,
        &TargetRegistersPaneWidget::onTargetStateChanged
    );

    this->connect(
        &insightWorker,
        &InsightWorker::targetRegistersWritten,
        this,
        &TargetRegistersPaneWidget::onRegistersWritten
    );
}

void TargetRegistersPaneWidget::resizeEvent(QResizeEvent* event) {
    auto parentSize = this->parent->size();
    this->container->setFixedSize(
        parentSize.width() - 1,
        parentSize.height()
    );
    this->searchInput->setFixedWidth(parentSize.width() - 20);
}

void TargetRegistersPaneWidget::postActivate() {
    if (this->targetState == Targets::TargetState::STOPPED) {
        this->refreshRegisterValues();
    }
}

void TargetRegistersPaneWidget::postDeactivate() {

}

void TargetRegistersPaneWidget::onTargetStateChanged(Targets::TargetState newState) {
    using Targets::TargetState;
    this->targetState = newState;

    if (newState == TargetState::STOPPED && this->activated) {
        this->refreshRegisterValues();
    }
}


void TargetRegistersPaneWidget::onRegistersWritten(const Bloom::Targets::TargetRegisterDescriptors& descriptors) {
    if (this->targetState != Targets::TargetState::STOPPED) {
        return;
    }

    /*
     * Don't bother refreshing individual registers if it will involve more than two refresh calls - In this case, it
     * will be faster to just refresh all of them at once.
     */
    if (descriptors.size() <= 2) {
        for (const auto& descriptor : descriptors) {
            for (const auto& registerGroupWidget : this->registerGroupWidgets) {
                if (registerGroupWidget->registerWidgetsMappedByDescriptor.contains(descriptor)) {
                    registerGroupWidget->registerWidgetsMappedByDescriptor.at(descriptor)->refreshValue();
                    break;
                }
            }
        }

    } else {
        this->refreshRegisterValues();
    }
}

void TargetRegistersPaneWidget::filterRegisters(const QString& keyword) {
    auto stdKeyword = keyword.toLower().toStdString();

    for (auto& registerGroupWidget : this->registerGroupWidgets) {
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
            registerGroupWidget->filterRegisters(stdKeyword);
        }
    }
}

void TargetRegistersPaneWidget::collapseAllRegisterGroups() {
    for (auto& registerGroupWidget : this->registerGroupWidgets) {
        registerGroupWidget->collapse();
    }
}

void TargetRegistersPaneWidget::expandAllRegisterGroups() {
    for (auto& registerGroupWidget : this->registerGroupWidgets) {
        registerGroupWidget->expand();
    }
}

void TargetRegistersPaneWidget::refreshRegisterValues(std::optional<std::function<void(void)>> callback) {
    auto& descriptors = this->renderedDescriptors;

    if (descriptors.empty()) {
        return;
    }

    auto readRegisterTask = new ReadTargetRegisters(descriptors);
    this->connect(
        readRegisterTask,
        &ReadTargetRegisters::targetRegistersRead,
        this,
        &TargetRegistersPaneWidget::onRegistersRead
    );

    if (callback.has_value()) {
        this->connect(
            readRegisterTask,
            &InsightWorkerTask::completed,
            this,
            callback.value()
        );
    }

    this->insightWorker.queueTask(readRegisterTask);
}

void TargetRegistersPaneWidget::clearInlineValues() {
    for (const auto& registerGroupWidget : this->registerGroupWidgets) {
        for (auto& [registerDescriptor, registerWidget] : registerGroupWidget->registerWidgetsMappedByDescriptor) {
            registerWidget->clearInlineValue();
        }
    }
}

void TargetRegistersPaneWidget::activate() {
    this->show();
    this->activated = true;
    this->postActivate();
}

void TargetRegistersPaneWidget::deactivate() {
    this->hide();
    this->activated = false;
    this->postDeactivate();
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

void TargetRegistersPaneWidget::onRegistersRead(const Targets::TargetRegisters& registers) {
    for (const auto& targetRegister : registers) {
        auto& descriptor = targetRegister.descriptor;

        for (const auto& registerGroupWidget : this->registerGroupWidgets) {
            if (registerGroupWidget->registerWidgetsMappedByDescriptor.contains(descriptor)) {
                registerGroupWidget->registerWidgetsMappedByDescriptor.at(descriptor)->setRegisterValue(targetRegister);
                break;
            }
        }
    }
}
