#include "TargetDescriptionFile.hpp"

namespace Targets::RiscV
{
    TargetDescriptionFile::TargetDescriptionFile(const std::string& xmlFilePath)
        : Targets::TargetDescription::TargetDescriptionFile(xmlFilePath)
    {}

    TargetAddressSpaceDescriptor TargetDescriptionFile::getSystemAddressSpaceDescriptor() const {
        return this->targetAddressSpaceDescriptorFromAddressSpace(this->getAddressSpace("system"));
    }
}
