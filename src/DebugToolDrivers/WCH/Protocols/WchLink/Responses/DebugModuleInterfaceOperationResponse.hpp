#pragma once

#include <vector>

#include "src/Targets/RiscV/DebugModule/DebugModule.hpp"
#include "src/Helpers/BiMap.hpp"
#include "src/Services/StringService.hpp"

#include "src/TargetController/Exceptions/DeviceCommunicationFailure.hpp"

namespace DebugToolDrivers::Wch::Protocols::WchLink::Responses
{
    class DebugModuleInterfaceOperationResponse
    {
    public:
        Targets::RiscV::DebugModule::DmiOperationStatus operationStatus;
        Targets::RiscV::DebugModule::RegisterAddress address;
        Targets::RiscV::DebugModule::RegisterValue value;

        explicit DebugModuleInterfaceOperationResponse(const std::vector<unsigned char>& payload)
        {
            if (payload.size() != 6) {
                throw Exceptions::DeviceCommunicationFailure(
                    "Unexpected response payload size for DMI operation command"
                );
            }

            const auto status = payload[5];
            if (
                status != static_cast<unsigned char>(Targets::RiscV::DebugModule::DmiOperationStatus::SUCCESS)
                && status != static_cast<unsigned char>(Targets::RiscV::DebugModule::DmiOperationStatus::FAILED)
                && status != static_cast<unsigned char>(Targets::RiscV::DebugModule::DmiOperationStatus::BUSY)
            ) {
                throw Exceptions::DeviceCommunicationFailure(
                    "Unknown DMI operation status returned: 0x" + Services::StringService::toHex(status)
                );
            }

            this->operationStatus = static_cast<Targets::RiscV::DebugModule::DmiOperationStatus>(status);
            this->address = payload[0];
            this->value = static_cast<Targets::RiscV::DebugModule::RegisterValue>(
                (payload[1] << 24) | (payload[2] << 16) | (payload[3] << 8) | (payload[4])
            );
        }
    };
}
