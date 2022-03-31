#include "ReadRegisters.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/TargetStopped.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"

#include "src/Targets/TargetRegister.hpp"

#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugServer::Gdb::CommandPackets
{
    using Targets::TargetRegister;
    using Targets::TargetRegisterDescriptors;

    using ResponsePackets::ResponsePacket;
    using ResponsePackets::ErrorResponsePacket;

    using Exceptions::Exception;

    void ReadRegisters::init() {
        if (this->data.size() >= 2 && this->data.front() == 'p') {
            // This command packet is requesting a specific register
            this->registerNumber = static_cast<size_t>(
                std::stoi(std::string(this->data.begin() + 1, this->data.end()))
            );
        }
    }

    void ReadRegisters::handle(DebugSession& debugSession, TargetControllerConsole& targetControllerConsole) {
        Logger::debug("Handling ReadRegisters packet");

        try {
            const auto& targetDescriptor = debugSession.targetDescriptor;
            auto descriptors = TargetRegisterDescriptors();

            if (this->registerNumber.has_value()) {
                Logger::debug("Reading register number: " + std::to_string(this->registerNumber.value()));
                descriptors.insert(
                    targetDescriptor.getTargetRegisterDescriptorFromNumber(this->registerNumber.value())
                );

            } else {
                // Read all target registers mapped to a GDB register
                for (const auto& registerNumber : targetDescriptor.getRegisterNumbers()) {
                    descriptors.insert(targetDescriptor.getTargetRegisterDescriptorFromNumber(registerNumber));
                }
            }

            auto registerSet = targetControllerConsole.readRegisters(descriptors);

            /*
             * Sort each register by their respective GDB register number - this will leave us with a collection of
             * registers in the order expected by the GDB client.
             */
            std::sort(
                registerSet.begin(),
                registerSet.end(),
                [this, &targetDescriptor] (const TargetRegister& registerA, const TargetRegister& registerB) {
                    return targetDescriptor.getRegisterNumberFromTargetRegisterDescriptor(registerA.descriptor) <
                        targetDescriptor.getRegisterNumberFromTargetRegisterDescriptor(registerB.descriptor);
                }
            );

            /*
             * Finally, reverse the register values (as they're all currently in MSB, but GDB expects them in LSB), ensure
             * that each register value size matches the size in the associated GDB register descriptor, implode the
             * values, convert to hexadecimal form and send to the GDB client.
             */
            auto registers = std::vector<unsigned char>();
            for (auto& reg : registerSet) {
                std::reverse(reg.value.begin(), reg.value.end());

                const auto gdbRegisterNumber = targetDescriptor.getRegisterNumberFromTargetRegisterDescriptor(
                    reg.descriptor
                ).value();
                const auto& gdbRegisterDescriptor = targetDescriptor.getRegisterDescriptorFromNumber(gdbRegisterNumber);

                if (reg.value.size() < gdbRegisterDescriptor.size) {
                    reg.value.insert(reg.value.end(), (gdbRegisterDescriptor.size - reg.value.size()), 0x00);
                }

                registers.insert(registers.end(), reg.value.begin(), reg.value.end());
            }

            auto responseRegisters = Packet::dataToHex(registers);
            debugSession.connection.writePacket(
                ResponsePacket(std::vector<unsigned char>(responseRegisters.begin(), responseRegisters.end()))
            );

        } catch (const Exception& exception) {
            Logger::error("Failed to read general registers - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket());
        }
    }
}
