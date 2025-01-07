#pragma once

#include <cstdint>

#include "src/Targets/TargetPadDescriptor.hpp"
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
        const TargetPadId padId;
        const std::string padKey;
        std::uint8_t registerMask;

        const TargetRegisterDescriptor& dataDirectionRegisterDescriptor;
        const TargetRegisterDescriptor& inputRegisterDescriptor;
        const TargetRegisterDescriptor& outputRegisterDescriptor;

        GpioPadDescriptor(
            const std::string& padKey,
            std::uint8_t registerMask,
            const TargetRegisterDescriptor& dataDirectionRegisterDescriptor,
            const TargetRegisterDescriptor& inputRegisterDescriptor,
            const TargetRegisterDescriptor& outputRegisterDescriptor
        )
            : padId(TargetPadDescriptor::generateId(padKey))
            , padKey(padKey)
            , registerMask(registerMask)
            , dataDirectionRegisterDescriptor(dataDirectionRegisterDescriptor)
            , inputRegisterDescriptor(inputRegisterDescriptor)
            , outputRegisterDescriptor(outputRegisterDescriptor)
        {}
    };
}
