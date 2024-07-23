#pragma once

#include <cstdint>
#include <atomic>
#include <optional>

#include "TargetMemory.hpp"

namespace Targets
{
    enum class TargetExecutionState: std::uint8_t
    {
        UNKNOWN,
        STOPPED,
        RUNNING,
        STEPPING,
    };

    enum class TargetMode: std::uint8_t
    {
        DEBUGGING,
        PROGRAMMING,
    };

    static_assert(std::atomic<TargetExecutionState>::is_always_lock_free);
    static_assert(std::atomic<TargetMode>::is_always_lock_free);
    static_assert(std::atomic<std::optional<TargetMemoryAddress>>::is_always_lock_free);

    struct TargetState
    {
        std::atomic<TargetExecutionState> executionState = TargetExecutionState::UNKNOWN;
        std::atomic<TargetMode> mode = TargetMode::DEBUGGING;

        /**
         * Current program counter - only populated when TargetState::executionState == TargetExecutionState::STOPPED
         */
        std::atomic<std::optional<TargetMemoryAddress>> programCounter = {};

        TargetState() = default;

        TargetState(
            TargetExecutionState executionState,
            TargetMode mode,
            std::optional<TargetMemoryAddress> programCounter = std::nullopt
        )
            : executionState(executionState)
            , mode(mode)
            , programCounter(programCounter)
        {}

        TargetState(const TargetState& other)
            : executionState(other.executionState.load())
            , mode(other.mode.load())
            , programCounter(other.programCounter.load())
        {}

        TargetState& operator = (const TargetState& other) {
            this->executionState = other.executionState.load();
            this->mode = other.mode.load();
            this->programCounter = other.programCounter.load();

            return *this;
        }

        bool operator == (const TargetState& other) const {
            return
                this->executionState.load() == other.executionState.load()
                && this->mode.load() == other.mode.load()
                && this->programCounter.load() == other.programCounter.load()
            ;
        }

        bool operator != (const TargetState& other) const {
            return !(*this == other);
        }
    };
}
