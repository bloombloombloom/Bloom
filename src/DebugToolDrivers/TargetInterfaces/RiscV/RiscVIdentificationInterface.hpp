#pragma once

#include <string>

namespace DebugToolDrivers::TargetInterfaces::RiscV
{
    class RiscVIdentificationInterface
    {
    public:
        /**
         * Should retrieve the RISC-V target ID in string form.
         *
         * @return
         *  The target ID, in the form of a string.
         */
        virtual std::string getDeviceId() = 0;
    };
}
