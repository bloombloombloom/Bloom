#pragma once

#include <cstdint>

#include "HouseKeepingCommandFrame.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::CommandFrames::HouseKeeping
{
    enum class ParameterContext: unsigned char
    {
        CONFIG = 0x00,
        ANALOG = 0x01,
        USB = 0x03,
    };

    struct Parameter
    {
        ParameterContext context;
        unsigned char id;
        std::uint8_t size;

        constexpr Parameter(ParameterContext context, unsigned char id, std::uint8_t size)
            : context(context)
            , id(id)
            , size(size)
        {};
    };

    struct Parameters
    {
        static constexpr Parameter CONFIG_FW_VERSION_MAJOR{ParameterContext::CONFIG, 0x01, 1};
        static constexpr Parameter CONFIG_FW_VERSION_MINOR{ParameterContext::CONFIG, 0x02, 1};
        static constexpr Parameter CONFIG_FW_VERSION_BUILD{ParameterContext::CONFIG, 0x03, 2};
        static constexpr Parameter CONFIG_FW_VERSION_FULL{ParameterContext::CONFIG, 0x01, 4};
        static constexpr Parameter USB_MAX_READ{ParameterContext::USB, 0x00, 2};
        static constexpr Parameter USB_MAX_WRITE{ParameterContext::USB, 0x01, 2};
    };
}
