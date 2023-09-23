#include "Avr8TargetConfig.hpp"

#include "src/Services/PathService.hpp"
#include "src/Services/StringService.hpp"

#include "src/Exceptions/InvalidConfig.hpp"

namespace Targets::Microchip::Avr::Avr8Bit
{
    Avr8TargetConfig::Avr8TargetConfig(const TargetConfig& targetConfig)
        : TargetConfig(targetConfig)
    {
        using Exceptions::InvalidConfig;

        const auto& targetNode = targetConfig.targetNode;

        if (!targetNode["physicalInterface"]) {
            throw InvalidConfig("Missing physical interface config parameter for AVR8 target.");
        }

        const auto physicalInterfaceName = Services::StringService::asciiToLower(targetNode["physicalInterface"].as<std::string>());
        const auto physicalInterfaceIt = Avr8TargetConfig::debugPhysicalInterfacesByConfigName.find(
            physicalInterfaceName
        );

        if (physicalInterfaceIt == Avr8TargetConfig::debugPhysicalInterfacesByConfigName.end()) {
            throw InvalidConfig(
                "Invalid physical interface provided (\"" + physicalInterfaceName + "\") for AVR8 target. "
                "See " + Services::PathService::homeDomainName() + "/docs/configuration/avr8-physical-interfaces for valid physical "
                "interface configuration values."
            );
        }

        this->physicalInterface = physicalInterfaceIt->second;

        // The 'manageDwenFuseBit' param used to be 'updateDwenFuseBit' - we still support the old, for now.
        if (targetNode["updateDwenFuseBit"]) {
            this->manageDwenFuseBit = targetNode["updateDwenFuseBit"].as<bool>(
                this->manageDwenFuseBit
            );
        }

        if (targetNode["manageDwenFuseBit"]) {
            this->manageDwenFuseBit = targetNode["manageDwenFuseBit"].as<bool>(
                this->manageDwenFuseBit
            );
        }

        if (targetNode["cycleTargetPowerPostDwenUpdate"]) {
            this->cycleTargetPowerPostDwenUpdate = targetNode["cycleTargetPowerPostDwenUpdate"].as<bool>(
                this->cycleTargetPowerPostDwenUpdate
            );
        }

        if (targetNode["disableDebugWirePreDisconnect"]) {
            this->disableDebugWireOnDeactivate = targetNode["disableDebugWirePreDisconnect"].as<bool>(
                this->disableDebugWireOnDeactivate
            );
        }

        if (targetNode["targetPowerCycleDelay"]) {
            this->targetPowerCycleDelay = std::chrono::milliseconds(targetNode["targetPowerCycleDelay"].as<int>(
                this->targetPowerCycleDelay.count()
            ));
        }

        if (targetNode["manageOcdenFuseBit"]) {
            this->manageOcdenFuseBit = targetNode["manageOcdenFuseBit"].as<bool>(
                this->manageOcdenFuseBit
            );
        }

        if (targetNode["preserveEeprom"]) {
            this->preserveEeprom = targetNode["preserveEeprom"].as<bool>(
                this->preserveEeprom
            );
        }

        if (targetNode["reserveSteppingBreakpoint"]) {
            this->reserveSteppingBreakpoint = targetNode["reserveSteppingBreakpoint"].as<bool>(
                this->reserveSteppingBreakpoint
            );
        }
    }
}
