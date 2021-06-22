#pragma once

#include <string>
#include <utility>

#include "Event.hpp"
#include "src/Targets/TargetRegister.hpp"

namespace Bloom::Events
{
    class WriteRegistersToTarget: public Event
    {
    public:
        static inline const std::string name = "WriteRegistersToTarget";
        Targets::TargetRegisters registers;

        [[nodiscard]] std::string getName() const override {
            return WriteRegistersToTarget::name;
        }

        WriteRegistersToTarget() = default;
        explicit WriteRegistersToTarget(Targets::TargetRegisters registers): registers(std::move(registers)) {};
    };
}
