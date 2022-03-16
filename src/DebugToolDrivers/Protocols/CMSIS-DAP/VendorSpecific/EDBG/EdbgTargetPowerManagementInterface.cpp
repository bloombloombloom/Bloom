#include "EdbgTargetPowerManagementInterface.hpp"

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/EDBGControl/GetParameter.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/EDBGControl/SetParameter.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg
{
    using namespace Bloom::Exceptions;

    using Protocols::CmsisDap::Edbg::Avr::ResponseFrames::EdbgControl::EdbgControlResponseId;

    using Protocols::CmsisDap::Edbg::Avr::CommandFrames::EdbgControl::GetParameter;
    using Protocols::CmsisDap::Edbg::Avr::CommandFrames::EdbgControl::SetParameter;

    void EdbgTargetPowerManagementInterface::enableTargetPower() {
        auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(
            SetParameter(EdbgParameters::CONTROL_TARGET_POWER, 0x01)
        );

        if (response.getResponseId() == EdbgControlResponseId::FAILED) {
            throw Exception("Failed to enable target power via EDBG Control protocol");
        }
    }

    void EdbgTargetPowerManagementInterface::disableTargetPower() {
        auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(
            SetParameter(EdbgParameters::CONTROL_TARGET_POWER, 0x00)
        );

        if (response.getResponseId() == EdbgControlResponseId::FAILED) {
            throw Exception("Failed to disable target power via EDBG Control protocol");
        }
    }
}
