#pragma once

#include <cstdint>

#include "src/Targets/TargetRegisterDescriptor.hpp"

namespace Targets::Microchip::Avr8
{
    /**
     * This struct contains all of the relevant GPIO register descriptors for a particular pad, on an AVR8 target.
     *
     * We use this to read and manipulate the state of GPIO pins.
     */
    struct GpioPadDescriptor
    {
        std::uint8_t registerMask;

        const TargetRegisterDescriptor& dataDirectionRegisterDescriptor;
        const TargetRegisterDescriptor& inputRegisterDescriptor;
        const TargetRegisterDescriptor& outputRegisterDescriptor;

        GpioPadDescriptor(
            std::uint8_t registerMask,
            const TargetRegisterDescriptor& dataDirectionRegisterDescriptor,
            const TargetRegisterDescriptor& inputRegisterDescriptor,
            const TargetRegisterDescriptor& outputRegisterDescriptor
        )
            : registerMask(registerMask)
            , dataDirectionRegisterDescriptor(dataDirectionRegisterDescriptor)
            , inputRegisterDescriptor(inputRegisterDescriptor)
            , outputRegisterDescriptor(outputRegisterDescriptor)
        {}
    };
}
