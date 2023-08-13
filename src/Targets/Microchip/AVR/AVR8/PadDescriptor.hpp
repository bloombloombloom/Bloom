#pragma once

#include <string>
#include <cstdint>
#include <optional>

namespace Targets::Microchip::Avr::Avr8Bit
{
    /**
     * Pin configurations for AVR8 targets may vary across target variants. This is why we must differentiate a pin
     * to a pad. A pin is mapped to a pad, but this mapping is variant specific. For example, pin 4 on
     * the ATmega328P-PN (DIP variant) is mapped to a GPIO pad (PORTD/PIN2), but on the QFN variant (ATmega328P-MN),
     * pin 4 is mapped to a GND pad.
     *
     * PadDescriptor describes a single pad on an AVR8 target. On target configuration, PadDescriptors are
     * generated from the AVR8 target description file. These descriptors are mapped to pad names.
     * See Avr8::loadPadDescriptors() for more.
     */
    struct PadDescriptor
    {
        std::string name;

        std::optional<std::uint8_t> gpioPinNumber;
        std::optional<std::uint16_t> gpioPortAddress;
        std::optional<std::uint16_t> gpioPortInputAddress;
        std::optional<std::uint16_t> gpioDdrAddress;
    };
}
