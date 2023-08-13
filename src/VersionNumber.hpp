#pragma once

#include <cstdint>
#include <string>
#include <QString>

class VersionNumber
{
public:
    std::uint16_t major = 0;
    std::uint16_t minor = 0;
    std::uint16_t patch = 0;

    VersionNumber(std::uint16_t major, std::uint16_t minor, std::uint16_t patch);
    VersionNumber(const std::string& versionNumber);
    VersionNumber(const QString& versionNumber);

    std::string toString() const;

    bool operator == (const VersionNumber& other) const {
        return
            this->major == other.major
            && this->minor == other.minor
            && this->patch == other.patch;
    }

    bool operator != (const VersionNumber& other) const {
        return !(*this == other);
    }

    bool operator < (const VersionNumber& other) const {
        return
            this->major < other.major
            || (this->major == other.major && this->minor < other.minor)
            || (this->major == other.major && this->minor == other.minor && this->patch < other.patch);
    }

    bool operator > (const VersionNumber& other) const {
        return other < *this;
    }

    bool operator <= (const VersionNumber& other) const {
        return !(other < *this);
    }

    bool operator >= (const VersionNumber& other) const {
        return !(*this < other);
    }
};
