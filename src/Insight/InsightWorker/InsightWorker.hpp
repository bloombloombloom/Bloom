#pragma once

#include <cstdint>
#include <atomic>
#include <QtCore>
#include <queue>
#include <map>
#include <QSharedPointer>

#include "Tasks/InsightWorkerTask.hpp"

#include "src/Helpers/Synchronised.hpp"
#include "src/Services/TargetControllerService.hpp"

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
    static void queueTask(const QSharedPointer<InsightWorkerTask>& task);

signals:
    void ready();

private:
    static inline std::atomic<std::uint8_t> lastWorkerId = 0;
    static inline Synchronised<std::map<InsightWorkerTask::IdType, QSharedPointer<InsightWorkerTask>>> queuedTasksById = {};
    static inline Synchronised<TaskGroups> taskGroupsInExecution = {};

    Services::TargetControllerService targetControllerService = Services::TargetControllerService();

    void executeTasks();
};
