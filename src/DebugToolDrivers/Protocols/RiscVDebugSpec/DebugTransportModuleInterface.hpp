#pragma once

#include <cstdint>
#include <string>

#include "Common.hpp"
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
