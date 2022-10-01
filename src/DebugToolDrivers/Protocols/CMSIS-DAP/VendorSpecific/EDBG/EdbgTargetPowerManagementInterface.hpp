#pragma once

#include <cstdint>

#include "src/DebugToolDrivers/TargetInterfaces/TargetPowerManagementInterface.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/EdbgInterface.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg
{
    class EdbgTargetPowerManagementInterface: public TargetInterfaces::TargetPowerManagementInterface
    {
    public:
        explicit EdbgTargetPowerManagementInterface(EdbgInterface* edbgInterface);

        /**
         * Issues a Set Parameter command to the EDBG tool, to enable the target power.
         */
        void enableTargetPower() override;

        /**
         * Issues a Set Parameter command to the EDBG tool, to disable the target power.
         */
        void disableTargetPower() override;

    private:
        EdbgInterface* edbgInterface;
    };
}
