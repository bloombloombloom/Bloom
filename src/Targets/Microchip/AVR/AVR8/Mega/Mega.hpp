#pragma once

#include "src/Targets/Microchip/AVR/AVR8/Avr8.hpp"

namespace Bloom::Targets::Microchip::Avr::Avr8Bit
{
    class Mega: public Avr8
    {
    protected:

    public:
        Mega(const Avr8& avr8) : Avr8(avr8) {};

        virtual bool supportsPromotion() override {
            return false;
        }
    };
}
