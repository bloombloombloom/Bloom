#include "InsightWorkerTask.hpp"

#include "src/Logger/Logger.hpp"

namespace Bloom
{
    using TargetController::TargetControllerConsole;

    void InsightWorkerTask::execute(TargetControllerConsole& targetControllerConsole) {
        try {
            this->state = InsightWorkerTaskState::STARTED;
            emit this->started();
            this->run(targetControllerConsole);
            this->state = InsightWorkerTaskState::COMPLETED;
            emit this->completed();

        } catch (std::exception& exception) {
            this->state = InsightWorkerTaskState::FAILED;
            Logger::error("InsightWorker task failed - " + std::string(exception.what()));
            emit this->failed(QString::fromStdString(exception.what()));
        }

        emit this->finished();
    }
}
