#include "RegisterHistoryWidget.hpp"

#include <QFile>
#include <QVBoxLayout>
#include <QMargins>
#include <QTableWidget>
#include <QScrollBar>
#include <set>

#include "src/Insight/UserInterfaces/InsightWindow/UiLoader.hpp"
#include "src/Insight/InsightSignals.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/Label.hpp"

#include "src/Services/PathService.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Widgets
{
    using namespace Exceptions;

    using Targets::TargetRegisterDescriptor;
    using Targets::TargetRegisterDescriptors;
    using Targets::TargetRegisterType;

    RegisterHistoryWidget::RegisterHistoryWidget(
        const Targets::TargetRegisterDescriptor& registerDescriptor,
        const Targets::TargetMemoryBuffer& currentValue,
        QWidget* parent
    )
        : QWidget(parent)
        , registerDescriptor(registerDescriptor)
    {
        this->setObjectName("target-register-history-widget");
        this->setFixedWidth(300);

        this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        auto widgetUiFile = QFile(
            QString::fromStdString(Services::PathService::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetRegisterInspector/RegisterHistoryWidget"
                + "/UiFiles/RegisterHistoryWidget.ui"
            )
        );

        if (!widgetUiFile.open(QFile::ReadOnly)) {
            throw Exception("Failed to open RegisterHistoryWidget UI file");
        }

        auto uiLoader = UiLoader(this);
        this->container = uiLoader.load(&widgetUiFile, this);
        this->container->setMinimumSize(this->size());
        this->container->setContentsMargins(1, 1, 1, 1);

        this->itemContainer = this->container->findChild<QWidget*>("item-container");
        this->itemContainerLayout = this->itemContainer->findChild<QVBoxLayout*>("item-container-layout");
        auto* titleBar = this->container->findChild<QWidget*>("title-bar");
        auto* title = titleBar->findChild<Label*>("title");

        titleBar->setContentsMargins(0, 0, 0, 0);
        title->setFixedHeight(titleBar->height());

        auto* insightSignals = InsightSignals::instance();

        QObject::connect(
            insightSignals,
            &InsightSignals::targetStateUpdated,
            this,
            &RegisterHistoryWidget::onTargetStateChanged
        );

        QObject::connect(
            insightSignals,
            &InsightSignals::targetRegistersWritten,
            this,
            &RegisterHistoryWidget::onRegistersWritten
        );

        this->currentItem = new CurrentItem(currentValue, this);
        QObject::connect(this->currentItem, &Item::selected, this, &RegisterHistoryWidget::onItemSelectionChange);
        this->itemContainerLayout->addWidget(this->currentItem);
        this->currentItem->setSelected(true);

        auto* separatorWidget = new QWidget(this);
        auto* separatorLayout = new QHBoxLayout(separatorWidget);
        auto* separatorLabel = new Label("Select an item to restore", separatorWidget);
        separatorWidget->setFixedHeight(40);
        separatorWidget->setObjectName("separator-widget");
        separatorLayout->setContentsMargins(0, 10, 0, 10);
        separatorLabel->setObjectName("separator-label");
        separatorLayout->addWidget(separatorLabel, 0, Qt::AlignmentFlag::AlignHCenter);
        this->itemContainerLayout->addWidget(separatorWidget);

        this->show();
    }

    void RegisterHistoryWidget::updateCurrentItemValue(const Targets::TargetMemoryBuffer& registerValue) {
        this->currentItem->registerValue = registerValue;

        if (this->selectedItemWidget != nullptr && this->currentItem == this->selectedItemWidget) {
            this->selectCurrentItem();
        }
    }

    void RegisterHistoryWidget::selectCurrentItem() {
        this->currentItem->setSelected(true);
    }

    void RegisterHistoryWidget::addItem(const Targets::TargetMemoryBuffer& registerValue, const QDateTime& changeDate) {
        auto* item = new RegisterHistoryItem(registerValue, changeDate, this->itemContainer);
        QObject::connect(item, &Item::selected, this, &RegisterHistoryWidget::onItemSelectionChange);
        this->itemContainerLayout->insertWidget(2, item);
    }

    void RegisterHistoryWidget::resizeEvent(QResizeEvent* event) {
        this->container->setMinimumSize(
            this->width(),
            this->height()
        );
    }

    void RegisterHistoryWidget::onTargetStateChanged(Targets::TargetState newState) {
        using Targets::TargetState;
        this->targetState = newState;
    }

    void RegisterHistoryWidget::onItemSelectionChange(Item* newlySelectedWidget) {
        if (this->selectedItemWidget != newlySelectedWidget) {
            if (this->selectedItemWidget != nullptr) {
                this->selectedItemWidget->setSelected(false);
            }

            this->selectedItemWidget = newlySelectedWidget;
        }

        emit this->historyItemSelected(newlySelectedWidget->registerValue);
    }

    void RegisterHistoryWidget::onRegistersWritten(
        Targets::TargetRegisters targetRegisters,
        const QDateTime& changeDate
    ) {
        for (const auto& targetRegister : targetRegisters) {
            if (targetRegister.descriptorId == this->registerDescriptor.id) {
                this->addItem(targetRegister.value, changeDate);
                this->updateCurrentItemValue(targetRegister.value);
            }
        }
    }
}
