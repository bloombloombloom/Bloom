#include "Avr8TargetConfig.hpp"

namespace Targets::Microchip::Avr8
{
    Avr8TargetConfig::Avr8TargetConfig(const TargetConfig& targetConfig)
        : TargetConfig(targetConfig)
    {
        const auto& targetNode = targetConfig.targetNode;

        if (targetNode["manage_dwen_fuse_bit"]) {
            this->manageDwenFuseBit = targetNode["manage_dwen_fuse_bit"].as<bool>(
                this->manageDwenFuseBit
            );
        }

        if (targetNode["cycle_target_power_post_dwen_update"]) {
            this->cycleTargetPowerPostDwenUpdate = targetNode["cycle_target_power_post_dwen_update"].as<bool>(
                this->cycleTargetPowerPostDwenUpdate
            );
        }

        if (targetNode["disable_debug_wire_pre_disconnect"]) {
            this->disableDebugWireOnDeactivate = targetNode["disable_debug_wire_pre_disconnect"].as<bool>(
                this->disableDebugWireOnDeactivate
            );
        }

        if (targetNode["target_power_cycle_delay"]) {
            this->targetPowerCycleDelay = std::chrono::milliseconds{targetNode["target_power_cycle_delay"].as<int>(
                this->targetPowerCycleDelay.count()
            )};
        }

        if (targetNode["manage_ocden_fuse_bit"]) {
            this->manageOcdenFuseBit = targetNode["manage_ocden_fuse_bit"].as<bool>(this->manageOcdenFuseBit);
        }

        if (targetNode["preserve_eeprom"]) {
            this->preserveEeprom = targetNode["preserve_eeprom"].as<bool>(this->preserveEeprom);
        }
    }
}
