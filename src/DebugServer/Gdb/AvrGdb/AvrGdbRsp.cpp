#include "AvrGdbRsp.hpp"

// Command packets
#include "CommandPackets/ReadMemory.hpp"
#include "CommandPackets/WriteMemory.hpp"
#include "CommandPackets/ReadMemoryMap.hpp"

namespace Bloom::DebugServer::Gdb::AvrGdb
{
    using namespace Bloom::Exceptions;

    using Bloom::Targets::TargetRegisterDescriptor;
    using Bloom::Targets::TargetRegisterType;

    AvrGdbRsp::AvrGdbRsp(
        const DebugServerConfig& debugServerConfig,
        EventListener& eventListener,
        EventFdNotifier& eventNotifier
    )
        : GdbRspDebugServer(debugServerConfig, eventListener, eventNotifier)
    {}

    void AvrGdbRsp::init() {
        DebugServer::Gdb::GdbRspDebugServer::init();

        this->gdbTargetDescriptor = TargetDescriptor(
            this->targetControllerConsole.getTargetDescriptor()
        );
    }

    std::unique_ptr<Gdb::CommandPackets::CommandPacket> AvrGdbRsp::resolveCommandPacket(
        const RawPacketType& rawPacket
    ) {
        using AvrGdb::CommandPackets::ReadMemory;
        using AvrGdb::CommandPackets::WriteMemory;
        using AvrGdb::CommandPackets::ReadMemoryMap;

        if (rawPacket.size() >= 2) {
            if (rawPacket[1] == 'm') {
                return std::make_unique<ReadMemory>(rawPacket);
            }

            if (rawPacket[1] == 'M') {
                return std::make_unique<WriteMemory>(rawPacket);
            }

            const auto rawPacketString = std::string(rawPacket.begin() + 1, rawPacket.end());

            if (rawPacketString.find("qXfer:memory-map:read::") == 0) {
                return std::make_unique<ReadMemoryMap>(rawPacket);
            }
        }

        return GdbRspDebugServer::resolveCommandPacket(rawPacket);
    }

    std::set<std::pair<Feature, std::optional<std::string>>> AvrGdbRsp::getSupportedFeatures() {
        auto supportedFeatures = GdbRspDebugServer::getSupportedFeatures();

        // The AVR GDB server supports the
        supportedFeatures.insert({
            Feature::MEMORY_MAP_READ, std::nullopt
        });

        return supportedFeatures;
    }
}
