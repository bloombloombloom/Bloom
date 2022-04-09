#pragma once

#include <QObject>
#include <QString>

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

        void execute(TargetController::TargetControllerConsole& targetControllerConsole);

    signals:
        void started();
        void failed(QString errorMessage);
        void completed();

    protected:
        virtual void run(TargetController::TargetControllerConsole& targetControllerConsole) = 0;
    };
}
