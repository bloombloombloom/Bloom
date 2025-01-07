#include "IspParameters.hpp"

#include "src/Services/StringService.hpp"

namespace Targets::Microchip::Avr8
{
    IspParameters::IspParameters(const TargetDescriptionFile& targetDescriptionFile) {
        using Services::StringService;

        const auto& ispGroup = targetDescriptionFile.getPropertyGroup("isp_interface");

        this->programModeTimeout = StringService::toUint8(ispGroup.getProperty("ispenterprogmode_timeout").value);
        this->programModeStabilizationDelay = StringService::toUint8(
            ispGroup.getProperty("ispenterprogmode_stabdelay").value
        );
        this->programModeCommandExecutionDelay = StringService::toUint8(
            ispGroup.getProperty("ispenterprogmode_cmdexedelay").value
        );
        this->programModeSyncLoops = StringService::toUint8(ispGroup.getProperty("ispenterprogmode_synchloops").value);
        this->programModeByteDelay = StringService::toUint8(ispGroup.getProperty("ispenterprogmode_bytedelay").value);
        this->programModePollValue = StringService::toUint8(ispGroup.getProperty("ispenterprogmode_pollvalue").value);
        this->programModePollIndex = StringService::toUint8(ispGroup.getProperty("ispenterprogmode_pollindex").value);
        this->programModePreDelay = StringService::toUint8(ispGroup.getProperty("ispleaveprogmode_predelay").value);
        this->programModePostDelay = StringService::toUint8(ispGroup.getProperty("ispleaveprogmode_postdelay").value);
        this->readSignaturePollIndex = StringService::toUint8(ispGroup.getProperty("ispreadsign_pollindex").value);
        this->readFusePollIndex = StringService::toUint8(ispGroup.getProperty("ispreadfuse_pollindex").value);
        this->readLockPollIndex = StringService::toUint8(ispGroup.getProperty("ispreadlock_pollindex").value);
    }
}
