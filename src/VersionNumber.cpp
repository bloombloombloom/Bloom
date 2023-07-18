#include "VersionNumber.hpp"

#include <QStringList>

namespace Bloom
{
    VersionNumber::VersionNumber(std::uint16_t major, std::uint16_t minor, std::uint16_t patch)
        : major{major}
        , minor{minor}
        , patch{patch}
    {}

    VersionNumber::VersionNumber(const std::string& versionNumber)
        : VersionNumber(QString::fromStdString(versionNumber))
    {}

    VersionNumber::VersionNumber(const QString& versionNumber) {
        const auto explodedString = versionNumber.split('.');

        this->major = explodedString.value(0, "0").toUShort();
        this->minor = explodedString.value(1, "0").toUShort();
        this->patch = explodedString.value(2, "0").toUShort();
    }

    std::string VersionNumber::toString() const {
        return std::to_string(this->major) + "." + std::to_string(this->minor) + "." + std::to_string(this->patch);
    }
}
