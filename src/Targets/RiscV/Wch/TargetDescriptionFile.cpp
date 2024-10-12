#include "TargetDescriptionFile.hpp"

namespace Targets::RiscV::Wch
{
    TargetDescriptionFile::TargetDescriptionFile(const std::string& xmlFilePath)
        : Targets::RiscV::TargetDescriptionFile(xmlFilePath)
    {}

    std::map<
        std::string,
        const Targets::TargetDescription::Variant*
    > TargetDescriptionFile::getVariantsByWchVariantId() {
        auto output = std::map<std::string, const Targets::TargetDescription::Variant*>{};

        for (const auto& [variantKey, variant] : this->variantsByKey) {
            output.emplace(variant.getProperty("vendor", "variant_id").value, &variant);
        }

        return output;
    }
}
