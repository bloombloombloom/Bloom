#include "VContRangeStep.hpp"

#include <string>

#include "src/Targets/Microchip/AVR8/OpcodeDecoder/Decoder.hpp"
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
        using Services::StringService;

        if (this->data.size() < 10) {
            throw Exception{"Unexpected VContRangeStep packet size"};
        }

        const auto command = std::string{this->data.begin() + 7, this->data.end()};

        const auto delimiterPos = command.find(',');
        const auto threadIdDelimiterPos = command.find(':');
        if (delimiterPos == std::string::npos || delimiterPos >= (command.size() - 1)) {
            throw Exception{"Invalid VContRangeStep packet"};
        }

        this->startAddress = StringService::toUint32(command.substr(0, delimiterPos), 16);
        this->endAddress = StringService::toUint32(
            command.substr(delimiterPos + 1, threadIdDelimiterPos - (delimiterPos + 1)),
            16
        );
    }

    void VContRangeStep::handle(
        DebugSession& debugSession,
        const AvrGdbTargetDescriptor& gdbTargetDescriptor,
        const Targets::TargetDescriptor& targetDescriptor,
        TargetControllerService& targetControllerService
    ) {
        using Targets::Microchip::Avr8::OpcodeDecoder::Decoder;
        using Services::Avr8InstructionService;
        using Services::StringService;

        Logger::info("Handling VContRangeStep packet");

        Logger::debug("Requested stepping range start address: 0x" + StringService::toHex(this->startAddress));
        Logger::debug("Requested stepping range end address (exclusive): 0x" + StringService::toHex(this->endAddress));

        try {
            const auto stepAddressRange = Targets::TargetMemoryAddressRange{this->startAddress, this->endAddress};
            const auto stepByteSize = stepAddressRange.size() - 1; // -1 because the end address is exclusive
            const auto& programMemoryAddressRange = gdbTargetDescriptor.programMemorySegmentDescriptor.addressRange;

            if (
                stepAddressRange.startAddress > stepAddressRange.endAddress
                || (stepAddressRange.startAddress % 2) != 0
                || (stepAddressRange.endAddress % 2) != 0
                || !programMemoryAddressRange.contains(stepAddressRange)
            ) {
                throw Exception{"Invalid address range in VContRangeStep"};
            }

            if (debugSession.activeRangeSteppingSession.has_value()) {
                Logger::warning(
                    "Attempted to start new range stepping session with one already active - terminating active session"
                );
                debugSession.terminateRangeSteppingSession(targetControllerService);
            }

            if (stepByteSize <= 2) {
                // Single step requested. No need for a range step here.
                targetControllerService.stepTargetExecution();
                debugSession.waitingForBreak = true;
                return;
            }

            auto rangeSteppingSession = RangeSteppingSession{stepAddressRange, {}};

            const auto instructionsByAddress = Decoder::decode(
                stepAddressRange.startAddress,
                targetControllerService.readMemory(
                    gdbTargetDescriptor.programAddressSpaceDescriptor,
                    gdbTargetDescriptor.programMemorySegmentDescriptor,
                    stepAddressRange.startAddress,
                    stepByteSize
                )
            );

            Logger::debug(
                "Inspecting " + std::to_string(instructionsByAddress.size()) + " instruction(s) within stepping range "
                    "(byte addresses) 0x" + StringService::toHex(stepAddressRange.startAddress) + " -> 0x"
                    + StringService::toHex(stepAddressRange.endAddress) + ", in preparation for new range stepping "
                    "session"
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

                    if (!programMemoryAddressRange.contains(*destinationAddress)) {
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
                        *destinationAddress < stepAddressRange.startAddress
                        || *destinationAddress >= stepAddressRange.endAddress
                    ) {
                        /*
                         * This instruction may jump to an address outside the requested stepping range.
                         *
                         * Because we know exactly where it will jump to (if it jumps), we only need to intercept the
                         * destination address.
                         */
                        Logger::debug(
                            "Intercepting destination byte address 0x" + StringService::toHex(*destinationAddress)
                                + " of CCPF instruction (\"" + instruction->name + "\") at byte address 0x"
                                + StringService::toHex(instructionAddress)
                        );
                        rangeSteppingSession.interceptedAddresses.insert(*destinationAddress);
                    }
                }
            }

            /*
             * Finally, ensure that we intercept the first instruction outside the range (which is the end address
             * of the range, because it's exclusive).
             */
            rangeSteppingSession.interceptedAddresses.insert(stepAddressRange.endAddress);

            debugSession.startRangeSteppingSession(std::move(rangeSteppingSession), targetControllerService);

            /*
             * GDB expects us to start the range stepping session with a single step, and then only continue if the
             * single step didn't immediately take us out of the stepping range.
             *
             * So we kick off a single step here, then let AvrGdbRsp::handleTargetStoppedGdbResponse() determine if
             * we should continue. See that member function for more.
             */
            debugSession.activeRangeSteppingSession->singleStepping = true;
            targetControllerService.stepTargetExecution();
            debugSession.waitingForBreak = true;

        } catch (const Exception& exception) {
            Logger::error("Failed to start new range stepping session - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket{});
        }
    }
}
