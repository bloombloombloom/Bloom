#pragma once

#include <mutex>
#include <condition_variable>
#include <chrono>
#include <optional>

#include "NotifierInterface.hpp"

/**
 * The ConditionVariableNotifier class is an implementation of the NotifierInterface, using an
 * std::condition_variable.
 */
class ConditionVariableNotifier: public NotifierInterface
{
public:
    ConditionVariableNotifier() = default;
    ~ConditionVariableNotifier() override = default;

    ConditionVariableNotifier(ConditionVariableNotifier& other) = delete;
    ConditionVariableNotifier& operator = (ConditionVariableNotifier& other) = delete;

    ConditionVariableNotifier(ConditionVariableNotifier&& other) noexcept = delete;
    ConditionVariableNotifier& operator = (ConditionVariableNotifier&& other) = delete;

    void notify() override;

    /**
     * Blocks until the contained std::conditional_variable is notified.
     */
    void waitForNotification(std::optional<std::chrono::milliseconds> timeout = std::nullopt);

private:
    std::mutex mutex;
    std::condition_variable conditionalVariable;
    bool notified = false;
};
