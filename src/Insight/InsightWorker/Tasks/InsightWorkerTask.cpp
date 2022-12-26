#include "InsightWorkerTask.hpp"

#include "src/Logger/Logger.hpp"

namespace Bloom
{
    using Services::TargetControllerService;

    void InsightWorkerTask::execute(TargetControllerService& targetControllerService) {
        try {
            this->state = InsightWorkerTaskState::STARTED;
            emit this->started();
            this->run(targetControllerService);
            this->state = InsightWorkerTaskState::COMPLETED;
            emit this->completed();

        } catch (std::exception& exception) {
            this->state = InsightWorkerTaskState::FAILED;
            Logger::debug("InsightWorker task failed - " + std::string(exception.what()));
            emit this->failed(QString::fromStdString(exception.what()));
        }

        emit this->finished();
    }
}
