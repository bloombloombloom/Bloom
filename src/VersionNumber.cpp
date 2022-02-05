#include "VersionNumber.hpp"

#include <QString>
#include <QStringList>

namespace Bloom
{
    VersionNumber::VersionNumber(std::uint16_t major, std::uint16_t minor, std::uint16_t patch)
    : major{major}, minor{minor}, patch{patch} {
        this->combined = static_cast<std::uint32_t>(
            std::stoul(std::to_string(this->major) + std::to_string(this->minor) + std::to_string(this->patch))
        );
    }

    VersionNumber::VersionNumber(const std::string& versionNumber) {
        auto versionNumberQStr = QString::fromStdString(versionNumber);
        const auto explodedString = versionNumberQStr.split('.');

        this->major = explodedString.value(0, "0").toUShort();
        this->minor = explodedString.value(1, "0").toUShort();
        this->patch = explodedString.value(2, "0").toUShort();

        this->combined = versionNumberQStr.remove('.').toUInt();
    }

    std::string VersionNumber::toString() const {
        return std::to_string(this->major) + "." + std::to_string(this->minor) + "." + std::to_string(this->patch);
    }
}
