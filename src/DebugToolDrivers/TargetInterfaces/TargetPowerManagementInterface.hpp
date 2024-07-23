#pragma once

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

        TargetPowerManagementInterface(const TargetPowerManagementInterface& other) = delete;
        TargetPowerManagementInterface& operator = (const TargetPowerManagementInterface& other) = delete;

        TargetPowerManagementInterface(TargetPowerManagementInterface&& other) = default;
        TargetPowerManagementInterface& operator = (TargetPowerManagementInterface&& other) = default;

        virtual void enableTargetPower() = 0;
        virtual void disableTargetPower() = 0;
    };
}
