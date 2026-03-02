#pragma once

#include <cstdint>
#include <string>

namespace DebugToolDrivers::Microchip::Protocols::Edbg
{
    class EdbgFirmwareVersion
    {
    public:
        std::uint8_t major = 0;
        std::uint8_t minor = 0;
        std::uint16_t build = 0;

        [[nodiscard]] std::string toString() const {
            return std::to_string(this->major) + "." + std::to_string(this->minor) + "-" + std::to_string(this->build);
        }

        bool operator == (const EdbgFirmwareVersion& other) const {
            return this->major == other.major && this->minor == other.minor && this->build == other.build;
        }

        bool operator != (const EdbgFirmwareVersion& other) const {
            return !(*this == other);
        }

        bool operator < (const EdbgFirmwareVersion& other) const {
            return
                this->major < other.major
                || (this->major == other.major && this->minor < other.minor)
                || (this->major == other.major && this->minor == other.minor && this->build < other.build)
            ;
        }

        bool operator > (const EdbgFirmwareVersion& other) const {
            return other < *this;
        }

        bool operator <= (const EdbgFirmwareVersion& other) const {
            return !(other < *this);
        }

        bool operator >= (const EdbgFirmwareVersion& other) const {
            return !(*this < other);
        }
    };
}
