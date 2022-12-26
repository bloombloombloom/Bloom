#include "Avr8TargetConfig.hpp"

#include "src/Helpers/String.hpp"
#include "src/Services/PathService.hpp"
#include "src/Exceptions/InvalidConfig.hpp"

namespace Bloom::Targets::Microchip::Avr::Avr8Bit
{
    Avr8TargetConfig::Avr8TargetConfig(const TargetConfig& targetConfig)
        : TargetConfig(targetConfig)
    {
        using Bloom::Exceptions::InvalidConfig;

        const auto& targetNode = targetConfig.targetNode;

        if (!targetNode["physicalInterface"]) {
            throw InvalidConfig("Missing physical interface config parameter for AVR8 target.");
        }

        const auto physicalInterfaceName = String::asciiToLower(targetNode["physicalInterface"].as<std::string>());
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
            this->manageDwenFuseBit = targetNode["updateDwenFuseBit"].as<bool>();
        }

        if (targetNode["manageDwenFuseBit"]) {
            this->manageDwenFuseBit = targetNode["manageDwenFuseBit"].as<bool>();
        }

        if (targetNode["cycleTargetPowerPostDwenUpdate"]) {
            this->cycleTargetPowerPostDwenUpdate = targetNode["cycleTargetPowerPostDwenUpdate"].as<bool>();
        }

        if (targetNode["disableDebugWirePreDisconnect"]) {
            this->disableDebugWireOnDeactivate = targetNode["disableDebugWirePreDisconnect"].as<bool>();
        }

        if (targetNode["targetPowerCycleDelay"]) {
            this->targetPowerCycleDelay = std::chrono::milliseconds(targetNode["targetPowerCycleDelay"].as<int>());
        }
    }
}
