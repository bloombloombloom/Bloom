#pragma once

#include <set>

#include "Event.hpp"
#include "src/Targets/TargetRegister.hpp"

namespace Bloom::Events
{
    using Bloom::Targets::TargetRegister;

    class WriteRegistersToTarget: public Event
    {
    public:
        static inline const std::string name = "WriteRegistersToTarget";
        TargetRegisters registers;

        std::string getName() const override {
            return WriteRegistersToTarget::name;
        }

        WriteRegistersToTarget() = default;
        WriteRegistersToTarget(const TargetRegisters& registers): registers(registers) {};
    };
}
