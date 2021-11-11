#include "TargetMemoryInspectionPane.hpp"

#include <QVBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <set>

#include "src/Insight/UserInterfaces/InsightWindow/UiLoader.hpp"

#include "src/Insight/InsightWorker/Tasks/ReadTargetMemory.hpp"
#include "src/Insight/InsightWorker/Tasks/ReadStackPointer.hpp"

#include "src/Helpers/Paths.hpp"
#include "src/Exceptions/Exception.hpp"
#include "src/Logger/Logger.hpp"

using namespace Bloom::Widgets;
using namespace Bloom::Exceptions;

using Bloom::Targets::TargetMemoryDescriptor;
using Bloom::Targets::TargetMemoryType;

TargetMemoryInspectionPane::TargetMemoryInspectionPane(
    const TargetMemoryDescriptor& targetMemoryDescriptor,
    InsightWorker& insightWorker,
    PanelWidget* parent
): QWidget(parent), parent(parent), targetMemoryDescriptor(targetMemoryDescriptor), insightWorker(insightWorker) {
    this->setObjectName("target-memory-inspection-pane");

    auto memoryInspectionPaneUiFile = QFile(
        QString::fromStdString(Paths::compiledResourcesPath()
            + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/UiFiles/TargetMemoryInspectionPane.ui"
        )
    );

    if (!memoryInspectionPaneUiFile.open(QFile::ReadOnly)) {
        throw Exception("Failed to open MemoryInspectionPane UI file");
    }

    auto uiLoader = UiLoader(this);
    this->container = uiLoader.load(&memoryInspectionPaneUiFile, this);
    this->container->setFixedSize(parent->width(), parent->maximumHeight());

    this->titleBar = this->container->findChild<QWidget*>("title-bar");

    this->titleBar->layout()->setContentsMargins(7, 0, 7, 0);
    auto titleLabel = this->titleBar->findChild<QLabel*>("title");
    titleLabel->setText(
        this->targetMemoryDescriptor.type == TargetMemoryType::EEPROM ? "Internal EEPROM" : "Internal RAM"
    );

    auto subContainerLayout = this->container->findChild<QHBoxLayout*>("sub-container-layout");
    this->hexViewerWidget = new HexViewerWidget(this->targetMemoryDescriptor, this->insightWorker, this);
    this->hexViewerWidget->setDisabled(true);

    subContainerLayout->addWidget(this->hexViewerWidget);

    QObject::connect(
        &insightWorker,
        &InsightWorker::targetStateUpdated,
        this,
        &TargetMemoryInspectionPane::onTargetStateChanged
    );

    QObject::connect(
        this->hexViewerWidget->refreshButton,
        &QToolButton::clicked,
        this,
        [this] {
            this->refreshMemoryValues();
        }
    );
}

void TargetMemoryInspectionPane::refreshMemoryValues(std::optional<std::function<void(void)>> callback) {
    this->hexViewerWidget->refreshButton->setDisabled(true);

    auto* readMemoryTask = new ReadTargetMemory(
        this->targetMemoryDescriptor.type,
        this->targetMemoryDescriptor.addressRange.startAddress,
        this->targetMemoryDescriptor.size()
    );

    QObject::connect(
        readMemoryTask,
        &ReadTargetMemory::targetMemoryRead,
        this,
        [this] (const Targets::TargetMemoryBuffer& buffer) {
            this->onMemoryRead(buffer);

            auto* readStackPointerTask = new ReadStackPointer();
            QObject::connect(
                readStackPointerTask,
                &ReadStackPointer::stackPointerRead,
                this,
                [this] (std::uint32_t stackPointer) {
                    this->hexViewerWidget->setStackPointer(stackPointer);
                }
            );

            this->insightWorker.queueTask(readStackPointerTask);
        }
    );

    QObject::connect(
        readMemoryTask,
        &InsightWorkerTask::completed,
        this,
        [this] {
            this->hexViewerWidget->refreshButton->setDisabled(false);
        }
    );

    if (callback.has_value()) {
        QObject::connect(
            readMemoryTask,
            &InsightWorkerTask::completed,
            this,
            callback.value()
        );
    }

    this->insightWorker.queueTask(readMemoryTask);
}

void TargetMemoryInspectionPane::activate() {
    this->show();
    this->activated = true;
    this->postActivate();
}

void TargetMemoryInspectionPane::deactivate() {
    this->hide();
    this->activated = false;
    this->postDeactivate();
}

void TargetMemoryInspectionPane::resizeEvent(QResizeEvent* event) {
    const auto parentSize = this->parent->size();
    const auto width = parentSize.width() - 1;
    this->container->setFixedSize(width, parentSize.height());
}

void TargetMemoryInspectionPane::postActivate() {
    if (this->targetState == Targets::TargetState::STOPPED) {
        this->refreshMemoryValues([this] {
            this->hexViewerWidget->setDisabled(false);
        });
    }
}

void TargetMemoryInspectionPane::postDeactivate() {

}

void TargetMemoryInspectionPane::onTargetStateChanged(Targets::TargetState newState) {
    using Targets::TargetState;
    this->targetState = newState;

    if (this->activated) {
        if (newState == TargetState::STOPPED) {
            this->refreshMemoryValues([this] {
                this->hexViewerWidget->setDisabled(false);
            });

        } else {
            this->hexViewerWidget->setDisabled(true);
        }
    }
}

void TargetMemoryInspectionPane::onMemoryRead(const Targets::TargetMemoryBuffer& buffer) {
    this->hexViewerWidget->updateValues(buffer);
}
