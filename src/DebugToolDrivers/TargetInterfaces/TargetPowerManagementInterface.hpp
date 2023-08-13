#pragma once

#include <cstdint>

namespace DebugToolDrivers::TargetInterfaces
{
    /**
     * Some debug tools provide target power management functions. Those that do should expose an implementation of
     * this interface via DebugTool::getTargetPowerManagementInterface();
     */
    class TargetPowerManagementInterface
    {
    public:
        TargetPowerManagementInterface() = default;
        virtual ~TargetPowerManagementInterface() = default;

        TargetPowerManagementInterface(const TargetPowerManagementInterface& other) = default;
        TargetPowerManagementInterface(TargetPowerManagementInterface&& other) = default;

        TargetPowerManagementInterface& operator = (const TargetPowerManagementInterface& other) = default;
        TargetPowerManagementInterface& operator = (TargetPowerManagementInterface&& other) = default;

        /**
         * Should enable the target power if currently disabled.
         */
        virtual void enableTargetPower() = 0;

        /**
         * Should disable the target power if currently enabled.
         */
        virtual void disableTargetPower() = 0;
    };
}
