#include "VContRangeStep.hpp"

#include <string>

#include "src/Services/Avr8InstructionService.hpp"
#include "src/Services/StringService.hpp"
#include "src/Services/PathService.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"
#include "src/Exceptions/Exception.hpp"
#include "src/Logger/Logger.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    using Services::TargetControllerService;

    using ResponsePackets::ErrorResponsePacket;

    using ::Exceptions::Exception;

    VContRangeStep::VContRangeStep(const RawPacket& rawPacket)
        : CommandPacket(rawPacket)
    {
        if (this->data.size() < 10) {
            throw Exception("Unexpected VContRangeStep packet size");
        }

        /*
         * A single vCont packet can consist of multiple actions - we don't accommodate this ATM. We only focus on
         * the first action in the packet.
         *
         * We also ignore the thread id (as it's not necessary here).
         */

        const auto commandData = std::string(this->data.begin() + 7, this->data.end());

        const auto delimiterPosition = commandData.find(',');
        const auto threadIdDelimiterPosition = commandData.find(':');

        if (delimiterPosition == std::string::npos || delimiterPosition >= (commandData.size() - 1)) {
            throw Exception("Invalid VContRangeStep packet");
        }

        const auto& delimiterIt = commandData.begin() + static_cast<decltype(commandData)::difference_type>(
                delimiterPosition
        );
        const auto startAddressHex = std::string(commandData.begin(), delimiterIt);
        const auto endAddressHex = std::string(
            delimiterIt + 1,
            threadIdDelimiterPosition != std::string::npos
                ? commandData.begin() + static_cast<decltype(commandData)::difference_type>(threadIdDelimiterPosition)
                : commandData.end()
        );

        this->startAddress = static_cast<Targets::TargetProgramCounter>(std::stoi(startAddressHex, nullptr, 16));
        this->endAddress = static_cast<Targets::TargetProgramCounter>(std::stoi(endAddressHex, nullptr, 16));
    }

    void VContRangeStep::handle(Gdb::DebugSession& debugSession, TargetControllerService& targetControllerService) {
        using Services::Avr8InstructionService;
        using Services::StringService;

        Logger::info("Handling VContRangeStep packet");

        Logger::debug("Requested stepping range start address: 0x" + StringService::toHex(this->startAddress));
        Logger::debug("Requested stepping range end address (exclusive): 0x" + StringService::toHex(this->endAddress));

        try {
            const auto& targetDescriptor = debugSession.gdbTargetDescriptor.targetDescriptor;
            const auto& programMemoryAddressRange = targetDescriptor.memoryDescriptorsByType.at(
                targetDescriptor.programMemoryType
            ).addressRange;

            if (
                this->startAddress >= this->endAddress
                || (this->startAddress % 2) != 0
                || (this->endAddress % 2) != 0
                || this->startAddress < programMemoryAddressRange.startAddress
                || this->endAddress > programMemoryAddressRange.endAddress
            ) {
                throw Exception("Invalid address range in VContRangeStep");
            }

            if (debugSession.activeRangeSteppingSession.has_value()) {
                Logger::warning(
                    "Attempted to start new range stepping session with one already active - terminating active session"
                );
                debugSession.terminateRangeSteppingSession(targetControllerService);
            }

            if ((this->endAddress - this->startAddress) == 2) {
                // Single step requested. No need for a range step here.
                targetControllerService.stepTargetExecution(std::nullopt);
                debugSession.waitingForBreak = true;
                return;
            }

            const auto addressRange = Targets::TargetMemoryAddressRange(this->startAddress, this->endAddress);
            auto rangeSteppingSession = RangeSteppingSession(addressRange, {});

            const auto instructionsByAddress = Avr8InstructionService::fetchInstructions(
                addressRange,
                targetDescriptor,
                targetControllerService
            );

            Logger::debug(
                "Inspecting " + std::to_string(instructionsByAddress.size()) + " instructions within stepping range "
                "(byte addresses) 0x" + StringService::toHex(addressRange.startAddress) + " -> 0x"
                + StringService::toHex(addressRange.endAddress) + ", in preparation for new range stepping session"
            );

            for (const auto& [instructionAddress, instruction] : instructionsByAddress) {
                if (!instruction.has_value()) {
                    /*
                     * We weren't able to decode the opcode at this address. We have no idea what this instruction
                     * will do.
                     */
                    Logger::error(
                        "Failed to decode AVR8 opcode at byte address 0x" + StringService::toHex(instructionAddress)
                        + " - the instruction will have to be intercepted. Please enable debug logging, reproduce "
                        "this message and report as an issue via " + Services::PathService::homeDomainName()
                        + "/report-issue"
                    );

                    /*
                     * We have no choice but to intercept it. When we reach it, we'll perform a single step and see
                     * what happens.
                     */
                    rangeSteppingSession.interceptedAddresses.insert(instructionAddress);
                    continue;
                }

                if (instruction->canChangeProgramFlow) {
                    const auto destinationAddress = Avr8InstructionService::resolveProgramDestinationAddress(
                        *instruction,
                        instructionAddress,
                        instructionsByAddress
                    );

                    if (!destinationAddress.has_value()) {
                        /*
                         * We don't know where this instruction may jump to, so we'll have to intercept it and perform
                         * a single step when we reach it.
                         */
                        Logger::debug(
                            "Intercepting CCPF instruction (\"" + instruction->name + "\") at byte address 0x"
                            + StringService::toHex(instructionAddress)
                        );
                        rangeSteppingSession.interceptedAddresses.insert(instructionAddress);
                        continue;
                    }

                    if (
                        *destinationAddress < programMemoryAddressRange.startAddress
                        || *destinationAddress > programMemoryAddressRange.endAddress
                    ) {
                        /*
                         * This instruction may jump to an invalid address. Someone screwed up here - could be
                         * something wrong in Bloom (opcode decoding bug, incorrect program memory address range in
                         * the target descriptor, etc.), or the user has an invalid instruction in their program code.
                         *
                         * We have no choice but to intercept the instruction. When we reach it, we'll perform a single
                         * step and see what happens.
                         */
                        Logger::debug(
                            "Intercepting CCPF instruction (\"" + instruction->name + "\") with invalid destination "
                            "byte address (0x" + StringService::toHex(*destinationAddress) + "), at byte address 0x"
                            + StringService::toHex(instructionAddress)
                        );
                        rangeSteppingSession.interceptedAddresses.insert(instructionAddress);
                        continue;
                    }

                    if (
                        *destinationAddress < addressRange.startAddress
                        || *destinationAddress >= addressRange.endAddress
                    ) {
                        /*
                         * This instruction may jump to an address outside the requested stepping range.
                         *
                         * Because we know exactly where it will jump to (if it jumps), we only need to intercept the
                         * destination address.
                         */
                        Logger::debug(
                            "Intercepting CCPF instruction (\"" + instruction->name + "\") at destination byte address "
                            "0x" + StringService::toHex(*destinationAddress)
                        );
                        rangeSteppingSession.interceptedAddresses.insert(*destinationAddress);
                    }
                }

                const auto subsequentInstructionAddress = instructionAddress + instruction->byteSize;
                if (subsequentInstructionAddress >= addressRange.endAddress) {
                    /*
                     * Once this instruction has been executed, we'll end up outside the stepping range (so we'll want
                     * to stop there and report back to GDB).
                     */
                    Logger::debug(
                        "Intercepting subsequent instruction at byte address 0x"
                        + StringService::toHex(subsequentInstructionAddress)
                    );
                    rangeSteppingSession.interceptedAddresses.insert(subsequentInstructionAddress);
                }
            }

            debugSession.startRangeSteppingSession(std::move(rangeSteppingSession), targetControllerService);
            targetControllerService.continueTargetExecution(std::nullopt, std::nullopt);
            debugSession.waitingForBreak = true;

        } catch (const Exception& exception) {
            Logger::error("Failed to start new range stepping session - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket());
        }
    }
}
