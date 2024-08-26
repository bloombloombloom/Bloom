#include "HelpMonitorInfo.hpp"

#include <string>

#include "src/DebugServer/Gdb/ResponsePackets/ResponsePacket.hpp"

#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"

#include "src/Targets/TargetFamily.hpp"

namespace DebugServer::Gdb::CommandPackets
{
    using Services::TargetControllerService;
    using Services::StringService;

    using ResponsePackets::ResponsePacket;

    HelpMonitorInfo::HelpMonitorInfo(Monitor&& monitorPacket)
        : Monitor(std::move(monitorPacket))
    {}

    void HelpMonitorInfo::handle(
        DebugSession& debugSession,
        const TargetDescriptor& gdbTargetDescriptor,
        const Targets::TargetDescriptor& targetDescriptor,
        TargetControllerService&
    ) {
        Logger::info("Handling HelpMonitorInfo packet");

        static constexpr auto LEFT_PADDING = std::string::size_type{3};
        static constexpr auto RIGHT_PADDING = std::string::size_type{26};

        static const auto leftPadding = std::string{LEFT_PADDING, ' ', std::string::allocator_type{}};
        static const auto leftRightPadding = std::string{
            LEFT_PADDING + RIGHT_PADDING,
            ' ',
            std::string::allocator_type{}
        };

        auto output = std::string{"\nSupported Bloom commands:\n\n"};
        output += leftPadding + StringService::padRight("help", ' ', RIGHT_PADDING);
        output += "Displays this help text.\n";
        output += leftPadding + StringService::padRight("version", ' ', RIGHT_PADDING);
        output += "Outputs Bloom's version information.\n";
        output += leftPadding + StringService::padRight("version machine", ' ', RIGHT_PADDING);
        output += "Outputs Bloom's version information in JSON format.\n";

#ifndef EXCLUDE_INSIGHT
        output += leftPadding + StringService::padRight("insight", ' ', RIGHT_PADDING);
        output += "Activates the Insight GUI.\n";
#endif

        output += "\n\n";
        output += leftPadding + StringService::padRight("reset", ' ', RIGHT_PADDING);
        output += "Resets the target and holds it in a stopped state.\n\n";

        if (targetDescriptor.family == Targets::TargetFamily::AVR_8) {
            output += leftPadding + StringService::padRight("eeprom fill [VALUE]", ' ', RIGHT_PADDING);
            output += "Fills the target's EEPROM with the specified value. The value should be in hexadecimal\n";
            output += leftRightPadding + "format: \"monitor eeprom fill AABBCC\". If the specified value is smaller than the EEPROM\n";
            output += leftRightPadding + "memory segment size, it will be repeated across the entire segment address range. If the\n";
            output += leftRightPadding + "value size is not a multiple of the segment size, the value will be truncated in the final\n";
            output += leftRightPadding + "repetition. The value size must not exceed the segment size.\n";
        }

        debugSession.connection.writePacket(ResponsePacket{Services::StringService::toHex(output)});
    }
}
