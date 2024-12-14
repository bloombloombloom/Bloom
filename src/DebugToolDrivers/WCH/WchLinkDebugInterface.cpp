#include "WchLinkDebugInterface.hpp"

#include <array>

#include "Protocols/WchLink/Commands/Control/AttachTarget.hpp"
#include "Protocols/WchLink/Commands/Control/DetachTarget.hpp"
#include "Protocols/WchLink/Commands/Control/PostAttach.hpp"
#include "Protocols/WchLink/Commands/Control/GetDeviceInfo.hpp"
#include "Protocols/WchLink/Commands/DebugModuleInterfaceOperation.hpp"

#include "Protocols/WchLink/FlashProgramOpcodes.hpp"

#include "src/Targets/RiscV/Opcodes/Opcode.hpp"

#include "src/Services/StringService.hpp"

#include "src/Exceptions/InternalFatalErrorException.hpp"
#include "src/TargetController/Exceptions/TargetOperationFailure.hpp"
#include "src/Targets/TargetDescription/Exceptions/InvalidTargetDescriptionDataException.hpp"

#include "src/Logger/Logger.hpp"

namespace DebugToolDrivers::Wch
{
    using ::Targets::TargetExecutionState;
    using ::Targets::TargetMemoryAddress;
    using ::Targets::TargetMemoryAddressRange;
    using ::Targets::TargetMemorySize;
    using ::Targets::TargetMemoryBuffer;
    using ::Targets::TargetMemoryBufferSpan;
    using ::Targets::TargetStackPointer;
    using ::Targets::TargetAddressSpaceDescriptor;
    using ::Targets::TargetMemorySegmentDescriptor;
    using ::Targets::TargetProgramBreakpoint;
    using ::Targets::TargetMemorySegmentType;
    using ::Targets::TargetRegisterDescriptors;
    using ::Targets::TargetRegisterDescriptorAndValuePairs;

    using namespace Protocols::WchLink;
    using namespace ::Exceptions;

    WchLinkDebugInterface::WchLinkDebugInterface(
        const WchLinkToolConfig& toolConfig,
        const Targets::RiscV::RiscVTargetConfig& targetConfig,
        const Targets::RiscV::TargetDescriptionFile& targetDescriptionFile,
        Protocols::WchLink::WchLinkInterface& wchLinkInterface
    )
        : toolConfig(toolConfig)
        , targetConfig(targetConfig)
        , targetDescriptionFile(targetDescriptionFile)
        , wchLinkInterface(wchLinkInterface)
        , riscVTranslator(
            ::DebugToolDrivers::Protocols::RiscVDebugSpec::DebugTranslator{
                this->wchLinkInterface,
                this->toolConfig.riscVDebugTranslatorConfig,
                this->targetDescriptionFile,
                this->targetConfig
            }
        )
        , programSegmentDescriptor(
            this->targetDescriptionFile.getSystemAddressSpaceDescriptor().getMemorySegmentDescriptor("main_program")
        )
        , flashProgramOpcodes(
            WchLinkDebugInterface::getFlashProgramOpcodes(
                this->targetDescriptionFile.getProperty("wch_link_interface", "programming_opcode_key").value
            )
        )
        , programmingBlockSize(
            Services::StringService::toUint32(
                this->targetDescriptionFile.getProperty("wch_link_interface", "programming_block_size").value
            )
        )
    {}

    void WchLinkDebugInterface::activate() {
        this->wchLinkInterface.setClockSpeed(
            WchLinkTargetClockSpeed::CLK_6000_KHZ,
            this->cachedTargetId.value_or(0x01)
        );

        auto response = this->wchLinkInterface.sendCommandAndWaitForResponse(Commands::Control::AttachTarget{});
        if (response.payload.size() != 5) {
            throw Exceptions::DeviceCommunicationFailure{"Unexpected response payload size for AttachTarget command"};
        }

        this->cachedTargetId = response.payload[0];

        /*
         * For some WCH targets, we must send another command to the debug tool, immediately after attaching.
         *
         * I don't know what this post-attach command does. But what I *do* know is that the target and/or the debug
         * tool will misbehave if we don't send it immediately after the attach.
         *
         * More specifically, the debug tool will read an invalid target variant ID upon the mutation of the target's
         * program buffer. So when we write to progbuf2, progbuf3, progbuf4 or progbuf5, all subsequent reads of the
         * target variant ID will yield invalid values, until the target and debug tool have been power cycled.
         * Interestingly, when we restore those progbuf registers to their original values, the reading of the target
         * variant ID works again. So I suspect the debug tool is using the target's program buffer to read the
         * variant ID, but it's assuming the program buffer hasn't changed. Maybe.
         *
         * So how does this post-attach command fix this issue? I don't know. I just know that it does.
         *
         * In addition to sending the post-attach command, we have to send another attach command, because the target
         * variant ID returned in the response of the first attach command may be invalid. Sending another attach
         * command will ensure that we have a valid target variant ID.
         *
         * TODO: Add a property to the target's TDF, to determine whether the post-attach is required, instead of
         *       hardcoding target IDs here. This can be done after v2.0.0.
         */
        if (this->cachedTargetId == 0x09) {
            this->wchLinkInterface.sendCommandAndWaitForResponse(Commands::Control::PostAttach{});
            response = this->wchLinkInterface.sendCommandAndWaitForResponse(Commands::Control::AttachTarget{});

            if (response.payload.size() != 5) {
                throw Exceptions::DeviceCommunicationFailure{
                    "Unexpected response payload size for subsequent AttachTarget command"
                };
            }
        }

        this->cachedVariantId = static_cast<WchTargetVariantId>(
            (response.payload[1] << 24) | (response.payload[2] << 16) | (response.payload[3] << 8)
                | (response.payload[4])
        );

        this->riscVTranslator.activate();
    }

    void WchLinkDebugInterface::deactivate() {
        this->riscVTranslator.clearAllTriggers();
        this->riscVTranslator.deactivate();

        const auto response = this->wchLinkInterface.sendCommandAndWaitForResponse(Commands::Control::DetachTarget{});
        if (response.payload.size() != 1) {
            throw Exceptions::DeviceCommunicationFailure{"Unexpected response payload size for DetachTarget command"};
        }
    }

    std::string WchLinkDebugInterface::getDeviceId() {
        return "0x" + Services::StringService::toHex(this->cachedVariantId.value());
    }

    Targets::TargetExecutionState WchLinkDebugInterface::getExecutionState() {
        return this->riscVTranslator.getExecutionState();
    }

    void WchLinkDebugInterface::stop() {
        this->riscVTranslator.stop();
    }

    void WchLinkDebugInterface::run() {
        this->riscVTranslator.run();
    }

    void WchLinkDebugInterface::step() {
        this->riscVTranslator.step();
    }

    void WchLinkDebugInterface::reset() {
        this->riscVTranslator.reset();
    }

    Targets::BreakpointResources WchLinkDebugInterface::getBreakpointResources() {
        return {
            .hardwareBreakpoints = this->riscVTranslator.getTriggerCount(),
            .softwareBreakpoints = 0xFFFFFFFF, // TODO: Use the program memory size to determine the limit.
        };
    }

    void WchLinkDebugInterface::setProgramBreakpoint(const TargetProgramBreakpoint& breakpoint) {
        if (breakpoint.type == TargetProgramBreakpoint::Type::HARDWARE) {
            this->riscVTranslator.insertTriggerBreakpoint(breakpoint.address);

        } else {
            this->setSoftwareBreakpoint(breakpoint);
        }
    }

    void WchLinkDebugInterface::removeProgramBreakpoint(const TargetProgramBreakpoint& breakpoint) {
        if (breakpoint.type == TargetProgramBreakpoint::Type::HARDWARE) {
            this->riscVTranslator.clearTriggerBreakpoint(breakpoint.address);

        } else {
            this->clearSoftwareBreakpoint(breakpoint);
        }
    }

    Targets::TargetRegisterDescriptorAndValuePairs WchLinkDebugInterface::readCpuRegisters(
        const Targets::TargetRegisterDescriptors& descriptors
    ) {
        return this->riscVTranslator.readCpuRegisters(descriptors);
    }

    void WchLinkDebugInterface::writeCpuRegisters(const Targets::TargetRegisterDescriptorAndValuePairs& registers) {
        return this->riscVTranslator.writeCpuRegisters(registers);
    }

    Targets::TargetMemoryBuffer WchLinkDebugInterface::readMemory(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        Targets::TargetMemoryAddress startAddress,
        Targets::TargetMemorySize bytes,
        const std::set<Targets::TargetMemoryAddressRange>& excludedAddressRanges
    ) {
        return this->riscVTranslator.readMemory(
            addressSpaceDescriptor,
            memorySegmentDescriptor,
            startAddress,
            bytes,
            excludedAddressRanges
        );
    }

    void WchLinkDebugInterface::writeMemory(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        Targets::TargetMemoryAddress startAddress,
        Targets::TargetMemoryBufferSpan buffer
    ) {
        if (memorySegmentDescriptor.type == TargetMemorySegmentType::FLASH) {
            /*
             * WCH-Link tools cannot write to flash memory via the target's debug module. They do, however, offer a
             * set of dedicated commands for this. We invoke them here.
             *
             * There are two commands we can choose from:
             *
             * - Partial block write
             *     Writes any number of bytes to flash, but limited to a maximum of 64 bytes per write. Larger writes
             *     must be split into multiple writes.
             * - Full block write
             *     Writes an entire block to flash, where the block size is target-specific (resides in the target's
             *     TDF). Requires alignment to the block size. Requires reattaching to the target at the end of the
             *     programming session.
             *
             * The full block write is much faster for writing large buffers (KiBs), such as when we're programming
             * the target. But the partial block write is faster and more suitable for writing buffers that are
             * smaller than 64 bytes, such as when we're inserting software breakpoints.
             */
            const auto bufferSize = static_cast<TargetMemorySize>(buffer.size());
            const auto alignmentSize = this->programmingBlockSize;
            const auto alignedStartAddress = (startAddress / alignmentSize) * alignmentSize;
            const auto alignedBufferSize = static_cast<TargetMemorySize>(std::ceil(
                static_cast<double>(bufferSize) / static_cast<double>(alignmentSize)
            ) * alignmentSize);
            const auto alignmentRequired = alignedStartAddress != startAddress || alignedBufferSize != bufferSize;

            if (
                bufferSize <= WchLinkInterface::MAX_PARTIAL_BLOCK_WRITE_SIZE
                || (
                    alignmentRequired
                    && !memorySegmentDescriptor.addressRange.contains(
                        TargetMemoryAddressRange{
                            alignedStartAddress,
                            alignedStartAddress + alignedBufferSize - 1
                        }
                    )
                )
            ) {
                using namespace ::DebugToolDrivers::Protocols::RiscVDebugSpec;
                Logger::debug("Using partial block write command");

                /*
                 * WCH-Link tools seem to make use of the target's program buffer to service the partial block write
                 * command.
                 *
                 * This sometimes leads to exceptions occurring on the target, when the program buffer contains certain
                 * instructions before the partial block write command is invoked. This is why we clear the program
                 * buffer beforehand.
                 */
                this->riscVTranslator.clearProgramBuffer();
                this->wchLinkInterface.writeFlashPartialBlock(startAddress, buffer);

                const auto commandError = this->riscVTranslator.readAndClearAbstractCommandError();
                if (commandError != DebugModule::AbstractCommandError::NONE) {
                    throw Exceptions::Exception{
                        "Partial block write failed - abstract command error: 0x"
                            + Services::StringService::toHex(commandError)
                    };
                }

                return;
            }

            if (alignmentRequired) {
                auto alignedBuffer = (alignedStartAddress < startAddress)
                    ? this->readMemory(
                        addressSpaceDescriptor,
                        memorySegmentDescriptor,
                        alignedStartAddress,
                        (startAddress - alignedStartAddress),
                        {}
                    )
                    : TargetMemoryBuffer{};

                alignedBuffer.resize(alignedBufferSize);

                std::copy(
                    buffer.begin(),
                    buffer.end(),
                    alignedBuffer.begin() + (startAddress - alignedStartAddress)
                );

                const auto dataBack = this->readMemory(
                    addressSpaceDescriptor,
                    memorySegmentDescriptor,
                    startAddress + bufferSize,
                    alignedBufferSize - bufferSize - (startAddress - alignedStartAddress),
                    {}
                );
                std::copy(
                    dataBack.begin(),
                    dataBack.end(),
                    alignedBuffer.begin() + (startAddress - alignedStartAddress) + bufferSize
                );

                return this->writeMemory(
                    addressSpaceDescriptor,
                    memorySegmentDescriptor,
                    alignedStartAddress,
                    alignedBuffer
                );
            }

            Logger::debug(
                "Using full block write command (block size: " + std::to_string(this->programmingBlockSize) + ")"
            );
            this->wchLinkInterface.writeFlashFullBlocks(
                startAddress,
                buffer,
                this->programmingBlockSize,
                this->flashProgramOpcodes
            );

            /*
             * Would this not be better placed in endProgrammingSession()? We could persist the command type we invoked
             * to perform the write, and if required, reattach at the end of the programming session.
             *
             * I don't think that would work, because the target needs to be accessible for other operations whilst in
             * programming mode. We may perform other operations in between program memory writes, but that wouldn't
             * work if we left the target in an inaccessible state between writes. So I think we have to reattach here.
             *
             * TODO: Review after v2.0.0.
             */
            this->deactivate();
            this->wchLinkInterface.sendCommandAndWaitForResponse(Commands::Control::GetDeviceInfo{});
            this->activate();
            return;
        }

        this->riscVTranslator.writeMemory(
            addressSpaceDescriptor,
            memorySegmentDescriptor,
            startAddress,
            buffer
        );
    }

    void WchLinkDebugInterface::eraseMemory(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor
    ) {
        if (memorySegmentDescriptor == this->programSegmentDescriptor) {
            return this->wchLinkInterface.eraseProgramMemory();
        }

        // Ignore other (non-program memory) erase requests, for now.
    }

    void WchLinkDebugInterface::enableProgrammingMode() {
        /*
         * Nothing to do here
         *
         * We cannot prepare the WCH-Link tool for a programming session here, as the preparation process differs
         * across the two types of flash write commands (full and partial block write). We don't know which command
         * we'll be utilising at this point, so we perform the preparation in writeMemory().
         */
    }

    void WchLinkDebugInterface::disableProgrammingMode() {
        this->softwareBreakpointRegistry.clear();
    }

    void WchLinkDebugInterface::applyAccessRestrictions(TargetMemorySegmentDescriptor& memorySegmentDescriptor) {
        if (memorySegmentDescriptor.type == TargetMemorySegmentType::FLASH) {
            /*
             * Actually, we *can* write to flash memory whilst in debug mode (via a partial block write), but we don't
             * need to, so I'm just going to block it, for now.
             */
            memorySegmentDescriptor.debugModeAccess.writeable = false;
        }
    }

    void WchLinkDebugInterface::applyAccessRestrictions(Targets::TargetRegisterDescriptor& registerDescriptor) {
        // I don't believe any further access restrictions are required for registers. TODO: Review after v2.0.0.
    }

    void WchLinkDebugInterface::setSoftwareBreakpoint(const TargetProgramBreakpoint& breakpoint) {
        if (breakpoint.size != 2 && breakpoint.size != 4) {
            throw Exception{"Invalid software breakpoint size (" + std::to_string(breakpoint.size) + ")"};
        }

        const auto originalData = this->readMemory(
            breakpoint.addressSpaceDescriptor,
            breakpoint.memorySegmentDescriptor,
            breakpoint.address,
            breakpoint.size,
            {}
        );

        const auto softwareBreakpoint = ::Targets::RiscV::ProgramBreakpoint{
            breakpoint,
            static_cast<::Targets::RiscV::Opcodes::Opcode>(
                breakpoint.size == 2
                    ? (originalData[1] << 8) | originalData[0]
                    : (originalData[3] << 24) | (originalData[2] << 16) | (originalData[1] << 8) | originalData[0]
            )
        };

        static constexpr auto ebreakBytes = std::to_array<unsigned char>({
            static_cast<unsigned char>(::Targets::RiscV::Opcodes::Ebreak),
            static_cast<unsigned char>(::Targets::RiscV::Opcodes::Ebreak >> 8),
            static_cast<unsigned char>(::Targets::RiscV::Opcodes::Ebreak >> 16),
            static_cast<unsigned char>(::Targets::RiscV::Opcodes::Ebreak >> 24)
        });

        static constexpr auto compressedEbreakBytes = std::to_array<unsigned char>({
            static_cast<unsigned char>(::Targets::RiscV::Opcodes::EbreakCompressed),
            static_cast<unsigned char>(::Targets::RiscV::Opcodes::EbreakCompressed >> 8)
        });

        this->writeMemory(
            softwareBreakpoint.addressSpaceDescriptor,
            softwareBreakpoint.memorySegmentDescriptor,
            softwareBreakpoint.address,
            softwareBreakpoint.size == 2
                ? TargetMemoryBufferSpan{compressedEbreakBytes}
                : TargetMemoryBufferSpan{ebreakBytes}
        );

        this->softwareBreakpointRegistry.insert(softwareBreakpoint);
    }

    void WchLinkDebugInterface::clearSoftwareBreakpoint(const TargetProgramBreakpoint& breakpoint) {
        if (breakpoint.size != 2 && breakpoint.size != 4) {
            throw Exception{"Invalid software breakpoint size (" + std::to_string(breakpoint.size) + ")"};
        }

        const auto softwareBreakpointOpt = this->softwareBreakpointRegistry.find(breakpoint);
        if (!softwareBreakpointOpt.has_value()) {
            throw TargetOperationFailure{
                "Unknown software breakpoint (byte address: 0x" + Services::StringService::toHex(breakpoint.address)
                    + ")"
            };
        }

        const auto& softwareBreakpoint = softwareBreakpointOpt->get();
        if (!softwareBreakpoint.originalInstruction.has_value()) {
            throw InternalFatalErrorException{"Missing original opcode"};
        }

        this->writeMemory(
            softwareBreakpoint.addressSpaceDescriptor,
            softwareBreakpoint.memorySegmentDescriptor,
            softwareBreakpoint.address,
            softwareBreakpoint.size == 2
                ? TargetMemoryBuffer{
                    static_cast<unsigned char>(*(softwareBreakpoint.originalInstruction)),
                    static_cast<unsigned char>(*(softwareBreakpoint.originalInstruction) >> 8)
                }
                : TargetMemoryBuffer{
                    static_cast<unsigned char>(*(softwareBreakpoint.originalInstruction)),
                    static_cast<unsigned char>(*(softwareBreakpoint.originalInstruction) >> 8),
                    static_cast<unsigned char>(*(softwareBreakpoint.originalInstruction) >> 16),
                    static_cast<unsigned char>(*(softwareBreakpoint.originalInstruction) >> 24)
                }
        );

        this->softwareBreakpointRegistry.remove(softwareBreakpoint);
    }

    std::span<const unsigned char> WchLinkDebugInterface::getFlashProgramOpcodes(const std::string& key) {
        if (key == "op1") {
            return FlashProgramOpcodes::FLASH_OP1;
        }

        if (key == "op2") {
            return FlashProgramOpcodes::FLASH_OP2;
        }

        throw Targets::TargetDescription::Exceptions::InvalidTargetDescriptionDataException{
            "Invalid programming_opcode_key value (\"" + key + "\")"
        };
    }
}
