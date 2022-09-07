#include "InsightWorker.hpp"

#include <QObject>

#include "src/Insight/InsightSignals.hpp"
#include "src/Logger/Logger.hpp"

namespace Bloom
{
    using namespace Bloom::Exceptions;

    using Bloom::Targets::TargetState;

    void InsightWorker::startup() {
        Logger::debug("Starting InsightWorker thread");

        QObject::connect(
            InsightSignals::instance(),
            &InsightSignals::taskQueued,
            this,
            &InsightWorker::executeTasks
        );

        emit this->ready();
    }

    void InsightWorker::queueTask(InsightWorkerTask* task) {
        task->moveToThread(nullptr);

        {
            const auto taskQueueLock = InsightWorker::queuedTasks.acquireLock();
            InsightWorker::queuedTasks.getValue().push(task);
        }

        emit InsightSignals::instance()->taskQueued();
    }

    std::optional<InsightWorkerTask*> InsightWorker::getQueuedTask() {
        auto taskQueueLock = InsightWorker::queuedTasks.acquireLock();
        auto& queuedTasks = InsightWorker::queuedTasks.getValue();

        if (!queuedTasks.empty()) {
            auto* task = queuedTasks.front();
            queuedTasks.pop();
            return task;
        }

        return std::nullopt;
    }

    void InsightWorker::executeTasks() {
        auto queuedTask = std::optional<InsightWorkerTask*>();

        while ((queuedTask = InsightWorker::getQueuedTask()).has_value()) {
            auto* task = queuedTask.value();
            task->moveToThread(this->thread());
            task->setParent(this);
            task->execute(this->targetControllerConsole);
            task->deleteLater();
        }
    }
}
