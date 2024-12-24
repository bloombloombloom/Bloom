#pragma once

#include "src/Targets/TargetDescription/TargetDescriptionFile.hpp"

#include "IsaDescriptor.hpp"

namespace Targets::RiscV
{
    class TargetDescriptionFile: public Targets::TargetDescription::TargetDescriptionFile
    {
    public:
        explicit TargetDescriptionFile(const std::string& xmlFilePath);

        [[nodiscard]] const TargetDescription::AddressSpace& getSystemAddressSpace() const;
        [[nodiscard]] TargetAddressSpaceDescriptor getSystemAddressSpaceDescriptor() const;
        [[nodiscard]] IsaDescriptor getIsaDescriptor() const;
    };
}
