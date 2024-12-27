#include "TargetDescriptionFile.hpp"

namespace Targets::RiscV
{
    TargetDescriptionFile::TargetDescriptionFile(const std::string& xmlFilePath)
        : Targets::TargetDescription::TargetDescriptionFile(xmlFilePath)
    {}

    const TargetDescription::AddressSpace& TargetDescriptionFile::getCsrAddressSpace() const {
        return this->getAddressSpace("csr");
    }

    const TargetDescription::AddressSpace& TargetDescriptionFile::getGprAddressSpace() const {
        return this->getAddressSpace("gpr");
    }

    const TargetDescription::AddressSpace& TargetDescriptionFile::getSystemAddressSpace() const {
        return this->getAddressSpace("system");
    }

    TargetAddressSpaceDescriptor TargetDescriptionFile::getCsrAddressSpaceDescriptor() const {
        return this->targetAddressSpaceDescriptorFromAddressSpace(this->getCsrAddressSpace());
    }

    TargetAddressSpaceDescriptor TargetDescriptionFile::getGprAddressSpaceDescriptor() const {
        return this->targetAddressSpaceDescriptorFromAddressSpace(this->getGprAddressSpace());
    }

    TargetAddressSpaceDescriptor TargetDescriptionFile::getSystemAddressSpaceDescriptor() const {
        return this->targetAddressSpaceDescriptorFromAddressSpace(this->getSystemAddressSpace());
    }

    IsaDescriptor TargetDescriptionFile::getIsaDescriptor() const {
        return IsaDescriptor{this->getDeviceAttribute("architecture")};
    }
}
