#include "ConditionVariableNotifier.hpp"

void ConditionVariableNotifier::notify() {
    const auto lock = std::unique_lock{this->mutex};
    this->notified = true;
    this->conditionalVariable.notify_all();
}

void ConditionVariableNotifier::waitForNotification(std::optional<std::chrono::milliseconds> timeout) {
    const auto predicate = [this] {
        return this->notified;
    };
    auto lock = std::unique_lock{this->mutex};

    if (timeout.has_value()) {
        this->conditionalVariable.wait_for(lock, timeout.value(), predicate);

    } else {
        this->conditionalVariable.wait(lock, predicate);
    }

    this->notified = false;
}
