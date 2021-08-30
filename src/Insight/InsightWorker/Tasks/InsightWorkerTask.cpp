#include "InsightWorkerTask.hpp"

using namespace Bloom;

void InsightWorkerTask::execute(TargetControllerConsole& targetControllerConsole) {
    try {
        this->state = InsightWorkerTaskState::STARTED;
        emit this->started();
        this->run(targetControllerConsole);
        this->state = InsightWorkerTaskState::COMPLETED;
        emit this->completed();

    } catch (std::exception& exception) {
        this->state = InsightWorkerTaskState::FAILED;
        emit this->failed(QString::fromStdString(exception.what()));
    }
}
