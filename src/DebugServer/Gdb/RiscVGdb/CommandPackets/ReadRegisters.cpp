#include "ReadRegisters.hpp"

#include <algorithm>
#include <iterator>
#include <cassert>

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"

#include "src/Targets/TargetRegisterDescriptor.hpp"
#include "src/Targets/TargetMemory.hpp"

#include "src/Logger/Logger.hpp"

#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::RiscVGdb::CommandPackets
{
    using Services::TargetControllerService;

    using ResponsePackets::ResponsePacket;
    using ResponsePackets::ErrorResponsePacket;

    using Exceptions::Exception;

    ReadRegisters::ReadRegisters(const RawPacket& rawPacket)
        : CommandPacket(rawPacket)
    {}

    void ReadRegisters::handle(
        Gdb::DebugSession& debugSession,
        const RiscVGdbTargetDescriptor& gdbTargetDescriptor,
        const Targets::TargetDescriptor& targetDescriptor,
        const Targets::TargetState& targetState,
        TargetControllerService& targetControllerService
    ) {
        Logger::info("Handling ReadRegisters packet");

        try {
            const auto totalRegBytes = (gdbTargetDescriptor.targetRegisterDescriptorsByGdbId.size() + 1) * 4;
            auto buffer = Targets::TargetMemoryBuffer(totalRegBytes, 0x00);

            auto gpRegDescriptors = Targets::TargetRegisterDescriptors{};
            std::transform(
                gdbTargetDescriptor.targetRegisterDescriptorsByGdbId.begin(),
                gdbTargetDescriptor.targetRegisterDescriptorsByGdbId.end(),
                std::back_inserter(gpRegDescriptors),
                [] (const auto& pair) {
                    return pair.second;
                }
            );

            {
                const auto atomicSession = targetControllerService.makeAtomicSession();

                for (auto& [regDesc, regVal] : targetControllerService.readRegisters(gpRegDescriptors)) {
                    const auto bufferOffset = (
                        regDesc.startAddress - gdbTargetDescriptor.gpRegistersMemorySegmentDescriptor.addressRange.startAddress
                    ) * gdbTargetDescriptor.gpRegistersMemorySegmentDescriptor.addressSpaceUnitSize;

                    assert((buffer.size() - bufferOffset) >= regVal.size());

                    // GDB expects register values in LSB form, which is why we use reverse iterators below.
                    std::copy(regVal.rbegin(), regVal.rend(), buffer.begin() + bufferOffset);
                }

                const auto pcValue = targetState.programCounter.load().value();
                buffer[totalRegBytes - 4] = static_cast<unsigned char>(pcValue);
                buffer[totalRegBytes - 3] = static_cast<unsigned char>(pcValue >> 8);
                buffer[totalRegBytes - 2] = static_cast<unsigned char>(pcValue >> 16);
                buffer[totalRegBytes - 1] = static_cast<unsigned char>(pcValue >> 24);
            }

            debugSession.connection.writePacket(ResponsePacket{Services::StringService::toHex(buffer)});

        } catch (const Exception& exception) {
            Logger::error("Failed to read registers - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket{});
        }
    }
}
