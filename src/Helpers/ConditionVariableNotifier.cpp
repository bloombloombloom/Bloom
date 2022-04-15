#include "ConditionVariableNotifier.hpp"

namespace Bloom
{
    void ConditionVariableNotifier::notify() {
        auto lock = std::unique_lock(this->mutex);
        this->notified = true;
        this->conditionalVariable.notify_all();
    }

    void ConditionVariableNotifier::waitForNotification() {
        auto lock = std::unique_lock(this->mutex);
        this->conditionalVariable.wait(lock, [this] {
            return this->notified;
        });

        this->notified = false;
    }
}
