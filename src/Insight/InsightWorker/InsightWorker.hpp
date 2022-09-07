#pragma once

#include <QtCore>
#include <queue>

#include "Tasks/InsightWorkerTask.hpp"

#include "src/Helpers/SyncSafe.hpp"
#include "src/TargetController/TargetControllerConsole.hpp"

namespace Bloom
{
    /**
     * The InsightWorker runs on a separate thread to the main GUI thread. Its purpose is to handle any
     * blocking/time-expensive operations.
     */
    class InsightWorker: public QObject
    {
        Q_OBJECT

    public:
        InsightWorker() = default;

        void startup();
        static void queueTask(InsightWorkerTask* task);

    signals:
        void ready();

    private:
        TargetController::TargetControllerConsole targetControllerConsole = TargetController::TargetControllerConsole();

        static inline SyncSafe<std::queue<InsightWorkerTask*>> queuedTasks = {};

        static std::optional<InsightWorkerTask*> getQueuedTask();
        void executeTasks();
    };
}
