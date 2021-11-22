#pragma once

#include <cstdint>

#include "HouseKeepingCommandFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::HouseKeeping
{
    enum class ParameterContext : unsigned char
    {
        CONFIG = 0x00,
        ANALOG = 0x01,
        USB = 0x03,
    };

    struct Parameter
    {
        ParameterContext context;
        unsigned char id = 0x00;
        std::uint8_t size = 0x00;

        constexpr Parameter(ParameterContext context, unsigned char id, std::uint8_t size)
        : context(context), id(id), size(size) {};
    };

    struct Parameters
    {
        static constexpr Parameter USB_MAX_READ{ParameterContext::USB, 0x00, 2};
        static constexpr Parameter USB_MAX_WRITE{ParameterContext::USB, 0x01, 2};
    };
}
