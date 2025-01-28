#pragma once

#include <cstdint>
#include <string>

namespace DebugToolDrivers::Wch
{
    class WchFirmwareVersion
    {
    public:
        std::uint8_t major = 0;
        std::uint8_t minor = 0;

        [[nodiscard]] std::string toString() const {
            return std::to_string(this->major) + "." + std::to_string(this->minor);
        }

        bool operator == (const WchFirmwareVersion& other) const {
            return this->major == other.major && this->minor == other.minor;
        }

        bool operator != (const WchFirmwareVersion& other) const {
            return !(*this == other);
        }

        bool operator < (const WchFirmwareVersion& other) const {
            return this->major < other.major || (this->major == other.major && this->minor < other.minor);
        }

        bool operator > (const WchFirmwareVersion& other) const {
            return other < *this;
        }

        bool operator <= (const WchFirmwareVersion& other) const {
            return !(other < *this);
        }

        bool operator >= (const WchFirmwareVersion& other) const {
            return !(*this < other);
        }
    };
}
