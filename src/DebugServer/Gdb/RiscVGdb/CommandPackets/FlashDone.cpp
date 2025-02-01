#include "FlashDone.hpp"

#include "src/Targets/TargetMemoryAddressRange.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/OkResponsePacket.hpp"

#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::RiscVGdb::CommandPackets
{
    using Services::TargetControllerService;

    using ResponsePackets::ErrorResponsePacket;
    using ResponsePackets::OkResponsePacket;

    using namespace Exceptions;

    FlashDone::FlashDone(const RawPacket& rawPacket)
        : CommandPacket(rawPacket)
    {}

    void FlashDone::handle(
        Gdb::DebugSession& debugSession,
        const RiscVGdbTargetDescriptor& gdbTargetDescriptor,
        const Targets::TargetDescriptor& targetDescriptor,
        const Targets::TargetState& targetState,
        TargetControllerService& targetControllerService
    ) {
        Logger::info("Handling FlashDone packet");

        try {
            if (!debugSession.programmingSession.has_value()) {
                /*
                 * GDB will send a VFlashDone packet even it only performs an erase. In this case, there's nothing more
                 * to do, as erase operations are executed immediately.
                 */
                targetControllerService.disableProgrammingMode();
                debugSession.connection.writePacket(OkResponsePacket{});
                return;
            }

            Logger::info(
                "Flushing " + std::to_string(debugSession.programmingSession->buffer.size())
                    + " bytes to target's program memory"
            );

            const auto addressRange = Targets::TargetMemoryAddressRange{
                debugSession.programmingSession->startAddress,
                debugSession.programmingSession->startAddress
                    + static_cast<Targets::TargetMemorySize>(debugSession.programmingSession->buffer.size()) - 1
            };

            const auto memorySegmentDescriptors = gdbTargetDescriptor.systemAddressSpaceDescriptor.getIntersectingMemorySegmentDescriptors(
                addressRange
            );

            if (memorySegmentDescriptors.size() != 1) {
                throw Exception{
                    memorySegmentDescriptors.empty()
                        ? "Invalid command - no containing memory segments found for the given address range"
                        : "Invalid command - address range intersects multiple memory segments"
                };
            }

            const auto& segmentDescriptor = *(memorySegmentDescriptors.front());
            if (!segmentDescriptor.programmingModeAccess.writeable) {
                throw Exception{"Memory segment (\"" + segmentDescriptor.name + "\") not writable in programming mode"};
            }

            targetControllerService.enableProgrammingMode();

            targetControllerService.writeMemory(
                gdbTargetDescriptor.systemAddressSpaceDescriptor,
                segmentDescriptor,
                debugSession.programmingSession->startAddress,
                std::move(debugSession.programmingSession->buffer)
            );

            debugSession.programmingSession.reset();

            targetControllerService.disableProgrammingMode();
            Logger::warning("Program memory updated");

            Logger::warning("Resetting target");
            targetControllerService.resetTarget();
            Logger::info("Target reset complete");

            debugSession.connection.writePacket(OkResponsePacket{});

        } catch (const Exception& exception) {
            Logger::error("Failed to handle FlashDone packet - " + exception.getMessage());
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
