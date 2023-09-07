#include "Avr8InstructionService.hpp"

namespace Services
{
    using Targets::Microchip::Avr::Avr8Bit::OpcodeDecoder::Decoder;

    Decoder::InstructionMapping Avr8InstructionService::fetchInstructions(
        const Targets::TargetMemoryAddressRange& addressRange,
        const Targets::TargetDescriptor& targetDescriptor,
        TargetControllerService& targetControllerService
    ) {
        const auto programMemory = targetControllerService.readMemory(
            targetDescriptor.programMemoryType,
            addressRange.startAddress,
            addressRange.endAddress - addressRange.startAddress
        );

        return Decoder::decode(addressRange.startAddress, programMemory);
    }

    std::optional<Targets::TargetMemoryAddress> Avr8InstructionService::resolveProgramDestinationAddress(
        const Targets::Microchip::Avr::Avr8Bit::OpcodeDecoder::Instruction& instruction,
        Targets::TargetMemoryAddress instructionAddress,
        const Decoder::InstructionMapping& instructions
    ) {
        assert(instruction.canChangeProgramFlow);

        if (instruction.programWordAddress.has_value()) {
            return *(instruction.programWordAddress) * 2;
        }

        if (instruction.programWordAddressOffset.has_value()) {
            return static_cast<std::int64_t>(instructionAddress) + (*(instruction.programWordAddressOffset) * 2) + 2;
        }

        if (instruction.canSkipNextInstruction) {
            const auto subsequentInstructionAddress = instructionAddress + instruction.byteSize;
            const auto subsequentInstructionIt = instructions.find(subsequentInstructionAddress);

            if (subsequentInstructionIt != instructions.end() && subsequentInstructionIt->second.has_value()) {
                return subsequentInstructionAddress + subsequentInstructionIt->second->byteSize;
            }
        }

        return std::nullopt;
    }
}
