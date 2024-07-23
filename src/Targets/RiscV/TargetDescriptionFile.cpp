#include "TargetDescriptionFile.hpp"

namespace Targets::RiscV
{
    TargetDescriptionFile::TargetDescriptionFile(const std::string& xmlFilePath)
        : Targets::TargetDescription::TargetDescriptionFile(xmlFilePath)
    {}

    std::string TargetDescriptionFile::getTargetId() const {
        return this->getProperty("vendor", "target_id").value;
    }

    TargetAddressSpaceDescriptor TargetDescriptionFile::getSystemAddressSpaceDescriptor() const {
        return this->targetAddressSpaceDescriptorFromAddressSpace(this->getAddressSpace("system"));
    }
}
