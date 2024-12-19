#include "WchRiscV.hpp"

#include <utility>
#include <cassert>

#include "src/Targets/DynamicRegisterValue.hpp"

#include "src/Exceptions/InvalidConfig.hpp"
#include "src/Exceptions/Exception.hpp"

#include "src/EventManager/EventManager.hpp"

#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"

namespace Targets::RiscV::Wch
{
    WchRiscV::WchRiscV(
        const TargetConfig& targetConfig,
        TargetDescriptionFile&& targetDescriptionFile
    )
        : RiscV(targetConfig, targetDescriptionFile)
        , targetConfig(WchRiscVTargetConfig{RiscV::targetConfig})
        , targetDescriptionFile(std::move(targetDescriptionFile))
        , mappedSegmentDescriptor(this->sysAddressSpaceDescriptor.getMemorySegmentDescriptor("mapped_program_memory"))
        , mainProgramSegmentDescriptor(this->sysAddressSpaceDescriptor.getMemorySegmentDescriptor("main_program"))
        , bootProgramSegmentDescriptor(this->sysAddressSpaceDescriptor.getMemorySegmentDescriptor("boot_program"))
        , peripheralSegmentDescriptor(this->sysAddressSpaceDescriptor.getMemorySegmentDescriptor("peripherals"))
        , selectedProgramSegmentDescriptor(
            this->targetConfig.programSegmentKey.has_value()
            && *(this->targetConfig.programSegmentKey) == this->bootProgramSegmentDescriptor.key
                ? this->bootProgramSegmentDescriptor
                : this->mainProgramSegmentDescriptor
        )
        , flashPeripheralDescriptor(this->targetDescriptionFile.getTargetPeripheralDescriptor("flash"))
        , flashKeyRegisterDescriptor(this->flashPeripheralDescriptor.getRegisterDescriptor("flash", "keyr"))
        , flashBootKeyRegisterDescriptor(this->flashPeripheralDescriptor.getRegisterDescriptor("flash", "boot_modekeyr"))
        , flashStatusRegisterDescriptor(this->flashPeripheralDescriptor.getRegisterDescriptor("flash", "statr"))
        , flashStatusBootLockFieldDescriptor(this->flashStatusRegisterDescriptor.getBitFieldDescriptor("boot_lock"))
        , flashStatusBootModeFieldDescriptor(this->flashStatusRegisterDescriptor.getBitFieldDescriptor("boot_mode"))
    {
        if (
            this->targetConfig.programSegmentKey.has_value()
            && *(this->targetConfig.programSegmentKey) != this->bootProgramSegmentDescriptor.key
            && *(this->targetConfig.programSegmentKey) != this->mainProgramSegmentDescriptor.key
        ) {
            Logger::error(
                "Invalid program_segment_key parameter value (\"" + *(this->targetConfig.programSegmentKey)
                    + "\") - parameter will be ignored"
            );
        }

        Logger::info(
            "Selected program memory segment: \"" + this->selectedProgramSegmentDescriptor.name + "\" (\""
                + this->selectedProgramSegmentDescriptor.key + "\")"
        );

        if (
            this->selectedProgramSegmentDescriptor == this->bootProgramSegmentDescriptor
            && !this->selectedProgramSegmentDescriptor.programmingModeAccess.writeable
        ) {
            Logger::warning(
                "A read-only boot segment has been selected as the program memory segment - all programming sessions"
                    " will fail. This WCH target does not support storing custom bootloaders in the boot segment."
            );
            // TODO: Add link to further documentation here
        }
    }

    void WchRiscV::activate() {
        RiscV::activate();

        /*
         * WCH target IDs are specific to the variant. Each variant in the TDF should have a property group that holds
         * the variant ID.
         */
        const auto variantsById = this->targetDescriptionFile.getVariantsByWchVariantId();
        const auto deviceId = this->riscVDebugInterface->getDeviceId();

        const auto variantIt = variantsById.find(deviceId);
        if (variantIt == variantsById.end()) {
            throw Exceptions::InvalidConfig{
                "Unknown WCH variant ID \"" + deviceId + "\". Please check your configuration."
            };
        }

        this->variant = *(variantIt->second);
    }

    void WchRiscV::postActivate() {
        assert(this->variant.has_value());

        const auto& variant = this->variant->get();
        Logger::info("WCH variant ID: " + variant.getProperty("vendor", "variant_id").value);
        Logger::info("WCH variant name: " + variant.name);
    }

    TargetDescriptor WchRiscV::targetDescriptor() {
        auto descriptor = TargetDescriptor{
            this->targetDescriptionFile.getName(),
            this->targetDescriptionFile.getFamily(),
            this->variant->get().getProperty("vendor", "variant_id").value,
            this->targetDescriptionFile.getVendorName(),
            this->targetDescriptionFile.targetAddressSpaceDescriptorsByKey(),
            this->targetDescriptionFile.targetPeripheralDescriptorsByKey(),
            this->targetDescriptionFile.targetPadDescriptorsByKey(),
            this->targetDescriptionFile.targetPinoutDescriptorsByKey(),
            this->targetDescriptionFile.targetVariantDescriptorsByKey(),
            this->riscVDebugInterface->getBreakpointResources()
        };

        if (
            this->targetConfig.reserveSteppingBreakpoint.value_or(false)
            && descriptor.breakpointResources.hardwareBreakpoints > 0
        ) {
            descriptor.breakpointResources.reservedHardwareBreakpoints = 1;
        }

        // Copy the RISC-V CPU register address space and peripheral descriptor
        descriptor.addressSpaceDescriptorsByKey.emplace(
            this->cpuRegisterAddressSpaceDescriptor.key,
            this->cpuRegisterAddressSpaceDescriptor.clone()
        );

        descriptor.peripheralDescriptorsByKey.emplace(
            this->cpuPeripheralDescriptor.key,
            this->cpuPeripheralDescriptor.clone()
        );

        for (auto& [addressSpaceKey, addressSpaceDescriptor] : descriptor.addressSpaceDescriptorsByKey) {
            this->applyDebugInterfaceAccessRestrictions(addressSpaceDescriptor);
        }

        for (auto& [peripheralKey, peripheralDescriptor] : descriptor.peripheralDescriptorsByKey) {
            for (auto& [groupKey, groupDescriptor] : peripheralDescriptor.registerGroupDescriptorsByKey) {
                this->applyDebugInterfaceAccessRestrictions(groupDescriptor);
            }
        }

        auto& sysAddressSpaceDescriptor = descriptor.getAddressSpaceDescriptor("system");
        sysAddressSpaceDescriptor.getMemorySegmentDescriptor("main_program").inspectionEnabled = true;
        sysAddressSpaceDescriptor.getMemorySegmentDescriptor("internal_ram").inspectionEnabled = true;
        sysAddressSpaceDescriptor.getMemorySegmentDescriptor("boot_program").inspectionEnabled = true;

        /*
         * WCH targets typically possess a memory segment that is mapped to program memory. We cannot write to this
         * segment directly, which is why it's described as read-only in Bloom's TDFs. However, we enable writing to
         * the segment by forwarding any write operations to the appropriate (aliased) segment.
         *
         * For this reason, we adjust the access member on the memory segment descriptor so that other components
         * within Bloom will see the segment as writeable.
         *
         * See the overridden WchRiscV::writeMemory() member function below, for more.
         */
        sysAddressSpaceDescriptor.getMemorySegmentDescriptor(
            this->mappedSegmentDescriptor.key
        ).programmingModeAccess.writeable = true;

        return descriptor;
    }

    void WchRiscV::setProgramBreakpoint(const TargetProgramBreakpoint& breakpoint) {
        if (
            breakpoint.type == TargetProgramBreakpoint::Type::SOFTWARE
            && breakpoint.memorySegmentDescriptor == this->mappedSegmentDescriptor
        ) {
            if (
                !this->selectedProgramSegmentDescriptor.debugModeAccess.writeable
                && (!this->programmingMode || !this->selectedProgramSegmentDescriptor.programmingModeAccess.writeable)
            ) {
                throw Exceptions::Exception{
                    "The selected program memory segment (\"" + this->selectedProgramSegmentDescriptor.key
                        + "\") is not writable - cannot insert software breakpoint"
                };
            }

            this->riscVDebugInterface->setProgramBreakpoint(TargetProgramBreakpoint{
                .addressSpaceDescriptor = this->sysAddressSpaceDescriptor,
                .memorySegmentDescriptor = this->selectedProgramSegmentDescriptor,
                .address = this->transformMappedAddress(breakpoint.address, this->selectedProgramSegmentDescriptor),
                .size = breakpoint.size,
                .type = breakpoint.type
            });

            return;
        }

        this->riscVDebugInterface->setProgramBreakpoint(breakpoint);
    }

    void WchRiscV::removeProgramBreakpoint(const TargetProgramBreakpoint& breakpoint) {
        if (
            breakpoint.type == TargetProgramBreakpoint::Type::SOFTWARE
            && breakpoint.memorySegmentDescriptor == this->mappedSegmentDescriptor
        ) {
            if (
                !this->selectedProgramSegmentDescriptor.debugModeAccess.writeable
                && (!this->programmingMode || !this->selectedProgramSegmentDescriptor.programmingModeAccess.writeable)
            ) {
                throw Exceptions::Exception{
                    "The selected program memory segment (\"" + this->selectedProgramSegmentDescriptor.key
                        + "\") is not writable - cannot remove software breakpoint"
                };
            }

            this->riscVDebugInterface->removeProgramBreakpoint(TargetProgramBreakpoint{
                .addressSpaceDescriptor = this->sysAddressSpaceDescriptor,
                .memorySegmentDescriptor = this->selectedProgramSegmentDescriptor,
                .address = this->transformMappedAddress(breakpoint.address, this->selectedProgramSegmentDescriptor),
                .size = breakpoint.size,
                .type = breakpoint.type
            });

            return;
        }

        this->riscVDebugInterface->removeProgramBreakpoint(breakpoint);
    }

    TargetMemoryBuffer WchRiscV::readMemory(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        TargetMemoryAddress startAddress,
        TargetMemorySize bytes,
        const std::set<TargetMemoryAddressRange>& excludedAddressRanges
    ) {
        using Services::StringService;

        if (memorySegmentDescriptor == this->mappedSegmentDescriptor) {
            const auto& aliasedSegment = this->selectedProgramSegmentDescriptor;
            const auto transformedAddress = this->transformMappedAddress(startAddress, aliasedSegment);

            const auto addressRange = TargetMemoryAddressRange{
                transformedAddress,
                static_cast<TargetMemoryAddress>(transformedAddress + bytes - 1)
            };

            if (!aliasedSegment.addressRange.contains(addressRange)) {
                throw Exceptions::Exception{
                    "Read access range (0x" + StringService::toHex(addressRange.startAddress) + " -> 0x"
                        + StringService::toHex(addressRange.endAddress) + ", " + std::to_string(addressRange.size())
                        + " bytes) exceeds the boundary of the selected program segment \"" + aliasedSegment.key
                        + "\" (0x" + StringService::toHex(aliasedSegment.addressRange.startAddress) + " -> 0x"
                        + StringService::toHex(aliasedSegment.addressRange.endAddress) + ", "
                        + std::to_string(aliasedSegment.addressRange.size()) + " bytes)"
                };
            }

            return RiscV::readMemory(
                addressSpaceDescriptor,
                aliasedSegment,
                transformedAddress,
                bytes,
                excludedAddressRanges
            );
        }

        return RiscV::readMemory(
            addressSpaceDescriptor,
            memorySegmentDescriptor,
            startAddress,
            bytes,
            excludedAddressRanges
        );
    }

    void WchRiscV::writeMemory(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        TargetMemoryAddress startAddress,
        TargetMemoryBufferSpan buffer
    ) {
        using Services::StringService;

        if (memorySegmentDescriptor == this->mappedSegmentDescriptor) {
            const auto& aliasedSegment = this->selectedProgramSegmentDescriptor;

            if (
                !aliasedSegment.debugModeAccess.writeable
                && (!this->programmingMode || !aliasedSegment.programmingModeAccess.writeable)
            ) {
                throw Exceptions::Exception{
                    "The selected program memory segment (\"" + aliasedSegment.key + "\") is not writable"
                };
            }

            const auto transformedAddress = this->transformMappedAddress(startAddress, aliasedSegment);

            const auto addressRange = TargetMemoryAddressRange{
                transformedAddress,
                static_cast<TargetMemoryAddress>(transformedAddress + buffer.size() - 1)
            };

            if (!aliasedSegment.addressRange.contains(addressRange)) {
                throw Exceptions::Exception{
                    "Write access range (0x" + StringService::toHex(addressRange.startAddress) + " -> 0x"
                        + StringService::toHex(addressRange.endAddress) + ", " + std::to_string(addressRange.size())
                        + " bytes) exceeds the boundary of the selected program segment \"" + aliasedSegment.key
                        + "\" (0x" + StringService::toHex(aliasedSegment.addressRange.startAddress) + " -> 0x"
                        + StringService::toHex(aliasedSegment.addressRange.endAddress) + ", "
                        + std::to_string(aliasedSegment.addressRange.size()) + " bytes)"
                };
            }

            return RiscV::writeMemory(addressSpaceDescriptor, aliasedSegment, transformedAddress, buffer);
        }

        return RiscV::writeMemory(addressSpaceDescriptor, memorySegmentDescriptor, startAddress, buffer);
    }

    void WchRiscV::eraseMemory(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor
    ) {
        if (memorySegmentDescriptor == this->mappedSegmentDescriptor) {
            return RiscV::eraseMemory(addressSpaceDescriptor, this->selectedProgramSegmentDescriptor);
        }

        RiscV::eraseMemory(addressSpaceDescriptor, memorySegmentDescriptor);
    }

    TargetMemoryAddress WchRiscV::getProgramCounter() {
        using Services::StringService;

        const auto programCounter = RiscV::getProgramCounter();

        if (this->mappedSegmentDescriptor.addressRange.contains(programCounter)) {
            const auto& actualAliasedSegment = this->resolveAliasedMemorySegment();
            if (actualAliasedSegment != this->selectedProgramSegmentDescriptor) {
                /*
                 * The target's mapped segment no longer aliases the selected program segment.
                 *
                 * Imagine starting a debug session with GDB, then replacing the entire program being debugged with a
                 * totally different program, whilst GDB is still running and the same debug session is still active.
                 * Understandably, GDB would become very confused by this, as it has no idea what just happened, or why
                 * the program it was observing just moments ago has suddenly disappeared and been replaced by another.
                 *
                 * This is essentially what has just happened. The mapped segment initially aliased one segment in
                 * program memory, but now, all of a sudden, it appears to be aliasing a different segment. This can
                 * happen when the target switches to a different mode of operation. When the target is in "user mode",
                 * the mapped segment aliases the main program segment. But when the target is in "boot mode", the
                 * mapped segment aliases the boot segment. The program running on the target can invoke a mode switch
                 * by writing to a register and performing a software reset.
                 *
                 * So, we have a program counter that's addressing a totally different program, but to most external
                 * entities, it will appear as if it's addressing the same program.
                 *
                 * In order to avoid causing havoc and potentially misleading the user, we transform the PC to its
                 * aliased address. That way, it will be clear to all external entities, that the target is currently
                 * executing code in a different memory segment to the one that was selected for debugging.
                 */
                const auto transformedAddress = this->transformMappedAddress(programCounter, actualAliasedSegment);
                Logger::warning(
                    "The mapped program memory segment is currently aliasing a foreign segment (\""
                        + actualAliasedSegment.key + "\") - the program counter (0x"
                        + StringService::toHex(programCounter) + ") has been transformed to the aliased address (0x"
                        + StringService::toHex(transformedAddress) + ")"
                );
                return transformedAddress;
            }
        }

        return programCounter;
    }

    std::string WchRiscV::passthroughCommandHelpText() {
        using Services::StringService;

        static constexpr auto CMD_COLOR = StringService::TerminalColor::DARK_YELLOW;
        static constexpr auto PARAM_COLOR = StringService::TerminalColor::BLUE;

        static const auto leftPadding = std::string{std::string::size_type{3}, ' ', std::string::allocator_type{}};
        auto output = std::string{};

        output += StringService::applyTerminalColor("program_mode", CMD_COLOR) + "\n\n";
        output += leftPadding + "Determines the target's current program mode (boot/user).\n\n";

        output += StringService::applyTerminalColor("program_mode", CMD_COLOR) + " ["
            + StringService::applyTerminalColor("MODE", PARAM_COLOR) + "]\n\n";
        output += leftPadding + "Changes the program mode on the target. Triggers a target reset.\n";
        output += leftPadding + "Valid modes: \"boot\" and \"user\".\n\n";
        output += leftPadding + "Examples:\n\n";
        output += leftPadding + "mon " + StringService::applyTerminalColor("program_mode", CMD_COLOR) + " "
            + StringService::applyTerminalColor("boot", PARAM_COLOR) + "\n";
        output += leftPadding + "  To switch to boot mode, where the mapped program memory segment aliases the boot"
            " segment (key: \"" + this->bootProgramSegmentDescriptor.key + "\").\n\n";
        output += leftPadding + "mon " + StringService::applyTerminalColor("program_mode", CMD_COLOR) + " "
            + StringService::applyTerminalColor("user", PARAM_COLOR) + "\n";
        output += leftPadding + "  To switch to user mode, where the mapped program memory segment aliases the main"
            " program segment (key: \"" + this->mainProgramSegmentDescriptor.key + "\").\n";

        return output;
    }

    std::optional<PassthroughResponse> WchRiscV::invokePassthroughCommand(const PassthroughCommand& command) {
        using Services::StringService;

        const auto& arguments = command.arguments;
        if (arguments.empty()) {
            return std::nullopt;
        }

        auto response = PassthroughResponse{};

        try {
            if (arguments[0] == "program_mode") {
                const auto &actualAliasedSegment = this->resolveAliasedMemorySegment();

                if (arguments.size() == 1) {
                    response.output = "Program mode: \"" + StringService::applyTerminalColor(
                        actualAliasedSegment == this->bootProgramSegmentDescriptor ? "boot mode" : "user mode",
                        StringService::TerminalColor::DARK_YELLOW
                    ) + "\"\n";
                    response.output += "Aliased memory segment key: \""
                        + StringService::applyTerminalColor(
                            actualAliasedSegment.key,
                            StringService::TerminalColor::DARK_YELLOW
                        ) + "\"\n";
                    response.output += "Mapped address -> aliased address: " + StringService::applyTerminalColor(
                        "0x" + StringService::asciiToUpper(
                            StringService::toHex(this->mappedSegmentDescriptor.addressRange.startAddress)
                        ),
                        StringService::TerminalColor::BLUE
                    ) + " -> " + StringService::applyTerminalColor(
                        "0x" + StringService::asciiToUpper(
                            StringService::toHex(actualAliasedSegment.addressRange.startAddress)
                        ),
                        StringService::TerminalColor::BLUE
                    ) + "\n";
                    response.output += "Program counter: " + StringService::applyTerminalColor(
                        "0x" + StringService::asciiToUpper(StringService::toHex(this->getProgramCounter())),
                        StringService::TerminalColor::BLUE
                    ) + "\n";

                    return response;
                }

                if (arguments[1] == "boot") {
                    if (actualAliasedSegment == this->bootProgramSegmentDescriptor) {
                        response.output += "Target is already in \"boot mode\"\n";
                        response.output += "Proceeding, anyway...\n\n";
                    }

                    this->enableBootMode();
                    EventManager::triggerEvent(std::make_shared<Events::TargetReset>());

                    response.output += "Boot mode has been enabled\n";
                    response.output += "Program counter: " + StringService::applyTerminalColor(
                        "0x" + StringService::asciiToUpper(StringService::toHex(this->getProgramCounter())),
                        StringService::TerminalColor::BLUE
                    ) + "\n";

                    return response;
                }

                if (arguments[1] == "user") {
                    if (actualAliasedSegment == this->mainProgramSegmentDescriptor) {
                        response.output += "Target is already in \"user mode\"\n";
                        response.output += "Proceeding, anyway...\n\n";
                    }

                    this->enableUserMode();
                    EventManager::triggerEvent(std::make_shared<Events::TargetReset>());

                    response.output += "User mode has been enabled\n";
                    response.output += "Program counter: " + StringService::applyTerminalColor(
                        "0x" + StringService::asciiToUpper(StringService::toHex(this->getProgramCounter())),
                        StringService::TerminalColor::BLUE
                    ) + "\n";

                    return response;
                }
            }

        } catch (const Exceptions::Exception& exception) {
            Logger::error("Passthrough command error: " + exception.getMessage());
            response.output = "Error: " + exception.getMessage();
            return response;
        }

        return std::nullopt;
    }

    const TargetMemorySegmentDescriptor& WchRiscV::resolveAliasedMemorySegment() {
        /*
         * To determine the aliased segment, we probe the boundary of the boot segment via the mapped segment.
         *
         * Assumptions that must hold, for this to work:
         *   - The boot segment must be smaller than the main program memory segment
         *   - Breaching the boundary of the boot segment must always result in an exception (out-of-bounds error)
         *
         * If the mapped segment is aliasing the boot segment, the memory access will fail, due to an out-of-bounds
         * error. If the access succeeds, we can be fairly certain the mapped segment is aliasing the main program
         * memory segment.
         *
         * I did consider using the FLASH_STATR peripheral register to determine the aliased segment, but not all WCH
         * targets have the required bit fields for that to work. And even the ones that do, do not behave in the way
         * described by the documentation.
         */
        const auto probeAddress = this->bootProgramSegmentDescriptor.addressRange.endAddress
            - this->bootProgramSegmentDescriptor.addressRange.startAddress
            + this->mappedSegmentDescriptor.addressRange.startAddress + 1;

        assert(this->sysAddressSpaceDescriptor.addressRange.contains(probeAddress));
        assert(this->mainProgramSegmentDescriptor.size() > this->bootProgramSegmentDescriptor.size());

        const auto& segment = this->probeMemory(
            this->sysAddressSpaceDescriptor,
            this->mappedSegmentDescriptor,
            probeAddress
        ) ? this->mainProgramSegmentDescriptor : this->bootProgramSegmentDescriptor;

        Logger::debug("Aliased program memory segment: \"" + segment.key + "\"");
        return segment;
    }

    TargetMemoryAddress WchRiscV::transformMappedAddress(
        TargetMemoryAddress address,
        const TargetMemorySegmentDescriptor& segmentDescriptor
    ) {
        using Services::StringService;

        const auto transformedAddress = address - this->mappedSegmentDescriptor.addressRange.startAddress
            + segmentDescriptor.addressRange.startAddress;

        Logger::debug(
            "Transformed mapped program memory address 0x" + StringService::toHex(address) + " to 0x"
                + StringService::toHex(transformedAddress) + " (segment: \"" + segmentDescriptor.key + "\")"
        );

        return transformedAddress;
    }

    void WchRiscV::unlockFlash() {
        // TODO: Move these key values to a TDF property. After v2.0.0
        this->writeRegister(this->flashKeyRegisterDescriptor, 0x45670123);
        this->writeRegister(this->flashKeyRegisterDescriptor, 0xCDEF89AB);
    }

    void WchRiscV::unlockBootModeBitField() {
        // TODO: Move these key values to a TDF property. After v2.0.0
        this->writeRegister(this->flashBootKeyRegisterDescriptor, 0x45670123);
        this->writeRegister(this->flashBootKeyRegisterDescriptor, 0xCDEF89AB);
    }

    void WchRiscV::enableBootMode() {
        this->unlockFlash();
        this->unlockBootModeBitField();

        auto statusRegister = this->readRegisterDynamicValue(this->flashStatusRegisterDescriptor);

        if (statusRegister.bitFieldAs<bool>(this->flashStatusBootLockFieldDescriptor)) {
            throw Exceptions::Exception{"Failed to unlock boot mode field"};
        }

        statusRegister.setBitField(this->flashStatusBootModeFieldDescriptor, true);
        this->writeRegister(this->flashStatusRegisterDescriptor, statusRegister.data());

        this->reset();
    }

    void WchRiscV::enableUserMode() {
        this->unlockFlash();
        this->unlockBootModeBitField();

        auto statusRegister = this->readRegisterDynamicValue(this->flashStatusRegisterDescriptor);

        if (statusRegister.bitFieldAs<bool>(this->flashStatusBootLockFieldDescriptor)) {
            throw Exceptions::Exception{"Failed to unlock boot mode field"};
        }

        statusRegister.setBitField(this->flashStatusBootModeFieldDescriptor, false);
        this->writeRegister(this->flashStatusRegisterDescriptor, statusRegister.data());

        this->reset();
    }
}
