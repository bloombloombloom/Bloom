#pragma once

#include <cstdint>
#include <optional>

#include "src/DebugToolDrivers/Protocols/RiscVDebugSpec/TriggerModule/TriggerModule.hpp"

namespace DebugToolDrivers::Protocols::RiscVDebugSpec::TriggerModule::Registers
{
    struct MatchControl
    {
        enum class MatchMode: std::uint8_t
        {
            EQUAL = 0x00,
            TOP_BITS = 0x01,
            GREATER_THAN = 0x02,
            LESS_THAN = 0x03,
            MASK_LOW = 0x04,
            MASK_HIGH = 0x05,
            NOT_EQUAL = 0x08,
            NOT_TOP_BITS = 0x09,
            NOT_MASK_LOW = 0x0C,
            NOT_MASK_HIGH = 0x0D,
        };

        enum class AccessSize: std::uint8_t
        {
            ANY = 0x00,
            SIZE_8 = 0x01,
            SIZE_16 = 0x02,
            SIZE_32 = 0x03,
        };

        enum class CompareValueType: std::uint8_t
        {
            ADDRESS = 0x00,
            DATA = 0x01,
        };

        bool load:1 = false;
        bool store:1 = false;
        bool execute:1 = false;
        bool enabledInUserMode:1 = false;
        bool enabledInSupervisorMode:1 = false;
        bool enabledInMachineMode:1 = false;
        MatchMode matchMode:4 = MatchMode::EQUAL;
        bool chain:1 = false; // TODO: Consider making this an enum
        TriggerAction action:4 = TriggerAction::ENTER_DEBUG_MODE;
        AccessSize accessSize = AccessSize::ANY;
        bool timing:1 = false; // TODO: Consider making this an enum
        CompareValueType compareValueType:1 = CompareValueType::ADDRESS;
        bool hit:1 = false;

        MatchControl() = default;

        constexpr explicit MatchControl(
            bool load,
            bool store,
            bool execute,
            bool enabledInUserMode,
            bool enabledInSupervisorMode,
            bool enabledInMachineMode,
            MatchMode matchMode,
            bool chain,
            TriggerAction action,
            AccessSize accessSize,
            bool timing,
            CompareValueType compareValueType,
            bool hit
        )
            : load(load)
            , store(store)
            , execute(execute)
            , enabledInUserMode(enabledInUserMode)
            , enabledInSupervisorMode(enabledInSupervisorMode)
            , enabledInMachineMode(enabledInMachineMode)
            , matchMode(matchMode)
            , chain(chain)
            , action(action)
            , accessSize(accessSize)
            , timing(timing)
            , compareValueType(compareValueType)
            , hit(hit)
        {}

        constexpr explicit MatchControl(RegisterValue registerValue)
            : load(static_cast<bool>(registerValue & 0x01))
            , store(static_cast<bool>(registerValue & (0x01 << 1)))
            , execute(static_cast<bool>(registerValue & (0x01 << 2)))
            , enabledInUserMode(static_cast<bool>(registerValue & (0x01 << 3)))
            , enabledInSupervisorMode(static_cast<bool>(registerValue & (0x01 << 4)))
            , enabledInMachineMode(static_cast<bool>(registerValue & (0x01 << 6)))
            , matchMode(static_cast<MatchMode>((registerValue >> 7) & 0x0F))
            , chain(static_cast<bool>(registerValue & (0x01 << 11)))
            , action(static_cast<TriggerAction>((registerValue >> 12) & 0x0F))
            , accessSize(
                static_cast<AccessSize>(
                    (((registerValue >> 21) & 0x03) << 2) | ((registerValue >> 16) & 0x03)
                )
            )
            , timing(static_cast<bool>(registerValue & (0x01 << 18)))
            , compareValueType(static_cast<CompareValueType>((registerValue >> 19) & 0x01))
            , hit(static_cast<bool>(registerValue & (0x01 << 20)))
        {}

        [[nodiscard]] constexpr RegisterValue value() const {
            return RegisterValue{0}
                | static_cast<RegisterValue>(this->load)
                | static_cast<RegisterValue>(this->store) << 1
                | static_cast<RegisterValue>(this->execute) << 2
                | static_cast<RegisterValue>(this->enabledInUserMode) << 3
                | static_cast<RegisterValue>(this->enabledInSupervisorMode) << 4
                | static_cast<RegisterValue>(this->enabledInMachineMode) << 6
                | static_cast<RegisterValue>(this->matchMode) << 7
                | static_cast<RegisterValue>(this->chain) << 11
                | static_cast<RegisterValue>(this->action) << 12
                | (static_cast<RegisterValue>(this->accessSize) & 0x03) << 16
                | static_cast<RegisterValue>(this->timing) << 18
                | static_cast<RegisterValue>(this->compareValueType) << 19
                | static_cast<RegisterValue>(this->hit) << 20
                | (static_cast<RegisterValue>(this->accessSize) >> 2) << 21
            ;
        }
    };
}
