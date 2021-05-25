#pragma once

#include <string>

#include "Event.hpp"
#include "src/Targets/TargetRegister.hpp"

namespace Bloom::Events
{
    class WriteRegistersToTarget: public Event
    {
    public:
        static inline const std::string name = "WriteRegistersToTarget";
        Targets::TargetRegisters registers;

        std::string getName() const override {
            return WriteRegistersToTarget::name;
        }

        WriteRegistersToTarget() = default;
        WriteRegistersToTarget(const Targets::TargetRegisters& registers): registers(registers) {};
    };
}
