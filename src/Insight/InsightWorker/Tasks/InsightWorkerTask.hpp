#pragma once

#include <cstdint>
#include <atomic>
#include <QObject>
#include <QString>

#include "TaskGroup.hpp"
#include "src/Services/TargetControllerService.hpp"

namespace Bloom
{
    enum class InsightWorkerTaskState: std::uint8_t
    {
        CREATED,
        STARTED,
        FAILED,
        COMPLETED,
    };

    class InsightWorkerTask: public QObject
    {
        Q_OBJECT

    public:
        using IdType = std::uint64_t;
        const InsightWorkerTask::IdType id = ++(InsightWorkerTask::lastId);
        InsightWorkerTaskState state = InsightWorkerTaskState::CREATED;

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

    private:
        static inline std::atomic<InsightWorkerTask::IdType> lastId = 0;
    };
}
