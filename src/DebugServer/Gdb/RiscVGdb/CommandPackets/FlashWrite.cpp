#include "FlashWrite.hpp"

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

    FlashWrite::FlashWrite(const RawPacket& rawPacket)
        : CommandPacket(rawPacket)
    {
        using Services::StringService;

        if (this->data.size() < 15) {
            throw Exception{"Invalid packet length"};
        }

        /*
         * The flash write ('vFlashWrite') packet consists of two segments: an address and a buffer, seperated by a
         * colon.
         */
        const auto delimiterIt = std::find(this->data.begin() + 12, this->data.end(), ':');

        if (delimiterIt == this->data.end()) {
            throw Exception{"Failed to find colon delimiter in write flash packet."};
        }

        this->startAddress = StringService::toUint32(std::string{this->data.begin() + 12, delimiterIt}, 16);
        this->buffer = Targets::TargetMemoryBuffer{delimiterIt + 1, this->data.end()};
    }

    void FlashWrite::handle(
        Gdb::DebugSession& debugSession,
        const RiscVGdbTargetDescriptor& gdbTargetDescriptor,
        const Targets::TargetDescriptor& targetDescriptor,
        const Targets::TargetState& targetState,
        TargetControllerService& targetControllerService
    ) {
        Logger::info("Handling FlashWrite packet");

        try {
            if (this->buffer.empty()) {
                throw Exception{"Received empty buffer from GDB"};
            }

            if (!debugSession.programmingSession.has_value()) {
                debugSession.programmingSession = ProgrammingSession{this->startAddress, this->buffer};
                debugSession.connection.writePacket(OkResponsePacket{});
                return;
            }

            auto& programmingSession = debugSession.programmingSession.value();
            const auto currentEndAddress = programmingSession.startAddress + programmingSession.buffer.size() - 1;
            const auto expectedStartAddress = (currentEndAddress + 1);

            if (this->startAddress < expectedStartAddress) {
                throw Exception{"Invalid start address from GDB - the buffer would overlap a previous buffer"};
            }

            if (this->startAddress > expectedStartAddress) {
                // There is a gap in the buffer sent by GDB. Fill it with 0xFF
                programmingSession.buffer.insert(
                    programmingSession.buffer.end(),
                    this->startAddress - expectedStartAddress,
                    0xFF
                );
            }

            programmingSession.buffer.insert(
                programmingSession.buffer.end(),
                this->buffer.begin(),
                this->buffer.end()
            );

            debugSession.connection.writePacket(OkResponsePacket{});

        } catch (const Exception& exception) {
            Logger::error("Failed to handle FlashWrite packet - " + exception.getMessage());
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
