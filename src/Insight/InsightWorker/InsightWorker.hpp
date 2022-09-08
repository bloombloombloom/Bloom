#pragma once

#include <cstdint>
#include <atomic>
#include <QtCore>
#include <queue>
#include <map>

#include "Tasks/InsightWorkerTask.hpp"

#include "src/Helpers/SyncSafe.hpp"
#include "src/TargetController/TargetControllerConsole.hpp"

namespace Bloom
{
    static_assert(std::atomic<std::uint8_t>::is_always_lock_free);

    /**
     * The InsightWorker runs on a separate thread to the main GUI thread. Its purpose is to handle any
     * blocking/time-expensive operations.
     */
    class InsightWorker: public QObject
    {
        Q_OBJECT

    public:
        const std::uint8_t id = ++(InsightWorker::lastWorkerId);

        InsightWorker() = default;
        void startup();
        static void queueTask(InsightWorkerTask* task);

    signals:
        void ready();

    private:
        using QueuedTaskId = std::uint64_t;
        static_assert(std::atomic<QueuedTaskId>::is_always_lock_free);

        static inline std::atomic<std::uint8_t> lastWorkerId = 0;
        static inline SyncSafe<std::map<QueuedTaskId, InsightWorkerTask*>> queuedTasksById = {};
        static inline SyncSafe<TaskGroups> taskGroupsInExecution = {};

        TargetController::TargetControllerConsole targetControllerConsole = TargetController::TargetControllerConsole();

        void executeTasks();
    };
}
