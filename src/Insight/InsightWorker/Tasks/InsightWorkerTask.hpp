#pragma once

#include <QObject>
#include <QString>

#include "TaskGroup.hpp"
#include "src/TargetController/TargetControllerConsole.hpp"

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
        InsightWorkerTaskState state = InsightWorkerTaskState::CREATED;

        InsightWorkerTask(): QObject(nullptr) {};

        virtual TaskGroups getTaskGroups() const {
            return TaskGroups();
        };

        void execute(TargetController::TargetControllerConsole& targetControllerConsole);

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
        virtual void run(TargetController::TargetControllerConsole& targetControllerConsole) = 0;
    };
}
