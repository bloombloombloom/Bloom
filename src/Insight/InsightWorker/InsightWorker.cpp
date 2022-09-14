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

        QObject::connect(insightSignals, &InsightSignals::taskQueued, this, &InsightWorker::executeTasks);
        QObject::connect(insightSignals, &InsightSignals::taskProcessed, this, &InsightWorker::executeTasks);

        Logger::debug("InsightWorker" + std::to_string(this->id) + " ready");
        emit this->ready();
    }

    void InsightWorker::queueTask(InsightWorkerTask* task) {
        static std::atomic<QueuedTaskId> lastQueuedTaskId = 0;
        task->moveToThread(nullptr);

        {
            const auto taskQueueLock = InsightWorker::queuedTasksById.acquireLock();
            InsightWorker::queuedTasksById.getValue().insert(std::pair(++(lastQueuedTaskId), task));
        }

        emit InsightSignals::instance()->taskQueued();
    }

    void InsightWorker::executeTasks() {
        static const auto getQueuedTask = [] () -> std::optional<InsightWorkerTask*> {
            const auto taskQueueLock = InsightWorker::queuedTasksById.acquireLock();
            auto& queuedTasks = InsightWorker::queuedTasksById.getValue();

            if (!queuedTasks.empty()) {
                const auto taskGroupsLock = InsightWorker::taskGroupsInExecution.acquireLock();
                auto& taskGroupsInExecution = InsightWorker::taskGroupsInExecution.getValue();

                const auto canExecuteTask = [&taskGroupsInExecution] (InsightWorkerTask* task) {
                    for (const auto taskGroup : task->getTaskGroups()) {
                        if (taskGroupsInExecution.contains(taskGroup)) {
                            return false;
                        }
                    }

                    return true;
                };

                for (auto [queuedTaskId, task] : queuedTasks) {
                    if (canExecuteTask(task)) {
                        const auto taskGroups = task->getTaskGroups();
                        taskGroupsInExecution.insert(taskGroups.begin(), taskGroups.end());
                        queuedTasks.erase(queuedTaskId);
                        return task;
                    }
                }
            }

            return std::nullopt;
        };

        auto queuedTask = std::optional<InsightWorkerTask*>();

        while ((queuedTask = getQueuedTask())) {
            auto* task = queuedTask.value();
            task->moveToThread(this->thread());
            task->setParent(this);
            task->execute(this->targetControllerConsole);

            {
                const auto taskGroupsLock = InsightWorker::taskGroupsInExecution.acquireLock();
                auto& taskGroupsInExecution = InsightWorker::taskGroupsInExecution.getValue();

                for (const auto& taskGroup : task->getTaskGroups()) {
                    taskGroupsInExecution.erase(taskGroup);
                }
            }

            task->deleteLater();
            emit InsightSignals::instance()->taskProcessed();
        }
    }
}
