#pragma once

#include <cstdint>
#include <set>
#include <array>

#include "src/DebugToolDrivers/Protocols/RiscVDebug/TriggerModule/TriggerModule.hpp"

namespace DebugToolDrivers::Protocols::RiscVDebug::TriggerModule::Registers
{
    struct TriggerInfo
    {
        std::uint16_t info;
        std::uint8_t version;

        static constexpr auto fromValue(RegisterValue value) {
            return TriggerInfo{
                .info = static_cast<std::uint16_t>(value & 0xFFFF),
                .version = static_cast<std::uint8_t>(value >> 24),
            };
        }

        [[nodiscard]] constexpr RegisterValue value() const {
            return RegisterValue{0}
                | static_cast<RegisterValue>(this->info)
                | static_cast<RegisterValue>(this->version) << 24
            ;
        }

        [[nodiscard]] std::set<TriggerType> getSupportedTriggerTypes() const {
            auto output = std::set<TriggerType>{};

            static constexpr auto types = std::to_array<TriggerType>({
                TriggerType::LEGACY,
                TriggerType::MATCH_CONTROL,
                TriggerType::INSTRUCTION_COUNT,
                TriggerType::INTERRUPT_TRIGGER,
                TriggerType::EXCEPTION_TRIGGER,
                TriggerType::MATCH_CONTROL_TYPE_6,
                TriggerType::EXTERNAL,
            });

            for (const auto& type : types) {
                if (this->info & (0x01 << static_cast<std::uint8_t>(type))) {
                    output.insert(type);
                }
            }

            return output;
        }
    };
}
