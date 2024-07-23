#pragma once

#include <cstdint>

#include "TargetDescriptionFile.hpp"

namespace Targets::Microchip::Avr8
{
    /**
     * These parameters are required by ISP interfaces, to enter programming mode.
     *
     * These parameters are not specific to the EDBG protocol, which is why they do not reside in the EDBG protocol
     * directory.
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

        explicit IspParameters(const TargetDescriptionFile& targetDescriptionFile);
    };
}
