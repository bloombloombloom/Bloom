#include "TargetDescriptionFile.hpp"

namespace Targets::RiscV
{
    TargetDescriptionFile::TargetDescriptionFile(const std::string& xmlFilePath)
        : Targets::TargetDescription::TargetDescriptionFile(xmlFilePath)
    {}

    const TargetDescription::AddressSpace& TargetDescriptionFile::getSystemAddressSpace() const {
        return this->getAddressSpace("system");
    }

    TargetAddressSpaceDescriptor TargetDescriptionFile::getSystemAddressSpaceDescriptor() const {
        return this->targetAddressSpaceDescriptorFromAddressSpace(this->getSystemAddressSpace());
    }

    IsaDescriptor TargetDescriptionFile::getIsaDescriptor() const {
        return IsaDescriptor{this->getDeviceAttribute("architecture")};
    }
}
