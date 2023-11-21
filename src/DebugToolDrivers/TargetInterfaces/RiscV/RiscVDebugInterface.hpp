#pragma once

#include <cstdint>

#include "src/Targets/RiscV/DebugModule/DebugModule.hpp"
#include "src/Targets/RiscV/TargetParameters.hpp"

namespace DebugToolDrivers::TargetInterfaces::RiscV
{
    class RiscVDebugInterface
    {
    public:
        RiscVDebugInterface() = default;
        virtual ~RiscVDebugInterface() = default;

        RiscVDebugInterface(const RiscVDebugInterface& other) = default;
        RiscVDebugInterface(RiscVDebugInterface&& other) = default;

        RiscVDebugInterface& operator = (const RiscVDebugInterface& other) = default;
        RiscVDebugInterface& operator = (RiscVDebugInterface&& other) = default;

        /**
         * Should prepare for and then activate the physical interface between the debug tool and the RISC-V target.
         *
         * Should throw an exception if activation fails. The error will be considered fatal, and result in a shutdown.
         *
         * Unless otherwise stated, it can be assumed that this function will be called (and must succeed)
         * before any of the other functions below this point are called. In other words, we can assume that the
         * interface has been activated in the implementations of any of the functions below this point.
         *
         * @param targetParameters
         *  Parameters for the RISC-V target. These can be ignored if a particular implementation does not require
         *  any target parameters for activation.
         */
        virtual void activate(const Targets::RiscV::TargetParameters& targetParameters) = 0;

        /**
         * Should deactivate the physical interface between the debug tool and the RISC-V target.
         *
         * CAUTION: This function **CAN** be called before activate(), or in instances where activate() failed (threw
         * an exception). Implementations must accommodate this.
         */
        virtual void deactivate() = 0;

        /**
         * Should retrieve the RISC-V target ID in string form.
         *
         * @return
         *  The target ID, in the form of a string.
         */
        virtual std::string getDeviceId() = 0;

        /**
         * Should read the value of a debug module register.
         *
         * @param address
         *  The address of the debug module register to read.
         *
         * @return
         *  The register value.
         */
        virtual Targets::RiscV::DebugModule::RegisterValue readDebugModuleRegister(
            Targets::RiscV::DebugModule::RegisterAddress address
        ) = 0;

        /**
         * Should write a value to a debug module register.
         *
         * @param address
         *  The address of the debug module to update.
         *
         * @param value
         *  The value to write.
         */
        virtual void writeDebugModuleRegister(
            Targets::RiscV::DebugModule::RegisterAddress address,
            Targets::RiscV::DebugModule::RegisterValue value
        ) = 0;
    };
}
