#pragma once

#include <cstdint>
#include <string>

#include "RiscVGeneric.hpp"
#include "DebugModule/DebugModule.hpp"
#include "DebugModule/Registers/RegisterAddresses.hpp"

namespace DebugToolDrivers::Protocols::RiscVDebugSpec
{
    /**
     * Provides access to the RISC-V target's debug transport module (DTM).
     */
    class DebugTransportModuleInterface
    {
    public:
        /**
         * Should prepare for and then activate the physical interface between the debug tool and the RISC-V target.
         *
         * Should throw an exception if activation fails. The error will be considered fatal, and result in a shutdown.
         *
         * Unless otherwise stated, it can be assumed that this function will be called (and must succeed)
         * before any of the other functions below this point are called. In other words, we can assume that the
         * interface has been activated in the implementations of any of the functions below this point.
         */
        virtual void activate() = 0;

        /**
         * Should deactivate the physical interface between the debug tool and the RISC-V target.
         *
         * CAUTION: This function **CAN** be called before activate(), or in instances where activate() failed (threw
         * an exception). Implementations must accommodate this.
         */
        virtual void deactivate() = 0;

        /**
         * Should read the value of a debug module register.
         *
         * @param address
         *
         * @return
         */
        virtual DebugModule::RegisterValue readDebugModuleRegister(DebugModule::RegisterAddress address) = 0;

        DebugModule::RegisterValue readDebugModuleRegister(DebugModule::Registers::RegisterAddress address) {
            return this->readDebugModuleRegister(static_cast<DebugModule::RegisterAddress>(address));
        };

        /**
         * Should write a value to a debug module register.
         *
         * @param address
         * @param value
         */
        virtual void writeDebugModuleRegister(
            DebugModule::RegisterAddress address,
            DebugModule::RegisterValue value
        ) = 0;

        void writeDebugModuleRegister(
            DebugModule::Registers::RegisterAddress address,
            DebugModule::RegisterValue value
        ) {
            return this->writeDebugModuleRegister(
                static_cast<DebugModule::RegisterAddress>(address),
                value
            );
        };
    };
}
