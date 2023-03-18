#include "InsightWorkerTask.hpp"

#include "src/Logger/Logger.hpp"

namespace Bloom
{
    using Services::TargetControllerService;

    InsightWorkerTask::InsightWorkerTask()
        : QObject(nullptr)
    {}

    void InsightWorkerTask::execute(TargetControllerService& targetControllerService) {
        try {
            this->state = InsightWorkerTaskState::STARTED;
            emit this->started();

            this->run(targetControllerService);

            this->state = InsightWorkerTaskState::COMPLETED;
            this->setProgressPercentage(100);
            emit this->completed();

        } catch (std::exception& exception) {
            this->state = InsightWorkerTaskState::FAILED;
            Logger::debug("InsightWorker task failed - " + std::string(exception.what()));
            emit this->failed(QString::fromStdString(exception.what()));
        }

        emit this->finished();
    }

    void InsightWorkerTask::setProgressPercentage(std::uint8_t percentage) {
        this->progressPercentage = percentage;
        emit this->progressUpdate(this->progressPercentage);
    }
}
