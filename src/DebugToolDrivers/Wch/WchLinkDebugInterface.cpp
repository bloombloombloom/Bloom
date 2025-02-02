#include "WchLinkDebugInterface.hpp"

#include <array>

#include "Protocols/WchLink/Commands/Control/AttachTarget.hpp"
#include "Protocols/WchLink/Commands/Control/DetachTarget.hpp"
#include "Protocols/WchLink/Commands/Control/PostAttach.hpp"
#include "Protocols/WchLink/Commands/Control/GetDeviceInfo.hpp"
#include "Protocols/WchLink/Commands/DebugModuleInterfaceOperation.hpp"

#include "Protocols/WchLink/FlashProgramOpcodes.hpp"

#include "src/Targets/RiscV/Opcodes/Opcode.hpp"

#include "src/Services/AlignmentService.hpp"
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
    using ::Targets::BreakpointResources;
    using ::Targets::TargetMemorySegmentType;
    using ::Targets::TargetRegisterDescriptor;
    using ::Targets::TargetRegisterDescriptors;
    using ::Targets::TargetRegisterDescriptorAndValuePairs;

    using namespace Protocols::WchLink;
    using namespace ::Exceptions;

    WchLinkDebugInterface::WchLinkDebugInterface(
        const WchLinkToolConfig& toolConfig,
        const Targets::RiscV::RiscVTargetConfig& targetConfig,
        const Targets::RiscV::TargetDescriptionFile& targetDescriptionFile,
        Protocols::WchLink::WchLinkInterface& wchLinkInterface,
        const DeviceInfo& toolInfo
    )
        : toolConfig(toolConfig)
        , targetConfig(targetConfig)
        , targetDescriptionFile(targetDescriptionFile)
        , wchLinkInterface(wchLinkInterface)
        , riscVTranslator(
            ::DebugToolDrivers::Protocols::RiscVDebug::DebugTranslator{
                this->wchLinkInterface,
                this->toolConfig.riscVDebugTranslatorConfig,
                this->targetDescriptionFile,
                this->targetConfig
            }
        )
        , toolInfo(toolInfo)
        , sysAddressSpaceDescriptor(this->targetDescriptionFile.getSystemAddressSpaceDescriptor())
        , mainProgramSegmentDescriptor(this->sysAddressSpaceDescriptor.getMemorySegmentDescriptor("main_program"))
        , bootProgramSegmentDescriptor(this->sysAddressSpaceDescriptor.getMemorySegmentDescriptor("boot_program"))
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
            throw DeviceCommunicationFailure{"Unexpected response payload size for AttachTarget command"};
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
                throw DeviceCommunicationFailure{
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
        this->riscVTranslator.deactivate();

        const auto response = this->wchLinkInterface.sendCommandAndWaitForResponse(Commands::Control::DetachTarget{});
        if (response.payload.size() != 1) {
            throw DeviceCommunicationFailure{"Unexpected response payload size for DetachTarget command"};
        }
    }

    std::string WchLinkDebugInterface::getDeviceId() {
        return "0x" + Services::StringService::toHex(this->cachedVariantId.value());
    }

    TargetExecutionState WchLinkDebugInterface::getExecutionState() {
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

    BreakpointResources WchLinkDebugInterface::getBreakpointResources() {
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

    TargetRegisterDescriptorAndValuePairs WchLinkDebugInterface::readCpuRegisters(
        const TargetRegisterDescriptors& descriptors
    ) {
        return this->riscVTranslator.readCpuRegisters(descriptors);
    }

    void WchLinkDebugInterface::writeCpuRegisters(const TargetRegisterDescriptorAndValuePairs& registers) {
        return this->riscVTranslator.writeCpuRegisters(registers);
    }

    TargetMemoryBuffer WchLinkDebugInterface::readMemory(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        TargetMemoryAddress startAddress,
        TargetMemorySize bytes,
        const std::set<TargetMemoryAddressRange>& excludedAddressRanges
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
        TargetMemoryAddress startAddress,
        TargetMemoryBufferSpan buffer
    ) {
        if (memorySegmentDescriptor.type == TargetMemorySegmentType::FLASH) {
            /*
             * WCH-Link tools provide two dedicated commands for writing to flash memory:
             *
             * - Partial block write
             *     Writes any number of 16-bit-aligned bytes to flash, but limited to a maximum of 64 bytes per write -
             *     larger writes must be split into multiple writes. Can only access a single page at a time - writes
             *     which span multiple pages must be split into multiple writes.
             * - Full block write
             *     Writes an entire block to flash, where the block size is target-specific (resides in the target's
             *     TDF) and is typically equal to 16 pages. Requires alignment to the block size. Requires reattaching
             *     to the target at the end of the write operation.
             *
             * The full block write is much faster for writing large buffers (KiBs), such as when we're programming
             * the target. But the partial block write is faster and more suitable for writing buffers that are
             * smaller than 64 bytes, such as when we're inserting software breakpoints.
             */
            if (
                buffer.size() > (WchLinkInterface::MAX_PARTIAL_BLOCK_WRITE_SIZE * 3)
                && this->fullBlockWriteCompatible(addressSpaceDescriptor, memorySegmentDescriptor, startAddress)
            ) {
                Logger::debug("Using full block write method");
                return this->writeProgramMemoryFullBlock(
                    addressSpaceDescriptor,
                    memorySegmentDescriptor,
                    startAddress,
                    buffer
                );
            }

            Logger::debug("Using partial block write method");
            return this->writeProgramMemoryPartialBlock(
                addressSpaceDescriptor,
                memorySegmentDescriptor,
                startAddress,
                buffer
            );
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
        /*
         * WCH-Link tools provide an erase function that erases the entire user section (main program segment).
         * However, this function doesn't always work. Sometimes, it silently fails and leaves the target in a bad
         * state, causing further issues (like program memory corruption).
         *
         * I spent a long time trying to fix this, but all attempts failed. I then decided to cease all use of this
         * function and just implement the erase procedure myself, in the target driver.
         *
         * See the WchRiscV::eraseMainFlashSegment() member function for more.
         *
         * For future reference, my notes on this issue:
         *
         * - I discovered the issue with a WCH-LinkE, FW version 2.9, connected to a CH32V003. I'm not sure if other
         *   targets are affected
         *
         * - The issue occurs rarely - a very specific sequence of events must take place for the issue to occur:
         *     1. Perform exactly one partial block write, to the main program segment, at address 0x08000100, with
         *        exactly 64 bytes of data
         *     2. Reset the target
         *     3. Erase the target via the erase function provided by the WCH-Link tool
         *   The erase operation will silently fail, and the target will be left in a bad state, causing the subsequent
         *   full block write to corrupt the target's program memory
         *
         * - The reset is what causes the erase operation to fail, but I have no idea why. I've inspected the relevant
         *   registers, at the relevant times, but found nothing significant that could explain this
         *
         * - Subsequent resets do not fix the issue, but another full block write does. My guess is that the first full
         *   block write (which corrupted program memory) corrected the target state, cleaning the mess made by the
         *   failed erase operation
         */
        throw TargetOperationFailure{"Not supported"};
    }

    void WchLinkDebugInterface::enableProgrammingMode() {
        // TODO: Move this to target driver. After v2.0.0.
        this->clearAllBreakpoints();
    }

    void WchLinkDebugInterface::disableProgrammingMode() {}

    void WchLinkDebugInterface::applyAccessRestrictions(TargetMemorySegmentDescriptor& memorySegmentDescriptor) {
        if (memorySegmentDescriptor.type == TargetMemorySegmentType::FLASH) {
            /*
             * Actually, we *can* write to flash memory whilst in debug mode (via a partial block write), but we don't
             * need to, so I'm just going to block it, for now.
             */
            memorySegmentDescriptor.debugModeAccess.writeable = false;
        }
    }

    void WchLinkDebugInterface::applyAccessRestrictions(TargetRegisterDescriptor& registerDescriptor) {
        // I don't believe any further access restrictions are required for registers. TODO: Review after v2.0.0.
    }

    void WchLinkDebugInterface::setSoftwareBreakpoint(const TargetProgramBreakpoint& breakpoint) {
        if (breakpoint.size != 2 && breakpoint.size != 4) {
            throw Exception{"Invalid software breakpoint size (" + std::to_string(breakpoint.size) + ")"};
        }

        static constexpr auto EBREAK_OPCODE = std::to_array<unsigned char>({
            static_cast<unsigned char>(::Targets::RiscV::Opcodes::Ebreak),
            static_cast<unsigned char>(::Targets::RiscV::Opcodes::Ebreak >> 8),
            static_cast<unsigned char>(::Targets::RiscV::Opcodes::Ebreak >> 16),
            static_cast<unsigned char>(::Targets::RiscV::Opcodes::Ebreak >> 24)
        });

        static constexpr auto COMPRESSED_EBREAK_OPCODE = std::to_array<unsigned char>({
            static_cast<unsigned char>(::Targets::RiscV::Opcodes::EbreakCompressed),
            static_cast<unsigned char>(::Targets::RiscV::Opcodes::EbreakCompressed >> 8)
        });

        this->writeMemory(
            breakpoint.addressSpaceDescriptor,
            breakpoint.memorySegmentDescriptor,
            breakpoint.address,
            breakpoint.size == 2
                ? TargetMemoryBufferSpan{COMPRESSED_EBREAK_OPCODE}
                : TargetMemoryBufferSpan{EBREAK_OPCODE}
        );

        this->softwareBreakpointRegistry.insert(breakpoint);
    }

    void WchLinkDebugInterface::clearSoftwareBreakpoint(const TargetProgramBreakpoint& breakpoint) {
        if (breakpoint.size != 2 && breakpoint.size != 4) {
            throw Exception{"Invalid software breakpoint size (" + std::to_string(breakpoint.size) + ")"};
        }

        this->writeMemory(
            breakpoint.addressSpaceDescriptor,
            breakpoint.memorySegmentDescriptor,
            breakpoint.address,
            TargetMemoryBufferSpan{
                breakpoint.originalData.begin(),
                breakpoint.originalData.begin() + breakpoint.size
            }
        );

        this->softwareBreakpointRegistry.remove(breakpoint);
    }

    void WchLinkDebugInterface::clearAllBreakpoints() {
        this->riscVTranslator.clearAllTriggers();
        for (const auto [addressSpaceId, breakpointsByAddress] : this->softwareBreakpointRegistry) {
            for (const auto& [address, breakpoint] : breakpointsByAddress) {
                this->clearSoftwareBreakpoint(breakpoint);
            }
        }
    }

    void WchLinkDebugInterface::writeProgramMemoryPartialBlock(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        Targets::TargetMemoryAddress startAddress,
        Targets::TargetMemoryBufferSpan buffer
    ) {
        using Services::AlignmentService;
        using namespace ::DebugToolDrivers::Protocols::RiscVDebug;

        if (buffer.empty()) {
            return;
        }

        const auto bufferSize = static_cast<TargetMemorySize>(buffer.size());
        const auto addressRange = TargetMemoryAddressRange{startAddress, startAddress + bufferSize - 1};
        assert(memorySegmentDescriptor.addressRange.contains(addressRange));

        /*
         * Partial block writes can only write to a single flash page at a time. If a write operation spans multiple
         * pages, the WCH-Link tool will write to the first page and ignore the rest, without reporting any error.
         *
         * We must break down write operations that span multiple pages.
         */
        assert(memorySegmentDescriptor.pageSize.has_value());
        const auto pages = addressRange.blocks(*memorySegmentDescriptor.pageSize);
        if (pages.size() > 1) {
            for (const auto& pageAddressRange : pages) {
                this->writeProgramMemoryPartialBlock(
                    addressSpaceDescriptor,
                    memorySegmentDescriptor,
                    pageAddressRange.startAddress,
                    buffer.subspan(
                        pageAddressRange.startAddress - addressRange.startAddress,
                        pageAddressRange.size()
                    )
                );
            }

            return;
        }

        /*
         * Partial block write operations must typically be 16-bit aligned.
         *
         * However, there is a bug in the WCH-Link firmware version 2.9, where partial writes of less than 64 bytes,
         * to the main program segment, whilst the target is in "boot mode", result in the entire 64-byte block being
         * written with garbage data. The bug can be circumvented by aligning to 64 bytes in these cases.
         *
         * This bug is fixed in firmware version 2.11, but I'm not sure if the fix was introduced in that version.
         * I don't have a tool with version 2.10, to check, so I'm just going to apply the 64-byte alignment to
         * anything below 2.11.
         */
        static constexpr auto MIN_FW_VERSION = WchFirmwareVersion{.major = 2, .minor = 11};
        const auto alignmentSize = static_cast<TargetMemorySize>(
            this->toolInfo.firmwareVersion < MIN_FW_VERSION ? 64 : 2
        );
        const auto alignedAddressRange = AlignmentService::alignAddressRange(addressRange, alignmentSize);

        if (alignedAddressRange != addressRange) {
            const auto alignedBufferSize = alignedAddressRange.size();
            const auto addressAlignmentBytes = static_cast<TargetMemorySize>(
                addressRange.startAddress - alignedAddressRange.startAddress
            );
            const auto sizeAlignmentBytes = alignedBufferSize - bufferSize - addressAlignmentBytes;

            auto alignedBuffer = addressAlignmentBytes > 0
                ? this->readMemory(
                    addressSpaceDescriptor,
                    memorySegmentDescriptor,
                    alignedAddressRange.startAddress,
                    addressAlignmentBytes,
                    {}
                )
                : TargetMemoryBuffer{};

            alignedBuffer.resize(alignedBufferSize);

            std::copy(buffer.begin(), buffer.end(), alignedBuffer.begin() + addressAlignmentBytes);

            if (sizeAlignmentBytes > 0) {
                const auto dataBack = this->readMemory(
                    addressSpaceDescriptor,
                    memorySegmentDescriptor,
                    addressRange.startAddress + bufferSize,
                    sizeAlignmentBytes,
                    {}
                );
                std::copy(
                    dataBack.begin(),
                    dataBack.end(),
                    alignedBuffer.begin() + addressAlignmentBytes + bufferSize
                );
            }

            return this->writeProgramMemoryPartialBlock(
                addressSpaceDescriptor,
                memorySegmentDescriptor,
                alignedAddressRange.startAddress,
                alignedBuffer
            );
        }

        /*
         * WCH-Link tools seem to make use of the target's program buffer to service the partial block write
         * command.
         *
         * This sometimes leads to exceptions occurring on the target, when the program buffer contains certain
         * instructions before the partial block write command is invoked. This is why we clear the program buffer
         * beforehand.
         */
        this->riscVTranslator.clearProgramBuffer();
        this->wchLinkInterface.writeFlashPartialBlock(startAddress, buffer);

        /*
         * Sometimes, when delegating part of a full block write operation to the partial block write method, a "busy"
         * error occurs. However, this doesn't seem to affect the outcome of the operation at all.
         *
         * This only seems to happen when writing to the boot segment of the CH32V003, shortly after a full block write
         * has taken place. It doesn't happen in the absence of a full block write.
         *
         * I suspect the tool may be attempting to verify the newly written data, and that is what's failing. But I
         * really don't know.
         *
         * For now, I think it's safe to ignore the "busy" error.
         */
        const auto commandError = this->riscVTranslator.readAndClearAbstractCommandError();
        if (
            commandError != DebugModule::AbstractCommandError::NONE
            && commandError != DebugModule::AbstractCommandError::BUSY
        ) {
            throw Exception{
                "Partial block write failed - abstract command error: 0x"
                    + Services::StringService::toHex(commandError)
            };
        }
    }

    void WchLinkDebugInterface::writeProgramMemoryFullBlock(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        Targets::TargetMemoryAddress startAddress,
        Targets::TargetMemoryBufferSpan buffer
    ) {
        using Services::AlignmentService;
        using Services::StringService;
        using namespace ::DebugToolDrivers::Protocols::RiscVDebug;

        if (buffer.empty()) {
            return;
        }

        const auto bufferSize = static_cast<TargetMemorySize>(buffer.size());
        const auto addressRange = TargetMemoryAddressRange{startAddress, startAddress + bufferSize - 1};
        assert(memorySegmentDescriptor.addressRange.contains(addressRange));

        auto alignedAddressRange = AlignmentService::alignAddressRange(addressRange, this->programmingBlockSize);
        if (alignedAddressRange != addressRange) {
            /*
             * The memory segment capacity may not be a multiple of the (target-specific) block size, meaning alignment
             * to the block size could result in breaching the boundary of the segment.
             *
             * For example, the CH32X035 has a block size of 4096, but its main program segment (`main_program`) has a
             * capacity of 62KiB (63488 bytes), which is not a multiple of 4096. This means we cannot access the final,
             * partial block of that segment, via a full block write.
             *
             * Some segments on some WCH RISC-V targets don't even have the capacity to accommodate the block size.
             *
             * This makes me suspect that
             *   1. I may be using the wrong block size, and the actual size is smaller, or
             *   2. Memory segment capacities could be wrong. I obtained these from the target datasheet.
             *
             * I have already tried experimenting with smaller block sizes, but nothing has worked. The WCH-Link tool
             * seems to expect these exact sizes before it will begin the full block write operation.
             *
             * Anyway, if the alignment results in the segment boundary being breached, we delegate the final part
             * of the write operation to the partial block write method, which only requires 16-bit alignment.
             *
             * In other words, we will write as many blocks as we can with the full block write method, and then write
             * the final part with the partial block write method. This allows us to benefit from the performance of
             * full block writes, whilst maintaining the ability to access the entire segment.
             */
            auto delegatedBytes = TargetMemorySize{0};
            if (!memorySegmentDescriptor.addressRange.contains(alignedAddressRange)) {
                Logger::debug(
                    "Alignment to the block size (" + std::to_string(this->programmingBlockSize)
                        + ") has resulted in a segment boundary breach"
                );

                alignedAddressRange.endAddress -= this->programmingBlockSize;

                /*
                 * This function isn't designed to handle instances where the entire write operation needs to be
                 * delegated. In such instances, this function should not be called at all. The following assertion
                 * enforces this.
                 *
                 * The WchLinkDebugInterface::fullBlockWriteCompatible() function will determine if at least part of
                 * the operation can be performed using the full block write method.
                 */
                assert(alignedAddressRange.intersectsWith(addressRange));

                delegatedBytes = addressRange.endAddress - alignedAddressRange.endAddress;

                Logger::debug(
                    "The full block write has been reduced to " + std::to_string(alignedAddressRange.size())
                        + " byte(s), from 0x" + StringService::toHex(alignedAddressRange.startAddress)
                );
                Logger::debug(std::to_string(delegatedBytes) + " byte(s) will be delegated to a partial write");
            }

            const auto alignedBufferSize = alignedAddressRange.size();
            const auto addressAlignmentBytes = static_cast<TargetMemorySize>(
                startAddress - alignedAddressRange.startAddress
            );
            const auto sizeAlignmentBytes = (alignedAddressRange.endAddress > addressRange.endAddress)
                ? alignedAddressRange.endAddress - addressRange.endAddress
                : 0;

            auto alignedBuffer = addressAlignmentBytes > 0
                ? this->readMemory(
                    addressSpaceDescriptor,
                    memorySegmentDescriptor,
                    alignedAddressRange.startAddress,
                    addressAlignmentBytes,
                    {}
                )
                : TargetMemoryBuffer{};

            alignedBuffer.resize(alignedBufferSize);

            std::copy(
                buffer.begin(),
                buffer.begin() + (bufferSize - delegatedBytes),
                alignedBuffer.begin() + addressAlignmentBytes
            );

            if (sizeAlignmentBytes > 0) {
                const auto dataBack = this->readMemory(
                    addressSpaceDescriptor,
                    memorySegmentDescriptor,
                    startAddress + bufferSize,
                    sizeAlignmentBytes,
                    {}
                );
                std::copy(
                    dataBack.begin(),
                    dataBack.end(),
                    alignedBuffer.begin() + addressAlignmentBytes + bufferSize
                );
            }

            this->writeProgramMemoryFullBlock(
                addressSpaceDescriptor,
                memorySegmentDescriptor,
                alignedAddressRange.startAddress,
                alignedBuffer
            );

            if (delegatedBytes > 0) {
                // Delegate the final part of the write operation to the partial write method
                const auto delegatedStartAddress = alignedAddressRange.endAddress + 1;
                const auto delegatedBuffer = buffer.subspan(bufferSize - delegatedBytes);
                Logger::debug(
                    "Delegating write operation 0x" + StringService::toHex(delegatedStartAddress) + ", "
                        + std::to_string(delegatedBuffer.size()) + " byte(s)"
                );

                this->writeProgramMemoryPartialBlock(
                    addressSpaceDescriptor,
                    memorySegmentDescriptor,
                    delegatedStartAddress,
                    delegatedBuffer
                );
            }

            return;
        }

        this->wchLinkInterface.writeFlashFullBlocks(
            startAddress,
            buffer,
            this->programmingBlockSize,
            this->flashProgramOpcodes
        );

        /*
         * Would this not be better placed in endProgrammingSession()? We could persist the command type we invoked to
         * perform the write, and if required, reattach at the end of the programming session.
         *
         * I don't think that would work, because the target needs to be accessible for other operations whilst in
         * programming mode. We may perform other operations in between program memory writes, but that wouldn't work
         * if we left the target in an inaccessible state between writes. So I think we have to reattach here.
         *
         * TODO: Review after v2.0.0.
         */
        this->deactivate();
        this->wchLinkInterface.sendCommandAndWaitForResponse(Commands::Control::GetDeviceInfo{});
        this->activate();
    }

    bool WchLinkDebugInterface::fullBlockWriteCompatible(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        TargetMemoryAddress startAddress
    ) {
        /*
         * If we cannot access the entire segment via the full block write method (the segment capacity is not a
         * multiple of the block size), we delegate the final part of the write operation to the partial write method.
         *
         * We use the end address of the final accessible block to determine if the write operation is contained
         * within the inaccessible region of the segment. If it is, we must not attempt the write operation via the
         * full block write, as the full block write code doesn't handle instances where the entire operation needs to
         * be delegated.
         *
         * See the WchLinkDebugInterface::writeProgramMemoryFullBlock() member function for more.
         */
        const auto finalBlockEnd = (
            (memorySegmentDescriptor.addressRange.endAddress / this->programmingBlockSize) * this->programmingBlockSize
        );
        return addressSpaceDescriptor == this->sysAddressSpaceDescriptor
            && memorySegmentDescriptor.type == TargetMemorySegmentType::FLASH
            && memorySegmentDescriptor.size() >= this->programmingBlockSize
            && (memorySegmentDescriptor.addressRange.startAddress % this->programmingBlockSize) == 0
            && (memorySegmentDescriptor.size() % this->programmingBlockSize == 0 || startAddress <= finalBlockEnd)
        ;
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
