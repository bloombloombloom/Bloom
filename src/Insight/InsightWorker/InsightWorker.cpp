#include "InsightWorker.hpp"

#include <QObject>
#include <string>

#include "src/Insight/InsightSignals.hpp"
#include "src/Logger/Logger.hpp"

using namespace Exceptions;

using Targets::TargetState;

void InsightWorker::startup() {
    Logger::setThreadName("IW" + std::to_string(this->id));

    auto* insightSignals = InsightSignals::instance();

    QObject::connect(
        insightSignals,
        &InsightSignals::taskQueued,
        this,
        &InsightWorker::executeTasks,
        Qt::ConnectionType::QueuedConnection
    );
    QObject::connect(
        insightSignals,
        &InsightSignals::taskProcessed,
        this,
        &InsightWorker::executeTasks,
        Qt::ConnectionType::QueuedConnection
    );

    Logger::debug("InsightWorker" + std::to_string(this->id) + " ready");
    emit this->ready();
}

void InsightWorker::queueTask(const QSharedPointer<InsightWorkerTask>& task) {
    task->moveToThread(nullptr);

    InsightWorker::queuedTasksById.accessor()->emplace(task->id, task);

    emit InsightSignals::instance()->taskQueued(task);
}

void InsightWorker::executeTasks() {
    static const auto getQueuedTask = [] () -> std::optional<QSharedPointer<InsightWorkerTask>> {
        auto queuedTasks = InsightWorker::queuedTasksById.accessor();

        if (!queuedTasks->empty()) {
            auto taskGroupsInExecution = InsightWorker::taskGroupsInExecution.accessor();

            const auto canExecuteTask = [&taskGroupsInExecution] (const QSharedPointer<InsightWorkerTask>& task) {
                for (const auto taskGroup : task->taskGroups()) {
                    if (taskGroupsInExecution->contains(taskGroup)) {
                        return false;
                    }
                }

                return true;
            };

            for (auto [queuedTaskId, task] : *queuedTasks) {
                if (canExecuteTask(task)) {
                    const auto taskGroups = task->taskGroups();
                    taskGroupsInExecution->insert(taskGroups.begin(), taskGroups.end());
                    queuedTasks->erase(queuedTaskId);
                    return task;
                }
            }
        }

        return std::nullopt;
    };

    auto queuedTask = std::optional<QSharedPointer<InsightWorkerTask>>{};

    while ((queuedTask = getQueuedTask())) {
        auto& task = *queuedTask;
        task->moveToThread(this->thread());
        task->execute(this->targetControllerService);

        {
            auto taskGroupsInExecution = InsightWorker::taskGroupsInExecution.accessor();

            for (const auto& taskGroup : task->taskGroups()) {
                taskGroupsInExecution->erase(taskGroup);
            }
        }

        emit InsightSignals::instance()->taskProcessed(task);
    }
}
