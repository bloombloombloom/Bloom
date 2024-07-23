#pragma once

#include <string>

#include "Event.hpp"

#include "src/Targets/TargetRegisterDescriptor.hpp"

namespace Events
{
    class RegistersWrittenToTarget: public Event
    {
    public:
        static constexpr EventType type = EventType::REGISTERS_WRITTEN_TO_TARGET;
        static const inline std::string name = "RegistersWrittenToTarget";

        Targets::TargetRegisterDescriptorAndValuePairs registers;

        explicit RegistersWrittenToTarget(const Targets::TargetRegisterDescriptorAndValuePairs& registers)
            : registers(registers)
        {}

        [[nodiscard]] EventType getType() const override {
            return RegistersWrittenToTarget::type;
        }

        [[nodiscard]] std::string getName() const override {
            return RegistersWrittenToTarget::name;
        }
    };
}
