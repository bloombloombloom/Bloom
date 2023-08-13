#pragma once

#include <cstdint>
#include <atomic>
#include <QObject>
#include <QString>

#include "TaskGroup.hpp"
#include "src/Services/TargetControllerService.hpp"

enum class InsightWorkerTaskState: std::uint8_t
{
    CREATED,
    STARTED,
    FAILED,
    COMPLETED,
};

static_assert(std::atomic<InsightWorkerTaskState>::is_always_lock_free);
static_assert(std::atomic<std::uint8_t>::is_always_lock_free);

class InsightWorkerTask: public QObject
{
    Q_OBJECT

public:
    using IdType = std::uint64_t;
    const InsightWorkerTask::IdType id = ++(InsightWorkerTask::lastId);
    std::atomic<InsightWorkerTaskState> state = InsightWorkerTaskState::CREATED;
    std::atomic<std::uint8_t> progressPercentage = 0;

    InsightWorkerTask();

    virtual QString brief() const = 0;

    virtual TaskGroups taskGroups() const {
        return TaskGroups();
    };

    void execute(Services::TargetControllerService& targetControllerService);

signals:
    /**
     * The InsightWorkerTask::started() signal will be emitted once the task has started (InsightWorker::run() is
     * called)
     */
    void started();

    /**
     * Some tasks will emit an InsightWorkerTask::progressUpdate() signal to provide an update on their progress.
     *
     * This is used for progress bar widgets.
     *
     * NOTE: A task doesn't have to emit this signal. Currently, the time-expensive tasks (like ReadTargetMemory)
     * emit this signal.
     *
     * @param progressPercentage
     *  The task's current progress.
     */
    void progressUpdate(std::uint8_t progressPercentage);

    /**
     * The InsightWorkerTask::completed() signal will be emitted once the task has successfully completed.
     */
    void completed();

    /**
     * The InsightWorkerTask::failed() signal will be emitted when the task fails (InsightWorkerTask::run() throws
     * an exception).
     */
    void failed(QString errorMessage);

    /**
     * The InsightWorkerTask::finished() signal will be emitted at the end of the task, regardless to whether it
     * completed successfully or failed.
     */
    void finished();

protected:
    virtual void run(Services::TargetControllerService& targetControllerService) = 0;
    void setProgressPercentage(std::uint8_t percentage);

private:
    static inline std::atomic<InsightWorkerTask::IdType> lastId = 0;
};
