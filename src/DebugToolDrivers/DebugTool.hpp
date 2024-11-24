#pragma once

#include "TargetInterfaces/TargetPowerManagementInterface.hpp"

#include "TargetInterfaces/Microchip/AVR8/Avr8DebugInterface.hpp"
#include "TargetInterfaces/Microchip/AVR8/AvrIspInterface.hpp"
#include "src/Targets/Microchip/AVR8/TargetDescriptionFile.hpp"
#include "src/Targets/Microchip/AVR8/Avr8TargetConfig.hpp"

#include "TargetInterfaces/RiscV/RiscVDebugInterface.hpp"
#include "src/Targets/RiscV/TargetDescriptionFile.hpp"
#include "src/Targets/RiscV/RiscVTargetConfig.hpp"

#include "src/Targets/TargetRegisterDescriptor.hpp"

/**
 * A debug tool can be any device that provides access to the connected target. Debug tools are usually connected
 * to the host machine via USB.
 *
 * Each debug tool must implement this interface. Note that target specific driver code should not be placed here.
 * Each target family will expect the debug tool to provide an interface for that particular group of targets.
 * For an example, see the Avr8DebugInterface class and DebugTool::getAvr8DebugInterface(), for the family of AVR
 * 8-bit targets.
 */
class DebugTool
{
public:
    DebugTool() = default;
    virtual ~DebugTool() = default;

    DebugTool(const DebugTool& other) = default;
    DebugTool(DebugTool&& other) = default;

    DebugTool& operator = (const DebugTool& other) = default;
    DebugTool& operator = (DebugTool&& other) = default;

    /**
     * Should establish a connection to the device and prepare it for a debug session.
     */
    virtual void init() = 0;

    /**
     * Should disconnect from the device after performing any tasks required to formally end the debug session.
     */
    virtual void close() = 0;

    /**
     * This function is called immediately after successful initialisation of the debug tool.
     */
    virtual void postInit() = 0;

    virtual bool isInitialised() const = 0;

    virtual std::string getName() = 0;

    virtual std::string getSerialNumber() = 0;

    /**
     * All debug tools that support target power management functions must provide an implementation of the
     * TargetPowerManagementInterface class, via this function.
     *
     * For debug tools that cannot manage target power, a nullptr should be returned.
     *
     * Note: the caller of this function will not manage the lifetime of the returned TargetPowerManagementInterface
     * instance.
     *
     * @return
     */
    virtual DebugToolDrivers::TargetInterfaces::TargetPowerManagementInterface* getTargetPowerManagementInterface() {
        return nullptr;
    }

    /**
     * All debug tools that support debugging operations on AVR8 targets must provide an implementation of
     * the Avr8DebugInterface class, via this function.
     *
     * For debug tools that do not support debugging on AVR8 targets, this function should return a nullptr.
     *
     * Note: the caller of this function will not manage the lifetime of the returned Avr8DebugInterface instance.
     *
     * @return
     */
    virtual DebugToolDrivers::TargetInterfaces::Microchip::Avr8::Avr8DebugInterface* getAvr8DebugInterface(
        const Targets::Microchip::Avr8::TargetDescriptionFile& targetDescriptionFile,
        const Targets::Microchip::Avr8::Avr8TargetConfig& targetConfig
    ) {
        return nullptr;
    }

    /**
     * All debug tools that support interfacing with AVR targets via the ISP interface must provide an
     * implementation of the AvrIspInterface class, via this function.
     *
     * For debug tools that do not support the interface, a nullptr should be returned.
     *
     * Note: the caller of this function will not manage the lifetime of the returned AvrIspInterface instance.
     *
     * @return
     */
    virtual DebugToolDrivers::TargetInterfaces::Microchip::Avr8::AvrIspInterface* getAvrIspInterface(
        const Targets::Microchip::Avr8::TargetDescriptionFile& targetDescriptionFile,
        const Targets::Microchip::Avr8::Avr8TargetConfig& targetConfig
    ) {
        return nullptr;
    }

    /**
     * All debug tools that support interfacing with RISC-V targets must provide an implementation of the
     * RiscVDebugInterface class, via this function.
     *
     * For debug tools that do not support this interface, a nullptr should be returned.
     *
     * Note: the caller of this function will not manage the lifetime of the returned instance.
     *
     * @return
     */
    virtual DebugToolDrivers::TargetInterfaces::RiscV::RiscVDebugInterface* getRiscVDebugInterface(
        const Targets::RiscV::TargetDescriptionFile& targetDescriptionFile,
        const Targets::RiscV::RiscVTargetConfig& targetConfig
    ) {
        return nullptr;
    }
};
