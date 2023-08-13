#pragma once

#include <cstdint>

namespace Targets::Microchip::Avr
{
    /**
     * These parameters are required by the ISP interface. They can be extracted from the target's TDF.
     */
    struct IspParameters
    {
        std::uint8_t programModeTimeout;
        std::uint8_t programModeStabilizationDelay;
        std::uint8_t programModeCommandExecutionDelay;
        std::uint8_t programModeSyncLoops;
        std::uint8_t programModeByteDelay;
        std::uint8_t programModePollValue;
        std::uint8_t programModePollIndex;
        std::uint8_t programModePreDelay;
        std::uint8_t programModePostDelay;

        std::uint8_t readSignaturePollIndex;
        std::uint8_t readFusePollIndex;
        std::uint8_t readLockPollIndex;
    };
}
