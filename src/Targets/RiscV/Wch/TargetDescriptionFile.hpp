#pragma once

#include <map>

#include "src/Targets/RiscV/TargetDescriptionFile.hpp"
#include "src/Targets/TargetDescription/Variant.hpp"

namespace Targets::RiscV::Wch
{
    class TargetDescriptionFile: public Targets::RiscV::TargetDescriptionFile
    {
    public:
        explicit TargetDescriptionFile(const std::string& xmlFilePath);

        std::map<std::string, const Targets::TargetDescription::Variant*> getVariantsByWchVariantId();
    };
}
