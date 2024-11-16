#include "RiscVGdbRsp.hpp"

#include "src/Services/StringService.hpp"

// Command packets
#include "CommandPackets/ReadRegister.hpp"
#include "CommandPackets/ReadRegisters.hpp"
#include "CommandPackets/WriteRegister.hpp"
#include "CommandPackets/ReadMemory.hpp"
#include "CommandPackets/WriteMemory.hpp"
#include "CommandPackets/ReadMemoryMap.hpp"
#include "CommandPackets/FlashErase.hpp"
#include "CommandPackets/FlashWrite.hpp"
#include "CommandPackets/FlashDone.hpp"
#include "CommandPackets/VContSupportedActionsQuery.hpp"

#include "src/DebugServer/Gdb/CommandPackets/Monitor.hpp"

namespace DebugServer::Gdb::RiscVGdb
{
    using namespace Exceptions;

    using Targets::TargetRegisterDescriptor;
    using Targets::TargetRegisterType;

    RiscVGdbRsp::RiscVGdbRsp(
        const DebugServerConfig& debugServerConfig,
        const Targets::TargetDescriptor& targetDescriptor,
        EventListener& eventListener,
        EventFdNotifier& eventNotifier
    )
        : GdbRspDebugServer(
            debugServerConfig,
            targetDescriptor,
            RiscVGdbTargetDescriptor{targetDescriptor},
            eventListener,
            eventNotifier
        )
    {}

    std::string RiscVGdbRsp::getName() const {
        return "RISC-V GDB RSP Server";
    }

    std::unique_ptr<CommandPackets::RiscVGdbCommandPacketInterface> RiscVGdbRsp::rawPacketToCommandPacket(
        const RawPacket& rawPacket
    ) {
        using Gdb::CommandPackets::Monitor;

        using CommandPackets::ReadRegister;
        using CommandPackets::ReadRegisters;
        using CommandPackets::WriteRegister;
        using CommandPackets::ReadMemory;
        using CommandPackets::WriteMemory;
        using CommandPackets::ReadMemoryMap;
        using CommandPackets::FlashErase;
        using CommandPackets::FlashWrite;
        using CommandPackets::FlashDone;
        using CommandPackets::VContSupportedActionsQuery;

        if (rawPacket.size() < 2) {
            throw ::Exceptions::Exception{"Invalid raw packet - no data"};
        }

        if (rawPacket[1] == 'p') {
            return std::make_unique<ReadRegister>(rawPacket);
        }

        if (rawPacket[1] == 'g') {
            return std::make_unique<ReadRegisters>(rawPacket);
        }

        if (rawPacket[1] == 'P') {
            return std::make_unique<WriteRegister>(rawPacket);
        }

        if (rawPacket[1] == 'm') {
            return std::make_unique<ReadMemory>(rawPacket, this->gdbTargetDescriptor);
        }

        if (rawPacket[1] == 'M') {
            return std::make_unique<WriteMemory>(rawPacket, this->gdbTargetDescriptor);
        }

        if (rawPacket.size() > 1) {
            const auto rawPacketString = std::string{rawPacket.begin() + 1, rawPacket.end()};

            if (rawPacketString.find("qXfer:memory-map:read::") == 0) {
                return std::make_unique<ReadMemoryMap>(rawPacket);
            }

            if (rawPacketString.find("vFlashErase") == 0) {
                return std::make_unique<FlashErase>(rawPacket);
            }

            if (rawPacketString.find("vFlashWrite") == 0) {
                return std::make_unique<FlashWrite>(rawPacket);
            }

            if (rawPacketString.find("vFlashDone") == 0) {
                return std::make_unique<FlashDone>(rawPacket);
            }

            if (rawPacketString.find("vCont?") == 0) {
                return std::make_unique<VContSupportedActionsQuery>(rawPacket);
            }
        }

        return nullptr;
    }

    std::set<std::pair<Feature, std::optional<std::string>>> RiscVGdbRsp::getSupportedFeatures() {
        auto output = std::set<std::pair<Feature, std::optional<std::string>>>{
            {Feature::HARDWARE_BREAKPOINTS, std::nullopt},
            {Feature::SOFTWARE_BREAKPOINTS, std::nullopt},
            {Feature::MEMORY_MAP_READ, std::nullopt},
            {Feature::VCONT_ACTIONS_QUERY, std::nullopt},
        };

        if (!this->debugServerConfig.packetAcknowledgement) {
            output.emplace(Feature::NO_ACK_MODE, std::nullopt);
        }

        return output;
    }

    void RiscVGdbRsp::handleTargetStoppedGdbResponse(Targets::TargetMemoryAddress programAddress) {
        using Services::StringService;

        Logger::debug("Target stopped at byte address: 0x" + StringService::toHex(programAddress));

        // Report the stop to GDB
        return GdbRspDebugServer::handleTargetStoppedGdbResponse(programAddress);
    }
}
