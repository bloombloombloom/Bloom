#pragma once

#include <string>
#include <utility>

#include "Event.hpp"
#include "RegistersWrittenToTarget.hpp"
#include "src/Targets/TargetRegister.hpp"

namespace Bloom::Events
{
    class WriteRegistersToTarget: public Event
    {
    public:
        using TargetControllerResponseType = RegistersWrittenToTarget;

        static inline EventType type = EventType::WRITE_REGISTERS_TO_TARGET;
        static inline const std::string name = "WriteRegistersToTarget";
        Targets::TargetRegisters registers;

        WriteRegistersToTarget() = default;
        explicit WriteRegistersToTarget(Targets::TargetRegisters registers): registers(std::move(registers)) {};

        [[nodiscard]] EventType getType() const override {
            return WriteRegistersToTarget::type;
        }

        [[nodiscard]] std::string getName() const override {
            return WriteRegistersToTarget::name;
        }
    };
}
