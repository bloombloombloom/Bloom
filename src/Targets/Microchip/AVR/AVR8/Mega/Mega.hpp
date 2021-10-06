#pragma once

#include "src/Targets/Microchip/AVR/AVR8/Avr8.hpp"

namespace Bloom::Targets::Microchip::Avr::Avr8Bit
{
    class Mega: public Avr8
    {
    public:
        explicit Mega(const Avr8& avr8): Avr8(avr8) {};

        bool supportsPromotion() override {
            return false;
        }
    };
}
