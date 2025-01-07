#pragma once

#include <chrono>
#include <string>
#include <map>

#include "src/ProjectConfig.hpp"

namespace Targets::Microchip::Avr8
{
    /**
     * Extending the generic TargetConfig struct to accommodate AVR8 target configuration parameters.
     */
    struct Avr8TargetConfig: public TargetConfig
    {
    public:
        /**
         * Because the debugWIRE module requires control of the reset pin on the target, enabling this module will
         * effectively mean losing control of the reset pin. This means users won't be able to use other
         * interfaces that require access to the reset pin, such as ISP, until the debugWIRE module is disabled.
         *
         * The EdbgAvr8Interface provides a function for temporarily disabling the debugWIRE module on the target.
         * This doesn't change the DWEN fuse and its affect is only temporary - the debugWIRE module will be
         * reactivated upon the user cycling the power to the target.
         *
         * Bloom is able to temporarily disable the debugWIRE module, automatically, upon deactivating of the
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
         * The manageDwenFuseBit flag determines if Bloom should manage the DWEN fuse bit, for debugWIRE sessions.
         *
         * This parameter is optional, and the function is disabled by default. Users must explicitly enable it in
         * their target configuration.
         */
        bool manageDwenFuseBit = false;

        /**
         * For debug tools that provide target power management functions (such as some evaluation boards), Bloom can
         * automatically cycle the target power after updating the DWEN fuse bit, for debugWIRE sessions. This parameter
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
        std::chrono::milliseconds targetPowerCycleDelay = std::chrono::milliseconds{250};

        /**
         * The manageOcdenFuseBit flag determines if Bloom should manage the OCDEN fuse but on JTAG-enabled AVR
         * targets.
         *
         * This parameter is optional, and the function is disabled by default. Users must explicitly enable it in
         * their target configuration.
         */
        bool manageOcdenFuseBit = false;

        /**
         * With JTAG and UPDI targets, we have to perform a full chip erase when updating the target's flash memory.
         * This means the user will lose their EEPROM data whenever they wish to upload any program changes via Bloom.
         *
         * The preserveEeprom flag determines if Bloom should preserve the target's EEPROM by setting the EESAVE fuse
         * bit before performing the chip erase.
         *
         * This parameter is optional, and the function is enabled by default.
         */
        bool preserveEeprom = true;

        /**
         * Determines whether Bloom will check for an AVR signature mismatch between the signature in the TDF and the
         * connected target signature.
         */
        bool signatureValidation = true;

        /**
         * Determines whether Bloom will stop all timer peripherals on the target, when target execution is stopped.
         */
        bool stopAllTimers = true;

        explicit Avr8TargetConfig(const TargetConfig& targetConfig);
    };
}
