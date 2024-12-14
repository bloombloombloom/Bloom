#include "FlashErase.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/OkResponsePacket.hpp"

#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::RiscVGdb::CommandPackets
{
    using Services::TargetControllerService;

    using ResponsePackets::ErrorResponsePacket;
    using ResponsePackets::OkResponsePacket;

    using namespace Exceptions;

    FlashErase::FlashErase(const RawPacket& rawPacket)
        : CommandPacket(rawPacket)
    {
        using Services::StringService;

        if (rawPacket.size() < 8) {
            throw Exception{"Invalid packet length"};
        }

        const auto command = std::string{this->data.begin() + 12, this->data.end()};

        const auto delimiterPos = command.find_first_of(',');
        if (delimiterPos == std::string::npos) {
            throw Exception{"Invalid packet"};
        }

        this->startAddress = StringService::toUint32(command.substr(0, delimiterPos), 16);
        this->bytes = StringService::toUint32(command.substr(delimiterPos + 1), 16);
    }

    void FlashErase::handle(
        Gdb::DebugSession& debugSession,
        const RiscVGdbTargetDescriptor& gdbTargetDescriptor,
        const Targets::TargetDescriptor& targetDescriptor,
        const Targets::TargetState& targetState,
        TargetControllerService& targetControllerService
    ) {
        Logger::info("Handling FlashErase packet");

        try {
            const auto segmentDescriptorOpt = gdbTargetDescriptor.systemAddressSpaceDescriptor.getContainingMemorySegmentDescriptor(
                this->startAddress
            );

            if (!segmentDescriptorOpt.has_value()) {
                throw Exception{"Invalid command - no containing memory segment found for the given start address"};
            }

            const auto& segmentDescriptor = segmentDescriptorOpt->get();
            if (!segmentDescriptor.programmingModeAccess.writeable) {
                throw Exception{"Memory segment (\"" + segmentDescriptor.name + "\") not writable in programming mode"};
            }

            Logger::warning("Erasing \"" + segmentDescriptor.name + "\" segment, in preparation for programming");

            targetControllerService.enableProgrammingMode();

            // We don't erase a specific address range - we just erase the entire segment
            targetControllerService.eraseMemory(
                gdbTargetDescriptor.systemAddressSpaceDescriptor,
                segmentDescriptor
            );

            debugSession.connection.writePacket(OkResponsePacket{});

        } catch (const Exception& exception) {
            Logger::error("Failed to erase flash memory - " + exception.getMessage());
            debugSession.programmingSession.reset();

            try {
                targetControllerService.disableProgrammingMode();

            } catch (const Exception& exception) {
                Logger::error("Failed to disable programming mode - " + exception.getMessage());
            }

            debugSession.connection.writePacket(ErrorResponsePacket{});
        }
    }
}
