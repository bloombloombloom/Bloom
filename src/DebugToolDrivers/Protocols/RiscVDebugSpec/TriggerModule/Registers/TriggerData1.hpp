#pragma once

#include <cstdint>
#include <optional>

#include "src/DebugToolDrivers/Protocols/RiscVDebugSpec/TriggerModule/TriggerModule.hpp"

namespace DebugToolDrivers::Protocols::RiscVDebugSpec::TriggerModule::Registers
{
    struct TriggerData1
    {
        std::uint32_t data:27;
        bool debugModeOnly:1;
        std::uint8_t type:4;

        TriggerData1() = default;

        constexpr explicit TriggerData1(RegisterValue registerValue)
            : data(registerValue & 0x07FFFFFF)
            , debugModeOnly(static_cast<bool>(registerValue & (0x01 << 27)))
            , type(static_cast<std::uint8_t>(registerValue >> 28) & 0x0F)
        {}

        [[nodiscard]] constexpr RegisterValue value() const {
            return RegisterValue{0}
                | static_cast<RegisterValue>(this->data)
                | static_cast<RegisterValue>(this->debugModeOnly) << 27
                | static_cast<RegisterValue>(this->type) << 28
            ;
        }

        std::optional<TriggerType> getType() const {
            return (this->type >= 0x01 && this->type <= 0x07)
                ? std::optional{static_cast<TriggerType>(this->type)}
                : std::nullopt;
        }
    };
}
