#include "ReadRegisters.hpp"

#include <algorithm>
#include <iterator>
#include <cassert>

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"

#include "src/Targets/TargetRegisterDescriptor.hpp"
#include "src/Targets/TargetMemory.hpp"

#include "src/Logger/Logger.hpp"

#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    using Services::TargetControllerService;

    using ResponsePackets::ResponsePacket;
    using ResponsePackets::ErrorResponsePacket;

    using Exceptions::Exception;

    ReadRegisters::ReadRegisters(const RawPacket& rawPacket)
        : CommandPacket(rawPacket)
    {}

    void ReadRegisters::handle(
        DebugSession& debugSession,
        const AvrGdbTargetDescriptor& gdbTargetDescriptor,
        const Targets::TargetDescriptor& targetDescriptor,
        TargetControllerService& targetControllerService
    ) {
        Logger::info("Handling ReadRegisters packet");

        try {
            auto buffer = Targets::TargetMemoryBuffer(39, 0x00);

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
                    if (regDesc.type != Targets::TargetRegisterType::GENERAL_PURPOSE_REGISTER) {
                        // Status register (SREG)
                        assert(regVal.size() == 1);
                        buffer[32] = regVal[0];
                        continue;
                    }

                    const auto bufferOffset = regDesc.startAddress
                        - gdbTargetDescriptor.gpRegistersMemorySegmentDescriptor.addressRange.startAddress;

                    assert((buffer.size() - bufferOffset) >= regVal.size());

                    /*
                     * GDB expects register values in LSB form, which is why we use reverse iterators below.
                     *
                     * This isn't really necessary though, as all of the registers that are handled here are
                     * single-byte registers.
                     */
                    std::copy(regVal.rbegin(), regVal.rend(), buffer.begin() + bufferOffset);
                }

                const auto spValue = targetControllerService.getStackPointer();
                buffer[33] = static_cast<unsigned char>(spValue);
                buffer[34] = static_cast<unsigned char>(spValue >> 8);

                const auto pcValue = targetControllerService.getProgramCounter();
                buffer[35] = static_cast<unsigned char>(pcValue);
                buffer[36] = static_cast<unsigned char>(pcValue >> 8);
                buffer[37] = static_cast<unsigned char>(pcValue >> 16);
                buffer[38] = static_cast<unsigned char>(pcValue >> 24);
            }

            debugSession.connection.writePacket(ResponsePacket{Services::StringService::toHex(buffer)});

        } catch (const Exception& exception) {
            Logger::error("Failed to read registers - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket{});
        }
    }
}
