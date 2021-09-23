#include "RegisterWidget.hpp"

#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QStyle>

#include "src/Helpers/Paths.hpp"

#include "src/Insight/InsightWorker/Tasks/ReadTargetRegisters.hpp"

using namespace Bloom::Widgets;
using namespace Bloom::Exceptions;

using Bloom::Targets::TargetRegisterDescriptor;

RegisterWidget::RegisterWidget(
    TargetRegisterDescriptor descriptor,
    InsightWorker& insightWorker,
    QWidget *parent
): ItemWidget(parent), descriptor(descriptor), insightWorker(insightWorker) {
    this->setObjectName("register-item");
    this->setFixedHeight(25);

    this->nameLabel->setText(QString::fromStdString(this->descriptor.name.value()).toUpper());
    this->valueLabel->setObjectName("value");

    this->registerIcon->setObjectName("register-icon");
    auto static registerIconPath = QString::fromStdString(
        Paths::compiledResourcesPath()
            + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetRegistersPane/Images/register.svg"
    );
    this->registerIcon->setSvgFilePath(registerIconPath);
    this->registerIcon->setContainerHeight(15);
    this->registerIcon->setContainerWidth(15);

    this->layout->setContentsMargins(47, 0, 0, 0);
    this->layout->setSpacing(0);
    this->layout->addWidget(this->registerIcon);
    this->layout->addSpacing(7);
    this->layout->addWidget(this->nameLabel);
    this->layout->addSpacing(5);
    this->layout->addWidget(this->valueLabel);
    this->layout->addStretch(1);

    this->connect(this, &ClickableWidget::doubleClicked, this, &RegisterWidget::openInspectionWindow);
    this->connect(this->openInspectionWindowAction, &QAction::triggered, this, &RegisterWidget::openInspectionWindow);
    this->connect(this->refreshValueAction, &QAction::triggered, this, &RegisterWidget::refreshValue);
    this->connect(this->copyValueNameAction, &QAction::triggered, this, &RegisterWidget::copyName);
    this->connect(this->copyValueHexAction, &QAction::triggered, this, &RegisterWidget::copyValueHex);
    this->connect(this->copyValueDecimalAction, &QAction::triggered, this, &RegisterWidget::copyValueDecimal);
    this->connect(this->copyValueBinaryAction, &QAction::triggered, this, &RegisterWidget::copyValueBinary);

    this->connect(
        &(this->insightWorker),
        &InsightWorker::targetStateUpdated,
        this,
        &RegisterWidget::onTargetStateChange
    );
}

void RegisterWidget::setRegisterValue(const Targets::TargetRegister& targetRegister) {
    const auto valueChanged = this->currentRegister.has_value()
        && this->currentRegister.value().value != targetRegister.value;
    this->currentRegister = targetRegister;

    auto valueByteArray = QByteArray(
        reinterpret_cast<const char*>(targetRegister.value.data()),
        static_cast<qsizetype>(targetRegister.value.size())
    );

    auto hexValueByteArray = valueByteArray.toHex();
    auto registerValue = ": 0x" + QString(hexValueByteArray).toUpper()
        + " | " + QString::number(hexValueByteArray.toUInt(nullptr, 16));

    if (targetRegister.value.size() == 1 && targetRegister.value[0] >= 32 && targetRegister.value[0] <= 126) {
        registerValue += " | '" + QString(valueByteArray) + "'";
    }

    this->valueLabel->setProperty("changed", valueChanged);
    this->valueLabel->style()->unpolish(this->valueLabel);
    this->valueLabel->style()->polish(this->valueLabel);

    this->valueLabel->setText(registerValue);

    if (this->inspectWindow != nullptr) {
        this->inspectWindow->setValue(targetRegister.value);
    }
}

void RegisterWidget::clearInlineValue() {
    this->valueLabel->clear();
}

void RegisterWidget::openInspectionWindow() {
    if (!TargetRegisterInspectorWindow::registerSupported(this->descriptor)) {
        return;
    }

    if (this->inspectWindow == nullptr) {
        this->inspectWindow = new TargetRegisterInspectorWindow(
            this->descriptor,
            this->insightWorker,
            this->targetState,
            this->currentRegister.has_value() ? std::optional(this->currentRegister->value) : std::nullopt,
            this
        );

    } else {
        if (this->currentRegister.has_value()) {
            this->inspectWindow->setValue(this->currentRegister->value);
        }

        if (!this->inspectWindow->isVisible()) {
            this->inspectWindow->show();

        } else {
            this->inspectWindow->activateWindow();
        }
    }
}

void RegisterWidget::refreshValue() {
    auto readRegisterTask = new ReadTargetRegisters({this->descriptor});

    this->connect(
        readRegisterTask,
        &ReadTargetRegisters::targetRegistersRead,
        this,
        [this] (Targets::TargetRegisters registers) {
            for (const auto& targetRegister : registers) {
                if (targetRegister.descriptor == this->descriptor) {
                    this->setRegisterValue(targetRegister);
                }
            }
        }
    );

    this->insightWorker.queueTask(readRegisterTask);
}

void RegisterWidget::copyName() {
    if (this->nameLabel != nullptr) {
        QApplication::clipboard()->setText(this->nameLabel->text());
    }
}

void RegisterWidget::copyValueHex() {
    if (this->currentRegister.has_value()) {
        auto valueByteArray = QByteArray(
            reinterpret_cast<const char*>(this->currentRegister.value().value.data()),
            static_cast<qsizetype>(this->currentRegister.value().value.size())
        ).toHex();
        QApplication::clipboard()->setText(QString(valueByteArray).toUpper());
    }
}

void RegisterWidget::copyValueDecimal() {
    if (this->currentRegister.has_value()) {
        auto valueByteArray = QByteArray(
            reinterpret_cast<const char*>(this->currentRegister.value().value.data()),
            static_cast<qsizetype>(this->currentRegister.value().value.size())
        ).toHex();
        QApplication::clipboard()->setText(QString::number(valueByteArray.toUInt(nullptr, 16)));
    }
}

void RegisterWidget::copyValueBinary() {
    if (this->currentRegister.has_value()) {
        const auto registerValueSize = static_cast<qsizetype>(this->currentRegister.value().size());
        auto valueByteArray = QByteArray(
            reinterpret_cast<const char*>(this->currentRegister.value().value.data()),
            registerValueSize
        ).toHex();

        auto bitString = QString::number(valueByteArray.toUInt(nullptr, 16), 2);

        if (bitString.size() < (registerValueSize * 8)) {
            bitString = bitString.rightJustified((registerValueSize * 8), '0');
        }

        QApplication::clipboard()->setText(bitString);
    }
}

void RegisterWidget::contextMenuEvent(QContextMenuEvent* event) {
    this->setSelected(true);

    auto menu = new QMenu(this);
    menu->addAction(this->openInspectionWindowAction);
    menu->addAction(this->refreshValueAction);
    menu->addSeparator();

    auto copyMenu = new QMenu("Copy", this);
    copyMenu->addAction(this->copyValueNameAction);
    copyMenu->addSeparator();
    copyMenu->addAction(this->copyValueDecimalAction);
    copyMenu->addAction(this->copyValueHexAction);
    copyMenu->addAction(this->copyValueBinaryAction);

    menu->addMenu(copyMenu);

    this->openInspectionWindowAction->setEnabled(TargetRegisterInspectorWindow::registerSupported(this->descriptor));

    const auto targetStopped = this->targetState == Targets::TargetState::STOPPED;
    const auto targetStoppedAndValuePresent = targetStopped && this->currentRegister.has_value();
    this->refreshValueAction->setEnabled(targetStopped);
    this->copyValueDecimalAction->setEnabled(targetStoppedAndValuePresent);
    this->copyValueHexAction->setEnabled(targetStoppedAndValuePresent);
    this->copyValueBinaryAction->setEnabled(targetStoppedAndValuePresent);

    menu->exec(event->globalPos());
}

void RegisterWidget::postSetSelected(bool selected) {
    auto valueLabelStyle = this->valueLabel->style();
    valueLabelStyle->unpolish(this->valueLabel);
    valueLabelStyle->polish(this->valueLabel);
}

void RegisterWidget::onTargetStateChange(Targets::TargetState newState) {
    this->targetState = newState;

    if (this->targetState == Targets::TargetState::RUNNING) {
        this->clearInlineValue();
    }
}
