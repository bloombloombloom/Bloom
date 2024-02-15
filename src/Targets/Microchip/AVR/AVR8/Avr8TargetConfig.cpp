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
