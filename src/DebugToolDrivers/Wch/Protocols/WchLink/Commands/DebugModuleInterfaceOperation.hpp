#pragma once

#include <cstdint>
#include <optional>

#include "src/DebugToolDrivers/Wch/Protocols/WchLink/Commands/Command.hpp"
#include "src/DebugToolDrivers/Wch/Protocols/WchLink/Responses/DebugModuleInterfaceOperationResponse.hpp"

#include "src/DebugToolDrivers/Protocols/RiscVDebugSpec/DebugModule/DebugModule.hpp"

namespace DebugToolDrivers::Wch::Protocols::WchLink::Commands
{
    class DebugModuleInterfaceOperation: public Command<std::array<unsigned char, 6>>
    {
    public:
        using ExpectedResponseType = Responses::DebugModuleInterfaceOperationResponse;

        DebugModuleInterfaceOperation(
            DebugToolDrivers::Protocols::RiscVDebugSpec::DebugModule::DmiOperation operation,
            DebugToolDrivers::Protocols::RiscVDebugSpec::DebugModule::RegisterAddress address,
            std::optional<DebugToolDrivers::Protocols::RiscVDebugSpec::DebugModule::RegisterValue> value = std::nullopt
        )
            : Command(0x08)
        {
            if (!value.has_value()) {
                value = 0x00;
            }

            this->payload = {
                address,
                static_cast<unsigned char>(*value >> 24),
                static_cast<unsigned char>(*value >> 16),
                static_cast<unsigned char>(*value >> 8),
                static_cast<unsigned char>(*value),
                static_cast<unsigned char>(operation),
            };
        }
    };
}
