#include "TargetDescriptionFile.hpp"

#include <QString>

namespace Targets::RiscV::TargetDescription
{
    TargetDescriptionFile::TargetDescriptionFile(const std::string& xmlFilePath)
        : Targets::TargetDescription::TargetDescriptionFile(xmlFilePath)
    {}

    std::string TargetDescriptionFile::getTargetId() const {
        return this->deviceAttribute("id");
    }

    std::string TargetDescriptionFile::getVendorName() const {
        return this->deviceAttribute("vendor");
    }
}
