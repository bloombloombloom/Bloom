#pragma once

#include <cstdint>
#include <optional>

#include "src/DebugToolDrivers/Protocols/RiscVDebugSpec/TriggerModule/TriggerModule.hpp"

namespace DebugToolDrivers::Protocols::RiscVDebugSpec::TriggerModule::Registers
{
    struct TriggerData1
    {
        std::uint32_t data;
        bool debugModeOnly;
        std::uint8_t type;

        static constexpr auto fromValue(RegisterValue value) {
            return TriggerData1{
                .data = value & 0x07FFFFFF,
                .debugModeOnly = static_cast<bool>(value & (0x01 << 27)),
                .type = static_cast<std::uint8_t>((value >> 28) & 0x0F),
            };
        }

        [[nodiscard]] constexpr RegisterValue value() const {
            return RegisterValue{0}
                | static_cast<RegisterValue>(this->data & 0x07FFFFFF)
                | static_cast<RegisterValue>(this->debugModeOnly) << 27
                | static_cast<RegisterValue>(this->type & 0x0F) << 28
            ;
        }

        [[nodiscard]] std::optional<TriggerType> getType() const {
            return (this->type >= 0x01 && this->type <= 0x07)
                ? std::optional{static_cast<TriggerType>(this->type)}
                : std::nullopt;
        }
    };
}
