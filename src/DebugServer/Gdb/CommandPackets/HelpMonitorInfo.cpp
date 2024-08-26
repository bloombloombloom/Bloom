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

        static constexpr auto CMD_COLOR = StringService::TerminalColor::DARK_YELLOW;
        static constexpr auto PARAM_COLOR = StringService::TerminalColor::BLUE;

        static const auto leftPadding = std::string{std::string::size_type{3}, ' ', std::string::allocator_type{}};

        auto output = std::string{"\nSupported Bloom commands:\n\n"};

        output += StringService::applyTerminalColor("help", CMD_COLOR) + "\n";
        output += leftPadding + "Displays this help text.\n\n";

        output += StringService::applyTerminalColor("version", CMD_COLOR) + "\n";
        output += leftPadding + "Outputs Bloom's version information.\n\n";

        output += StringService::applyTerminalColor( "version machine", CMD_COLOR) + "\n";
        output += leftPadding + "Outputs Bloom's version information in JSON format.\n\n";

#ifndef EXCLUDE_INSIGHT
        output += StringService::applyTerminalColor("insight", CMD_COLOR) + "\n";
        output += leftPadding + "Activates the Insight GUI.\n\n";
#endif

        output += StringService::applyTerminalColor("reset", CMD_COLOR) + "\n";
        output += leftPadding + "Resets the target and holds it in a stopped state.\n\n";

        output += StringService::applyTerminalColor("lr", CMD_COLOR)
            + " [" + StringService::applyTerminalColor("PERIPHERAL_KEY", PARAM_COLOR) + "] ["
            + StringService::applyTerminalColor("ABS_REG_GROUP_KEY", PARAM_COLOR) + "]\n";
        output += leftPadding + "Lists all target registers in the given peripheral and register group.\n";
        output += leftPadding + "If a peripheral key is not provided, all registers across all peripherals will be listed.\n\n";
        output += leftPadding + "Examples:\n\n";
        output += leftPadding + "mon " + StringService::applyTerminalColor("lr", CMD_COLOR) + "\n";
        output += leftPadding + "  To list all target registers across all peripherals.\n\n";
        output += leftPadding + "mon " + StringService::applyTerminalColor("lr", CMD_COLOR)
            + " " + StringService::applyTerminalColor("tca0", PARAM_COLOR) + "\n";
        output += leftPadding + "  To list all target registers in the `tca0` peripheral.\n\n";
        output += leftPadding + "mon " + StringService::applyTerminalColor("lr", CMD_COLOR)
            + " " + StringService::applyTerminalColor("tca0", PARAM_COLOR) + " "
            + StringService::applyTerminalColor("tca.single", PARAM_COLOR) + "\n";
        output += leftPadding + "  To list all target registers in the `tca.single` register group, in the `tca0` peripheral.\n\n";

        output += StringService::applyTerminalColor("rr", CMD_COLOR)
            + " [" + StringService::applyTerminalColor("PERIPHERAL_KEY", PARAM_COLOR) + "] ["
            + StringService::applyTerminalColor("ABS_REG_GROUP_KEY", PARAM_COLOR) + "] ["
            + StringService::applyTerminalColor("REG_KEY", PARAM_COLOR) + "]\n";
        output += leftPadding + "Reads the value of the given register.\n";
        output += leftPadding + "If a register key is not provided, all registers in the given peripheral and register group will be read.\n";
        output += leftPadding + "The register group key can be omitted if the peripheral contains a single register group,\n";
        output += leftPadding + "and the register resides directly within that group (not in any subgroup).\n\n";
        output += leftPadding + "Examples:\n\n";
        output += leftPadding + "mon " + StringService::applyTerminalColor("rr", CMD_COLOR)
            + " " + StringService::applyTerminalColor("porta", PARAM_COLOR) + " "
            + StringService::applyTerminalColor("port", PARAM_COLOR) + " "
            + StringService::applyTerminalColor("dir", PARAM_COLOR) + "\n";
        output += leftPadding + "  To read the `dir` register in the `port` register group, in the `porta` peripheral.\n\n";
        output += leftPadding + "mon " + StringService::applyTerminalColor("rr", CMD_COLOR)
            + " " + StringService::applyTerminalColor("porta", PARAM_COLOR) + " "
            + StringService::applyTerminalColor("dir", PARAM_COLOR) + "\n";
        output += leftPadding + "  Same as above, excluding the register group key.\n\n";
        output += leftPadding + "mon " + StringService::applyTerminalColor("rr", CMD_COLOR)
            + " " + StringService::applyTerminalColor("tca0", PARAM_COLOR) + " "
            + StringService::applyTerminalColor("tca.split", PARAM_COLOR) + "\n";
        output += leftPadding + "  To read all registers in the `tca.split` register group, in the `tca0` peripheral.\n\n";
        output += leftPadding + "mon " + StringService::applyTerminalColor("rr", CMD_COLOR)
            + " " + StringService::applyTerminalColor("porta", PARAM_COLOR) + "\n";
        output += leftPadding + "  To read all registers in the `porta` peripheral.\n\n";

        if (targetDescriptor.family == Targets::TargetFamily::AVR_8) {
            output += StringService::applyTerminalColor("eeprom fill", CMD_COLOR)
                  + " [" + StringService::applyTerminalColor("VALUE", PARAM_COLOR) + "]\n";
            output += leftPadding + "Fills the target's EEPROM with the specified value. The value should be in hexadecimal\n";
            output += leftPadding + "format: \"monitor eeprom fill AABBCC\". If the specified value is smaller than the EEPROM\n";
            output += leftPadding + "memory segment size, it will be repeated across the entire segment address range. If the\n";
            output += leftPadding + "value size is not a multiple of the segment size, the value will be truncated in the final\n";
            output += leftPadding + "repetition. The value size must not exceed the segment size.\n";
        }

        output += "\n";
        debugSession.connection.writePacket(ResponsePacket{Services::StringService::toHex(output)});
    }
}
