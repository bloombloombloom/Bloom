#include "EdbgTargetPowerManagementInterface.hpp"

#include "src/DebugToolDrivers/Microchip/Protocols/EDBG/AVR/CommandFrames/EDBGControl/GetParameter.hpp"
#include "src/DebugToolDrivers/Microchip/Protocols/EDBG/AVR/CommandFrames/EDBGControl/SetParameter.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg
{
    using namespace Exceptions;

    using Microchip::Protocols::Edbg::Avr::ResponseFrames::EdbgControl::EdbgControlResponseId;

    using Microchip::Protocols::Edbg::Avr::CommandFrames::EdbgControl::GetParameter;
    using Microchip::Protocols::Edbg::Avr::CommandFrames::EdbgControl::SetParameter;

    EdbgTargetPowerManagementInterface::EdbgTargetPowerManagementInterface(EdbgInterface* edbgInterface)
        : edbgInterface(edbgInterface)
    {}

    void EdbgTargetPowerManagementInterface::enableTargetPower() {
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            SetParameter(EdbgParameters::CONTROL_TARGET_POWER, 0x01)
        );

        if (responseFrame.id == EdbgControlResponseId::FAILED) {
            throw Exception("Failed to enable target power via EDBG Control protocol");
        }
    }

    void EdbgTargetPowerManagementInterface::disableTargetPower() {
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            SetParameter(EdbgParameters::CONTROL_TARGET_POWER, 0x00)
        );

        if (responseFrame.id == EdbgControlResponseId::FAILED) {
            throw Exception("Failed to disable target power via EDBG Control protocol");
        }
    }
}
