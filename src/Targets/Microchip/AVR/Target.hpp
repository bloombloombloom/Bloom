#pragma once

#include <src/Logger/Logger.hpp>
#include <sstream>
#include <iomanip>

#include "../../Target.hpp"
#include "TargetSignature.hpp"

namespace Bloom::Targets::Microchip::Avr
{
    class Target: public ::Bloom::Targets::Target
    {
    protected:
        std::optional<TargetSignature> id;

        virtual void setId(unsigned char byteZero, unsigned char byteOne, unsigned char byteTwo) {
            if (!this->id.has_value()) {
                this->id = TargetSignature();
            }

            this->id->byteZero = byteZero;
            this->id->byteOne = byteOne;
            this->id->byteTwo = byteTwo;
        }

        virtual void setId(const TargetSignature& id) {
            this->id = id;
        }

        virtual TargetSignature getId() = 0;

    public:
        explicit Target() = default;

        std::string getHumanReadableId() override {
            return this->getId().toHex();
        }
    };
}