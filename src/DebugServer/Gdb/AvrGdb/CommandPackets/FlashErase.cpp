#include "FlashErase.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/OkResponsePacket.hpp"

#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
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

        /*
         * The flash erase ('vFlashErase') packet consists of two segments, an address and a length, separated by a
         * comma.
         *
         * Example: $vFlashErase:00000000,00004f00#f4
         */
        const auto command = std::string{this->data.begin() + 12, this->data.end()};

        const auto delimiterPos = command.find_first_of(',');
        if (delimiterPos == std::string::npos) {
            throw Exception{"Invalid packet"};
        }

        this->startAddress = StringService::toUint32(command.substr(0, delimiterPos), 16);
        this->bytes = StringService::toUint32(command.substr(delimiterPos + 1), 16);
    }

    void FlashErase::handle(
        DebugSession& debugSession,
        const AvrGdbTargetDescriptor& gdbTargetDescriptor,
        const Targets::TargetDescriptor& targetDescriptor,
        TargetControllerService& targetControllerService
    ) {
        Logger::info("Handling FlashErase packet");

        try {
            targetControllerService.enableProgrammingMode();

            Logger::warning("Erasing program memory, in preparation for programming");

            // We don't erase a specific address range - we just erase the entire program memory.
            targetControllerService.eraseMemory(
                gdbTargetDescriptor.programAddressSpaceDescriptor,
                gdbTargetDescriptor.programMemorySegmentDescriptor
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
