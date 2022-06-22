#pragma once

#include <cstdint>
#include <string>
#include <QString>

namespace Bloom
{
    class VersionNumber
    {
    public:
        VersionNumber(std::uint16_t major, std::uint16_t minor, std::uint16_t patch);
        VersionNumber(const std::string& versionNumber);
        VersionNumber(QString versionNumber);

        std::string toString() const;

        [[nodiscard]] std::uint16_t getMajor() const {
            return this->major;
        }

        [[nodiscard]] std::uint16_t getMinor() const {
            return this->minor;
        }

        [[nodiscard]] std::uint16_t getPatch() const {
            return this->patch;
        }

        bool operator == (const VersionNumber& other) const {
            return this->combined == other.combined;
        }

        bool operator != (const VersionNumber& other) const {
            return !(*this == other);
        }

        bool operator < (const VersionNumber& rhs) const {
            return this->combined < rhs.combined;
        }

        bool operator > (const VersionNumber& rhs) const {
            return rhs < *this;
        }

        bool operator <= (const VersionNumber& rhs) const {
            return !(rhs < *this);
        }

        bool operator >= (const VersionNumber& rhs) const {
            return !(*this < rhs);
        }

    private:
        std::uint16_t major = 0;
        std::uint16_t minor = 0;
        std::uint16_t patch = 0;

        /**
         * Integer of the three version segments concatenated (e.g for version 1.5.6, the combined value
         * would be 156).
         */
        std::uint32_t combined = 0;
    };
}
