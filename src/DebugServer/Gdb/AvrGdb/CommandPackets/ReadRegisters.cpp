#include "ReadRegisters.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"
#include "src/DebugServer/Gdb/AvrGdb/TargetDescriptor.hpp"

#include "src/Targets/TargetRegister.hpp"

#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"

#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugServer::Gdb::AvrGdb::CommandPackets
{
    using Services::TargetControllerService;

    using Targets::TargetRegister;
    using Targets::TargetRegisterDescriptorIds;

    using ResponsePackets::ResponsePacket;
    using ResponsePackets::ErrorResponsePacket;

    using Exceptions::Exception;

    ReadRegisters::ReadRegisters(const RawPacket& rawPacket)
        : CommandPacket(rawPacket)
    {}

    void ReadRegisters::handle(DebugSession& debugSession, TargetControllerService& targetControllerService) {
        Logger::info("Handling ReadRegisters packet");

        try {
            const auto& targetDescriptor = debugSession.gdbTargetDescriptor;
            auto descriptorIds = TargetRegisterDescriptorIds();

            // Read all target registers mapped to a GDB register
            for (const auto& [gdbRegisterId, gdbRegisterDescriptor] : targetDescriptor.gdbRegisterDescriptorsById) {
                const auto registerDescriptorId = targetDescriptor.getTargetRegisterDescriptorIdFromGdbRegisterId(
                    gdbRegisterId
                );

                if (registerDescriptorId.has_value()) {
                    descriptorIds.insert(*registerDescriptorId);
                }
            }

            auto registerSet = targetControllerService.readRegisters(descriptorIds);

            /*
             * Sort each register by their respective GDB register ID - this will leave us with a collection of
             * registers in the order expected by the GDB client.
             */
            std::sort(
                registerSet.begin(),
                registerSet.end(),
                [this, &targetDescriptor] (const TargetRegister& regA, const TargetRegister& regB) {
                    return targetDescriptor.getGdbRegisterIdFromTargetRegisterDescriptorId(regA.descriptorId).value() <
                        targetDescriptor.getGdbRegisterIdFromTargetRegisterDescriptorId(regB.descriptorId).value();
                }
            );

            /*
             * Reverse the register values (as they're all currently in MSB, but GDB expects them in LSB), ensure that
             * each register value size matches the size in the associated GDB register descriptor and implode the
             * values.
             */
            auto registers = std::vector<unsigned char>();
            for (auto& reg : registerSet) {
                std::reverse(reg.value.begin(), reg.value.end());

                const auto gdbRegisterId = targetDescriptor.getGdbRegisterIdFromTargetRegisterDescriptorId(
                    reg.descriptorId
                ).value();
                const auto& gdbRegisterDescriptor = targetDescriptor.gdbRegisterDescriptorsById.at(gdbRegisterId);

                if (reg.value.size() < gdbRegisterDescriptor.size) {
                    reg.value.insert(reg.value.end(), (gdbRegisterDescriptor.size - reg.value.size()), 0x00);
                }

                registers.insert(registers.end(), reg.value.begin(), reg.value.end());
            }

            // Finally, include the program counter (which GDB expects to reside at the end)
            const auto programCounter = targetControllerService.getProgramCounter();
            registers.insert(registers.end(), static_cast<unsigned char>(programCounter));
            registers.insert(registers.end(), static_cast<unsigned char>(programCounter >> 8));
            registers.insert(registers.end(), static_cast<unsigned char>(programCounter >> 16));
            registers.insert(registers.end(), static_cast<unsigned char>(programCounter >> 24));

            debugSession.connection.writePacket(
                ResponsePacket(Services::StringService::toHex(registers))
            );

        } catch (const Exception& exception) {
            Logger::error("Failed to read registers - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket());
        }
    }
}
