#pragma once

#include <chrono>
#include <string>
#include <map>

#include "src/ProjectConfig.hpp"

#include "PhysicalInterface.hpp"

namespace Bloom::Targets::Microchip::Avr::Avr8Bit
{
    /**
     * Extending the generic TargetConfig struct to accommodate AVR8 target configuration parameters.
     */
    struct Avr8TargetConfig: public TargetConfig
    {
    public:
        /**
         * The physical interface is the interface used for communication between the debug tool and the connected
         * target.
         */
        PhysicalInterface physicalInterface = PhysicalInterface::DEBUG_WIRE;

        /**
         * Because the debugWire module requires control of the reset pin on the target, enabling this module will
         * effectively mean losing control of the reset pin. This means users won't be able to use other
         * interfaces that require access to the reset pin, such as ISP, until the debugWire module is disabled.
         *
         * The EdbgAvr8Interface provides a function for temporarily disabling the debugWire module on the target.
         * This doesn't change the DWEN fuse and its affect is only temporary - the debugWire module will be
         * reactivated upon the user cycling the power to the target.
         *
         * Bloom is able to temporarily disable the debugWire module, automatically, upon deactivating of the
         * target (which usually occurs after a debug session has ended). This allows users to program the target via
         * ISP, after they've finished a debug session. After programming the target, the user will need to cycle the
         * target power before Bloom can gain access for another debug session. This flag control this function.
         *
         * NOTE: Currently, this flag is only honoured by the EdbgAvr8Interface.
         *
         * See the EdbgAvr8Interface::disableDebugWire() function for more.
         */
        bool disableDebugWireOnDeactivate = false;

        /**
         * The updateDwenFuseBit flag determines if Bloom should manage the DWEN fuse bit, for debugWire sessions.
         *
         * This parameter is optional, and the function is disabled by default. Users must explicitly enable it in
         * their target configuration.
         */
        bool updateDwenFuseBit = false;

        /**
         * For debug tools that provide target power management functions (such as some evaluation boards), Bloom can
         * automatically cycle the target power after updating the DWEN fuse bit, for debugWire sessions. This parameter
         * controls this function.
         *
         * This parameter is optional. The function is enabled by default.
         */
        bool cycleTargetPowerPostDwenUpdate = true;

        /**
         * When cycling target power after updating the DWEN fuse bit, we wait for a number of milliseconds, for the
         * target to power-down and back up.
         *
         * This parameter determines how long we wait.
         */
        std::chrono::milliseconds targetPowerCycleDelay = std::chrono::milliseconds(250);

        explicit Avr8TargetConfig(const TargetConfig& targetConfig);

    private:
        static inline auto debugPhysicalInterfacesByConfigName = std::map<std::string, PhysicalInterface>({
            {"debugwire", PhysicalInterface::DEBUG_WIRE}, // Deprecated - left here for backwards compatibility
            {"debug-wire", PhysicalInterface::DEBUG_WIRE},
            {"pdi", PhysicalInterface::PDI},
            {"jtag", PhysicalInterface::JTAG},
            {"updi", PhysicalInterface::UPDI},
        });
    };
}
