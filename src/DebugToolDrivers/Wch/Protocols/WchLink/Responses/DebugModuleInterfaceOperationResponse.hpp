#pragma once

#include <vector>

#include "src/DebugToolDrivers/Protocols/RiscVDebugSpec/DebugModule/DebugModule.hpp"
#include "src/Services/StringService.hpp"

#include "src/TargetController/Exceptions/DeviceCommunicationFailure.hpp"

namespace DebugToolDrivers::Wch::Protocols::WchLink::Responses
{
    class DebugModuleInterfaceOperationResponse
    {
    public:
        DebugToolDrivers::Protocols::RiscVDebugSpec::DebugModule::DmiOperationStatus operationStatus;
        DebugToolDrivers::Protocols::RiscVDebugSpec::DebugModule::RegisterAddress address;
        DebugToolDrivers::Protocols::RiscVDebugSpec::DebugModule::RegisterValue value;

        explicit DebugModuleInterfaceOperationResponse(const std::vector<unsigned char>& payload) {
            using DebugToolDrivers::Protocols::RiscVDebugSpec::DebugModule::DmiOperationStatus;

            if (payload.size() != 6) {
                throw Exceptions::DeviceCommunicationFailure{
                    "Unexpected response payload size for DMI operation command"
                };
            }

            const auto status = payload[5];
            if (
                status != static_cast<unsigned char>(DmiOperationStatus::SUCCESS)
                && status != static_cast<unsigned char>(DmiOperationStatus::FAILED)
                && status != static_cast<unsigned char>(DmiOperationStatus::BUSY)
            ) {
                throw Exceptions::DeviceCommunicationFailure{
                    "Unknown DMI operation status returned: 0x" + Services::StringService::toHex(status)
                };
            }

            this->operationStatus = static_cast<DmiOperationStatus>(status);
            this->address = payload[0];
            this->value = static_cast<DebugToolDrivers::Protocols::RiscVDebugSpec::DebugModule::RegisterValue>(
                (payload[1] << 24) | (payload[2] << 16) | (payload[3] << 8) | (payload[4])
            );
        }
    };
}
