#include "Avr8TargetConfig.hpp"

#include "src/Logger/Logger.hpp"
#include "src/Exceptions/InvalidConfig.hpp"

namespace Bloom::Targets::Microchip::Avr::Avr8Bit
{
    Avr8TargetConfig::Avr8TargetConfig(const TargetConfig& targetConfig): TargetConfig(targetConfig) {
        using Bloom::Exceptions::InvalidConfig;

        if (!targetConfig.jsonObject.contains("physicalInterface")) {
            throw InvalidConfig("Missing physical interface config parameter for AVR8 target.");
        }

        const auto physicalInterfaceName = targetConfig.jsonObject.value("physicalInterface").toString().toLower()
            .toStdString();

        static const auto physicalInterfacesByName = Avr8TargetConfig::getPhysicalInterfacesByName();

        if (!physicalInterfacesByName.contains(physicalInterfaceName)) {
            throw InvalidConfig("Invalid physical interface config parameter for AVR8 target.");
        }

        this->physicalInterface = physicalInterfacesByName.at(physicalInterfaceName);

        if (targetConfig.jsonObject.contains("updateDwenFuseBit")) {
            this->updateDwenFuseBit = targetConfig.jsonObject.value("updateDwenFuseBit").toBool();
        }

        if (targetConfig.jsonObject.contains("cycleTargetPowerPostDwenUpdate")) {
            this->cycleTargetPowerPostDwenUpdate = targetConfig.jsonObject.value(
                "cycleTargetPowerPostDwenUpdate"
            ).toBool();
        }

        if (targetConfig.jsonObject.contains("disableDebugWirePreDisconnect")) {
            this->disableDebugWireOnDeactivate = targetConfig.jsonObject.value(
                "disableDebugWirePreDisconnect"
            ).toBool();
        }

        if (targetConfig.jsonObject.contains("targetPowerCycleDelay")) {
            this->targetPowerCycleDelay = std::chrono::milliseconds(targetConfig.jsonObject.value(
                "targetPowerCycleDelay"
            ).toInt(static_cast<int>(this->targetPowerCycleDelay.count())));
        }
    }
}
