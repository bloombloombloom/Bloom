#include "TaskWindow.hpp"

#include <QScrollArea>
#include <QTimer>

#include "src/Insight/UserInterfaces/InsightWindow/UiLoader.hpp"

#include "src/Insight/InsightSignals.hpp"

#include "src/Services/PathService.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Widgets
{
    TaskWindow::TaskWindow(QWidget* parent)
        : QWidget(parent)
    {
        this->setObjectName("task-window");
        this->setMinimumSize(900, 500);

        this->setWindowFlag(Qt::Window);
        this->setWindowTitle("Background Tasks");

        auto windowUiFile = QFile(
            QString::fromStdString(Services::PathService::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TaskWindow/UiFiles/TaskWindow.ui"
            )
        );

        auto stylesheetFile = QFile(
            QString::fromStdString(Services::PathService::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TaskWindow/Stylesheets/TaskWindow.qss"
            )
        );

        if (!windowUiFile.open(QFile::ReadOnly)) {
            throw Exceptions::Exception("Failed to open TaskWindow UI file");
        }

        if (!stylesheetFile.open(QFile::ReadOnly)) {
            throw Exceptions::Exception("Failed to open TaskWindow stylesheet file");
        }

        auto uiLoader = UiLoader(this);
        this->container = uiLoader.load(&windowUiFile, this);
        this->container->setStyleSheet(stylesheetFile.readAll());

        this->container->setContentsMargins(0, 0, 0, 0);

        this->taskWidgetLayout = this->container->findChild<QVBoxLayout*>();
        this->taskPlaceholderLabel = this->container->findChild<Label*>("loading-placeholder-label");

        auto* insightSignals = InsightSignals::instance();

        QObject::connect(
            insightSignals,
            &InsightSignals::taskQueued,
            this,
            &TaskWindow::onTaskQueued
        );
    }

    void TaskWindow::resizeEvent(QResizeEvent* event) {
        this->container->setFixedSize(this->size());
    }

    void TaskWindow::onTaskQueued(const QSharedPointer<InsightWorkerTask>& task) {
        auto* taskWidget = new Task(task, this);
        this->taskWidgetLayout->insertWidget(0, taskWidget);

        QObject::connect(taskWidget, &Task::taskComplete, this, [this] (InsightWorkerTask::IdType taskId) {
            auto* finishedSignalTimer = new QTimer();
            finishedSignalTimer->setSingleShot(true);
            finishedSignalTimer->setInterval(10000);

            QObject::connect(finishedSignalTimer, &QTimer::timeout, this, [this, taskId] {
                const auto& taskWidgetIt = this->taskWidgetsByTaskId.find(taskId);

                if (taskWidgetIt == this->taskWidgetsByTaskId.end()) {
                    return;
                }

                auto* taskWidget = taskWidgetIt->second;
                this->taskWidgetLayout->removeWidget(taskWidget);
                taskWidget->deleteLater();
                this->taskWidgetsByTaskId.erase(taskWidgetIt);

                if (this->taskWidgetsByTaskId.empty()) {
                    this->taskPlaceholderLabel->show();
                }
            });

            finishedSignalTimer->start();
        });

        this->taskWidgetsByTaskId.emplace(task->id, taskWidget);

        this->taskPlaceholderLabel->hide();
    }
}
