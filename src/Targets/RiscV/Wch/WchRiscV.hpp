#pragma once

#include <cstdint>
#include <optional>
#include <functional>

#include "src/Targets/RiscV/RiscV.hpp"

#include "TargetDescriptionFile.hpp"

namespace Targets::RiscV::Wch
{
class WchRiscV: public ::Targets::RiscV::RiscV
    {
    public:
        WchRiscV(const TargetConfig& targetConfig, TargetDescriptionFile&& targetDescriptionFile);

        void activate() override;
        void postActivate() override;
        TargetDescriptor targetDescriptor() override;

    protected:
        TargetDescriptionFile targetDescriptionFile;
        std::optional<std::reference_wrapper<const TargetDescription::Variant>> variant = std::nullopt;
    };
}
