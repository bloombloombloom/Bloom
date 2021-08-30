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
    protected:
        virtual void run(TargetControllerConsole& targetControllerConsole) = 0;

    public:
        InsightWorkerTaskState state;

        InsightWorkerTask() = default;
        InsightWorkerTask(QObject* parent): QObject(parent) {};

        void execute(TargetControllerConsole& targetControllerConsole);

    signals:
        void started();
        void failed(QString errorMessage);
        void completed();
    };
}
