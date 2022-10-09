#pragma once

#include <iomanip>
#include <optional>

#include "src/Targets/Target.hpp"

#include "TargetSignature.hpp"

namespace Bloom::Targets::Microchip::Avr
{
    class Target: public ::Bloom::Targets::Target
    {
    public:
        explicit Target() = default;

        std::string getHumanReadableId() override {
            return this->getId().toHex();
        }

    protected:
        std::optional<TargetSignature> id;

        virtual void setId(const TargetSignature& id) {
            this->id = id;
        }

        virtual TargetSignature getId() = 0;
    };
}
