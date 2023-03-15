#include "InsightWorker.hpp"

#include <QObject>

#include "src/Insight/InsightSignals.hpp"
#include "src/Logger/Logger.hpp"

namespace Bloom
{
    using namespace Bloom::Exceptions;

    using Bloom::Targets::TargetState;

    void InsightWorker::startup() {
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

        {
            const auto taskQueueLock = InsightWorker::queuedTasksById.acquireLock();
            InsightWorker::queuedTasksById.getValue().emplace(task->id, task);
        }

        emit InsightSignals::instance()->taskQueued(task);
    }

    void InsightWorker::executeTasks() {
        static const auto getQueuedTask = [] () -> std::optional<QSharedPointer<InsightWorkerTask>> {
            const auto taskQueueLock = InsightWorker::queuedTasksById.acquireLock();
            auto& queuedTasks = InsightWorker::queuedTasksById.getValue();

            if (!queuedTasks.empty()) {
                const auto taskGroupsLock = InsightWorker::taskGroupsInExecution.acquireLock();
                auto& taskGroupsInExecution = InsightWorker::taskGroupsInExecution.getValue();

                const auto canExecuteTask = [&taskGroupsInExecution] (const QSharedPointer<InsightWorkerTask>& task) {
                    for (const auto taskGroup : task->taskGroups()) {
                        if (taskGroupsInExecution.contains(taskGroup)) {
                            return false;
                        }
                    }

                    return true;
                };

                for (auto [queuedTaskId, task] : queuedTasks) {
                    if (canExecuteTask(task)) {
                        const auto taskGroups = task->taskGroups();
                        taskGroupsInExecution.insert(taskGroups.begin(), taskGroups.end());
                        queuedTasks.erase(queuedTaskId);
                        return task;
                    }
                }
            }

            return std::nullopt;
        };

        auto queuedTask = std::optional<QSharedPointer<InsightWorkerTask>>();

        while ((queuedTask = getQueuedTask())) {
            auto& task = *queuedTask;
            task->moveToThread(this->thread());
            task->execute(this->targetControllerService);

            {
                const auto taskGroupsLock = InsightWorker::taskGroupsInExecution.acquireLock();
                auto& taskGroupsInExecution = InsightWorker::taskGroupsInExecution.getValue();

                for (const auto& taskGroup : task->taskGroups()) {
                    taskGroupsInExecution.erase(taskGroup);
                }
            }

            emit InsightSignals::instance()->taskProcessed(task);
        }
    }
}
