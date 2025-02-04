#include "WchRiscV.hpp"

#include <utility>
#include <cassert>
#include <algorithm>
#include <chrono>
#include <thread>

#include "src/Targets/DynamicRegisterValue.hpp"

#include "src/Exceptions/InvalidConfig.hpp"
#include "src/Exceptions/Exception.hpp"

#include "src/EventManager/EventManager.hpp"

#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"
#include "src/TargetController/Exceptions/TargetFailure.hpp"
#include "src/TargetController/Exceptions/TargetOperationFailure.hpp"

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
        , flashControlRegisterDescriptor(this->flashPeripheralDescriptor.getRegisterDescriptor("flash", "ctrl"))
        , flashOptionByteRegisterDescriptor(this->flashPeripheralDescriptor.getRegisterDescriptor("flash", "obr"))
        , flashControlRegisterFields(
            Peripherals::Flash::FlashControlRegisterFields{
                .locked = this->flashControlRegisterDescriptor.getBitFieldDescriptor("lock"),
                .startErase = this->flashControlRegisterDescriptor.getBitFieldDescriptor("strt"),
                .mainSegmentErase = this->flashControlRegisterDescriptor.getBitFieldDescriptor("mer"),
            }
        )
        , flashStatusRegisterFields(
            Peripherals::Flash::FlashStatusRegisterFields{
                .busy = this->flashStatusRegisterDescriptor.getBitFieldDescriptor("bsy"),
                .bootLock = this->flashStatusRegisterDescriptor.getBitFieldDescriptor("boot_lock"),
                .bootMode = this->flashStatusRegisterDescriptor.getBitFieldDescriptor("boot_mode"),
            }
        )
        , flashOptionByteRegisterFields(
            Peripherals::Flash::FlashOptionByteRegisterFields{
                .readProtected = this->flashOptionByteRegisterDescriptor.getBitFieldDescriptor("rdprt"),
            }
        )
        , rccPeripheralDescriptor(this->targetDescriptionFile.getTargetPeripheralDescriptor("rcc"))
        , portPeripheralClockEnableRegisterDescriptor(
            this->rccPeripheralDescriptor.getRegisterDescriptor("rcc", "apb2pcenr")
        )
        , padDescriptors(this->targetDescriptionFile.targetPadDescriptors())
        , gpioPortPeripheralDescriptors(this->targetDescriptionFile.gpioPortPeripheralDescriptors())
        , gpioPadDescriptorsByPadId(
            WchRiscV::generateGpioPadDescriptorMapping(
                this->portPeripheralClockEnableRegisterDescriptor,
                this->gpioPortPeripheralDescriptors,
                this->padDescriptors
            )
        )
    {
        if (this->targetConfig.physicalInterface != TargetPhysicalInterface::SDI) {
            throw Exceptions::InvalidConfig{
                "Invalid physical interface selection - WCH RISC-V targets only support the SDI interface"
            };
        }

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
            "Selected program memory segment: \"" + this->selectedProgramSegmentDescriptor.name + "\" ("
                + Services::StringService::formatKey(this->selectedProgramSegmentDescriptor.key) + ")"
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

        const auto optionByteValue = this->readRegisterDynamicValue(this->flashOptionByteRegisterDescriptor);
        if (optionByteValue.bitFieldAs<bool>(this->flashOptionByteRegisterFields.readProtected)) {
            throw Exceptions::TargetFailure{"Program memory is currently \"read-protected\""};
        }
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

        // Replace the TDF CPU peripheral descriptor with the mutated copy (see RiscV base class)
        descriptor.peripheralDescriptorsByKey.erase(this->cpuPeripheralDescriptor.key);
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
         * We cannot write to the mapped program memory segment directly, which is why it's described as read-only in
         * Bloom's TDFs. However, we enable writing to the segment by forwarding any write operations to the selected
         * program segment.
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
            if (!this->selectedProgramSegmentDescriptor.programmingModeAccess.writeable) {
                throw Exceptions::Exception{
                    "The selected program memory segment ("
                        + Services::StringService::formatKey(this->selectedProgramSegmentDescriptor.key)
                        + ") is not writable - cannot insert software breakpoint"
                };
            }

            this->riscVDebugInterface->setProgramBreakpoint(TargetProgramBreakpoint{
                .addressSpaceDescriptor = this->sysAddressSpaceDescriptor,
                .memorySegmentDescriptor = this->selectedProgramSegmentDescriptor,
                .address = this->deAliasMappedAddress(breakpoint.address, this->selectedProgramSegmentDescriptor),
                .size = breakpoint.size,
                .type = breakpoint.type,
                .originalData = breakpoint.originalData
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
            if (!this->selectedProgramSegmentDescriptor.programmingModeAccess.writeable) {
                throw Exceptions::Exception{
                    "The selected program memory segment ("
                        + Services::StringService::formatKey(this->selectedProgramSegmentDescriptor.key)
                        + ") is not writable - cannot remove software breakpoint"
                };
            }

            this->riscVDebugInterface->removeProgramBreakpoint(TargetProgramBreakpoint{
                .addressSpaceDescriptor = this->sysAddressSpaceDescriptor,
                .memorySegmentDescriptor = this->selectedProgramSegmentDescriptor,
                .address = this->deAliasMappedAddress(breakpoint.address, this->selectedProgramSegmentDescriptor),
                .size = breakpoint.size,
                .type = breakpoint.type,
                .originalData = breakpoint.originalData
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
            const auto transformedAddress = this->deAliasMappedAddress(startAddress, aliasedSegment);

            const auto addressRange = TargetMemoryAddressRange{
                transformedAddress,
                static_cast<TargetMemoryAddress>(transformedAddress + bytes - 1)
            };

            if (!aliasedSegment.addressRange.contains(addressRange)) {
                throw Exceptions::Exception{
                    "Read access range (0x" + StringService::toHex(addressRange.startAddress) + " -> 0x"
                        + StringService::toHex(addressRange.endAddress) + ", " + std::to_string(addressRange.size())
                        + " bytes) exceeds the boundary of the selected program segment "
                        + StringService::formatKey(aliasedSegment.key) + " (0x"
                        + StringService::toHex(aliasedSegment.addressRange.startAddress) + " -> 0x"
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
                    "The selected program memory segment (" + StringService::formatKey(aliasedSegment.key)
                        + ") is not writable"
                };
            }

            const auto transformedAddress = this->deAliasMappedAddress(startAddress, aliasedSegment);

            const auto addressRange = TargetMemoryAddressRange{
                transformedAddress,
                static_cast<TargetMemoryAddress>(transformedAddress + buffer.size() - 1)
            };

            if (!aliasedSegment.addressRange.contains(addressRange)) {
                throw Exceptions::Exception{
                    "Write access range (0x" + StringService::toHex(addressRange.startAddress) + " -> 0x"
                        + StringService::toHex(addressRange.endAddress) + ", " + std::to_string(addressRange.size())
                        + " bytes) exceeds the boundary of the selected program segment "
                        + StringService::formatKey(aliasedSegment.key) + " (0x"
                        + StringService::toHex(aliasedSegment.addressRange.startAddress) + " -> 0x"
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
        if (
            memorySegmentDescriptor == this->mainProgramSegmentDescriptor
            || (
                memorySegmentDescriptor == this->mappedSegmentDescriptor
                && this->selectedProgramSegmentDescriptor == this->mainProgramSegmentDescriptor
            )
        ) {
            return this->eraseMainFlashSegment();
        }

        Logger::debug(
            "Ignoring erase operation on " + Services::StringService::formatKey(memorySegmentDescriptor.key)
                + " segment - not supported"
        );
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
                 * In order to avoid causing havoc and potentially misleading the user, we de-alias the program
                 * counter in this scenario. That way, it will be clear to all external entities, that the target is
                 * currently executing code in a different memory segment to the one that was selected for debugging.
                 */
                const auto deAliasedAddress = this->deAliasMappedAddress(programCounter, actualAliasedSegment);
                Logger::warning(
                    "The mapped program memory segment is currently aliasing the " + actualAliasedSegment.key
                        + " segment - the program counter (0x" + StringService::toHex(programCounter)
                        + ") has been de-aliased to 0x" + StringService::toHex(deAliasedAddress)
                );
                return deAliasedAddress;
            }
        }

        return programCounter;
    }

    TargetGpioPadDescriptorAndStatePairs WchRiscV::getGpioPadStates(const TargetPadDescriptors& padDescriptors) {
        auto cachedRegsById = std::unordered_map<TargetRegisterId, DynamicRegisterValue>{};
        const auto readGpioReg = [this, &cachedRegsById] (const TargetRegisterDescriptor& descriptor)
            -> DynamicRegisterValue& {
            auto cachedRegIt = cachedRegsById.find(descriptor.id);
            if (cachedRegIt == cachedRegsById.end()) {
                cachedRegIt = cachedRegsById.emplace(
                    descriptor.id,
                    this->readRegisterDynamicValue(descriptor)
                ).first;
            }

            return cachedRegIt->second;
        };

        auto output = TargetGpioPadDescriptorAndStatePairs{};

        for (const auto* padDescriptor : padDescriptors) {
            const auto gpioPadDescriptorIt = this->gpioPadDescriptorsByPadId.find(padDescriptor->id);
            if (gpioPadDescriptorIt == this->gpioPadDescriptorsByPadId.end()) {
                continue;
            }

            const auto& gpioPadDescriptor = gpioPadDescriptorIt->second;

            const auto& portClockEnableRegisterValue = readGpioReg(this->portPeripheralClockEnableRegisterDescriptor);
            if (!portClockEnableRegisterValue.bitFieldAs<bool>(gpioPadDescriptor.peripheralClockEnableBitFieldDescriptor)) {
                // The port peripheral is currently disabled. We cannot obtain any meaningful state for this pad
                continue;
            }

            const auto& configRegisterValue = readGpioReg(gpioPadDescriptor.configRegisterDescriptor);
            const auto padMode = configRegisterValue.bitField(gpioPadDescriptor.modeBitFieldDescriptor);

            if (padMode == static_cast<std::uint8_t>(GpioPadDirection::INPUT)) {
                output.emplace_back(
                    *padDescriptor,
                    TargetGpioPadState{
                        readGpioReg(gpioPadDescriptor.inputDataRegisterDescriptor).bitFieldAs<bool>(
                            gpioPadDescriptor.inputDataBitFieldDescriptor
                        ) ? TargetGpioPadState::State::HIGH : TargetGpioPadState::State::LOW,
                        TargetGpioPadState::DataDirection::INPUT
                    }
                );

                continue;
            }

            output.emplace_back(
                *padDescriptor,
                TargetGpioPadState{
                    readGpioReg(gpioPadDescriptor.outputDataRegisterDescriptor).bitFieldAs<bool>(
                        gpioPadDescriptor.outputDataBitFieldDescriptor
                    ) ? TargetGpioPadState::State::HIGH : TargetGpioPadState::State::LOW,
                    TargetGpioPadState::DataDirection::OUTPUT
                }
            );
        }

        return output;
    }

    void WchRiscV::setGpioPadState(const TargetPadDescriptor& padDescriptor, const TargetGpioPadState& state) {
        const auto gpioPadDescriptorIt = this->gpioPadDescriptorsByPadId.find(padDescriptor.id);
        if (gpioPadDescriptorIt == this->gpioPadDescriptorsByPadId.end()) {
            throw Exceptions::Exception{"Unknown pad"};
        }

        const auto& gpioPadDescriptor = gpioPadDescriptorIt->second;

        auto configRegisterValue = this->readRegisterDynamicValue(gpioPadDescriptor.configRegisterDescriptor);
        const auto currentDir = configRegisterValue.bitField(
            gpioPadDescriptor.modeBitFieldDescriptor
        ) == static_cast<std::uint8_t>(GpioPadDirection::INPUT)
            ? TargetGpioPadState::DataDirection::INPUT
            : TargetGpioPadState::DataDirection::OUTPUT;

        if (currentDir != state.direction) {
            configRegisterValue.setBitField(
                gpioPadDescriptor.modeBitFieldDescriptor,
                static_cast<std::uint8_t>(
                    state.direction == TargetGpioPadState::DataDirection::INPUT
                        ? GpioPadDirection::INPUT
                        : GpioPadDirection::OUTPUT
                )
            );

            this->writeRegister(configRegisterValue);
        }

        if (state.direction == TargetGpioPadState::DataDirection::OUTPUT) {
            auto outDataRegisterValue = this->readRegisterDynamicValue(gpioPadDescriptor.outputDataRegisterDescriptor);
            outDataRegisterValue.setBitField(
                gpioPadDescriptor.outputDataBitFieldDescriptor,
                state.value == TargetGpioPadState::State::HIGH
                    ? 0x01
                    : 0x00
            );
            this->writeRegister(outDataRegisterValue);
        }
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
            " segment (" + StringService::formatKey(this->bootProgramSegmentDescriptor.key) + ").\n\n";
        output += leftPadding + "mon " + StringService::applyTerminalColor("program_mode", CMD_COLOR) + " "
            + StringService::applyTerminalColor("user", PARAM_COLOR) + "\n";
        output += leftPadding + "  To switch to user mode, where the mapped program memory segment aliases the main"
            " program segment (" + StringService::formatKey(this->mainProgramSegmentDescriptor.key) + ").\n";

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
                const auto& actualAliasedSegment = this->resolveAliasedMemorySegment();

                if (arguments.size() == 1) {
                    response.output = "Program mode: \"" + StringService::applyTerminalColor(
                        actualAliasedSegment == this->bootProgramSegmentDescriptor ? "boot mode" : "user mode",
                        StringService::TerminalColor::DARK_YELLOW
                    ) + "\"\n";
                    response.output += "Aliased memory segment key: "
                        + StringService::formatKey(actualAliasedSegment.key) + "\n";
                    response.output += "Mapped address -> aliased address: 0x" + StringService::asciiToUpper(
                        StringService::toHex(this->mappedSegmentDescriptor.addressRange.startAddress)
                    ) + " -> 0x" + StringService::asciiToUpper(
                        StringService::toHex(actualAliasedSegment.addressRange.startAddress)
                    ) + "\n";
                    response.output += "Program counter: 0x" + StringService::asciiToUpper(
                        StringService::toHex(this->getProgramCounter())
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
                    response.output += "Program counter: 0x" + StringService::asciiToUpper(
                        StringService::toHex(this->getProgramCounter())
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
                    response.output += "Program counter: 0x" + StringService::asciiToUpper(
                        StringService::toHex(this->getProgramCounter())
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

    DeltaProgramming::DeltaProgrammingInterface* WchRiscV::deltaProgrammingInterface() {
        return this;
    }

    TargetMemorySize WchRiscV::deltaBlockSize(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor
    ) {
        return 64;
    }

    bool WchRiscV::shouldAbandonSession(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        const std::vector<DeltaProgramming::Session::WriteOperation::Region>& deltaSegments
    ) {
        /*
         * Delta programming isn't always faster on WCH RISC-V targets with WCH-Link debug tools.
         *
         * This is because the debug tool can write to program memory with two methods - one better suited for small
         * operations and the other for larger ones. If there are many small delta segments, we'd end up using the
         * slower method many times, resulting in a negative impact on programming speed.
         *
         * For this reason, we abandon delta sessions if they consist of too many delta segments.
         *
         * See WchLinkDebugInterface::writeMemory() for more.
         *
         * TODO: Consider moving this to the WCH-Link driver, seeing as it's specific to that debug tool. In fact, I
         *       think the entire implementation of the DeltaProgrammingInterface should reside in the tool driver, as
         *       opposed to the target driver. Something to look at after v2.0.0.
         */
        return
            deltaSegments.size() > 5
            || std::count_if(
                deltaSegments.begin(),
                deltaSegments.end(),
                [] (const DeltaProgramming::Session::WriteOperation::Region& segment) {
                    return segment.buffer.size() > 192;
                }
            ) > 2;
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

        Logger::debug("Aliased program memory segment: " + Services::StringService::formatKey(segment.key));
        return segment;
    }

    TargetMemoryAddress WchRiscV::deAliasMappedAddress(
        TargetMemoryAddress address,
        const TargetMemorySegmentDescriptor& aliasedSegmentDescriptor
    ) {
        using Services::StringService;

        const auto deAliasedAddress = address - this->mappedSegmentDescriptor.addressRange.startAddress
            + aliasedSegmentDescriptor.addressRange.startAddress;

        Logger::debug(
            "De-aliased mapped program memory address 0x" + StringService::toHex(address) + " to 0x"
                + StringService::toHex(deAliasedAddress) + " (segment: "
                + StringService::formatKey(aliasedSegmentDescriptor.key) + ")"
        );

        return deAliasedAddress;
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

        if (statusRegister.bitFieldAs<bool>(this->flashStatusRegisterFields.bootLock)) {
            throw Exceptions::Exception{"Failed to unlock boot mode field"};
        }

        statusRegister.setBitField(this->flashStatusRegisterFields.bootMode, true);
        this->writeRegister(statusRegister);

        this->reset();
    }

    void WchRiscV::enableUserMode() {
        this->unlockFlash();
        this->unlockBootModeBitField();

        auto statusRegister = this->readRegisterDynamicValue(this->flashStatusRegisterDescriptor);

        if (statusRegister.bitFieldAs<bool>(this->flashStatusRegisterFields.bootLock)) {
            throw Exceptions::Exception{"Failed to unlock boot mode field"};
        }

        statusRegister.setBitField(this->flashStatusRegisterFields.bootMode, false);
        this->writeRegister(statusRegister);

        this->reset();
    }

    void WchRiscV::eraseMainFlashSegment() {
        static constexpr auto ERASE_RESPONSE_DELAY = std::chrono::microseconds{10};
        static constexpr auto ERASE_TIMEOUT = std::chrono::milliseconds{100};

        this->unlockFlash();

        auto statusRegister = this->readRegisterDynamicValue(this->flashStatusRegisterDescriptor);
        if (statusRegister.bitFieldAs<bool>(this->flashStatusRegisterFields.busy)) {
            throw Exceptions::TargetOperationFailure{"Flash peripheral is unavailable"};
        }

        auto controlRegister = this->readRegisterDynamicValue(this->flashControlRegisterDescriptor);
        if (controlRegister.bitFieldAs<bool>(this->flashControlRegisterFields.locked)) {
            throw Exceptions::TargetOperationFailure{"Failed to unlock flash"};
        }

        // It seems we have to write these bit fields individually. Otherwise, the target misbehaves
        controlRegister.setBitField(this->flashControlRegisterFields.mainSegmentErase, true);
        this->writeRegister(controlRegister);
        controlRegister.setBitField(this->flashControlRegisterFields.startErase, true);
        this->writeRegister(controlRegister);

        statusRegister = this->readRegisterDynamicValue(this->flashStatusRegisterDescriptor);
        for (
            auto attempts = 0;
            statusRegister.bitFieldAs<bool>(this->flashStatusRegisterFields.busy)
                && (ERASE_RESPONSE_DELAY * attempts) <= ERASE_TIMEOUT;
            ++attempts
        ) {
            std::this_thread::sleep_for(ERASE_RESPONSE_DELAY);
            statusRegister = this->readRegisterDynamicValue(this->flashStatusRegisterDescriptor);
        }

        controlRegister = this->readRegisterDynamicValue(this->flashControlRegisterDescriptor);
        controlRegister.setBitField(this->flashControlRegisterFields.mainSegmentErase, false);
        controlRegister.setBitField(this->flashControlRegisterFields.startErase, false);
        this->writeRegister(controlRegister);

        if (statusRegister.bitFieldAs<bool>(this->flashStatusRegisterFields.busy)) {
            throw Exceptions::TargetOperationFailure{"Erase operation timed out"};
        }
    }

    std::map<TargetPadId, GpioPadDescriptor> WchRiscV::generateGpioPadDescriptorMapping(
        const TargetRegisterDescriptor& portPeripheralClockEnableRegisterDescriptor,
        const std::vector<TargetPeripheralDescriptor>& portPeripheralDescriptors,
        const std::vector<TargetPadDescriptor>& padDescriptors
    ) {
        static const auto findConfigBitField = [] (
            const TargetPadDescriptor& padDescriptor,
            const TargetRegisterDescriptor& configRegisterDescriptor
        ) -> std::optional<std::reference_wrapper<const TargetBitFieldDescriptor>> {
            return padDescriptor.key.size() >= 3
                ? configRegisterDescriptor.tryGetBitFieldDescriptor("cnf" + padDescriptor.key.substr(2))
                : std::nullopt;
        };

        static const auto findModeBitField = [] (
            const TargetPadDescriptor& padDescriptor,
            const TargetRegisterDescriptor& configRegisterDescriptor
        ) -> std::optional<std::reference_wrapper<const TargetBitFieldDescriptor>> {
            return padDescriptor.key.size() >= 3
                ? configRegisterDescriptor.tryGetBitFieldDescriptor("mode" + padDescriptor.key.substr(2))
                : std::nullopt;
        };

        static const auto findInputDataBitField = [] (
            const TargetPadDescriptor& padDescriptor,
            const TargetRegisterDescriptor& inputDataRegisterDescriptor
        ) -> std::optional<std::reference_wrapper<const TargetBitFieldDescriptor>> {
            return padDescriptor.key.size() >= 3
               ? inputDataRegisterDescriptor.tryGetBitFieldDescriptor("indr" + padDescriptor.key.substr(2))
               : std::nullopt;
        };

        static const auto findOutputDataBitField = [] (
            const TargetPadDescriptor& padDescriptor,
            const TargetRegisterDescriptor& outputDataRegisterDescriptor
        ) -> std::optional<std::reference_wrapper<const TargetBitFieldDescriptor>> {
            return padDescriptor.key.size() >= 3
               ? outputDataRegisterDescriptor.tryGetBitFieldDescriptor("odr" + padDescriptor.key.substr(2))
               : std::nullopt;
        };

        auto output = std::map<TargetPadId, GpioPadDescriptor>{};

        for (const auto& padDesc : padDescriptors) {
            if (padDesc.type != TargetPadType::GPIO) {
                continue;
            }

            for (const auto& peripheralDesc : portPeripheralDescriptors) {
                if (
                    !peripheralDesc.tryGetFirstSignalDescriptor(padDesc.key).has_value()
                    || peripheralDesc.key.size() < 5
                    || peripheralDesc.key.find("port") != 0
                ) {
                    continue;
                }

                const auto portLetter = peripheralDesc.key.substr(4, 1);
                const auto peripheralClockEnableBitFieldDescOpt = portPeripheralClockEnableRegisterDescriptor.tryGetBitFieldDescriptor(
                    "iop" + portLetter + "en"
                );

                if (!peripheralClockEnableBitFieldDescOpt.has_value()) {
                    continue;
                }

                auto configRegisterDescOpt = std::optional<std::reference_wrapper<const TargetRegisterDescriptor>>{};
                auto configBitFieldDescOpt = std::optional<std::reference_wrapper<const TargetBitFieldDescriptor>>{};
                auto modeBitFieldDescOpt = std::optional<std::reference_wrapper<const TargetBitFieldDescriptor>>{};

                const auto portGroupDescOpt = peripheralDesc.tryGetRegisterGroupDescriptor("port");
                if (!portGroupDescOpt.has_value()) {
                    continue;
                }

                const auto& portGroupDescriptor = portGroupDescOpt->get();

                const auto configLowRegisterDescOpt = portGroupDescriptor.tryGetRegisterDescriptor("cfglr");
                const auto configHighRegisterDescOpt = portGroupDescriptor.tryGetRegisterDescriptor("cfghr");
                const auto configExtendedRegisterDescOpt = portGroupDescriptor.tryGetRegisterDescriptor("cfgxr");

                if (configLowRegisterDescOpt.has_value()) {
                    const auto& configLowRegisterDescriptor = configLowRegisterDescOpt->get();

                    configBitFieldDescOpt = findConfigBitField(padDesc, configLowRegisterDescriptor);
                    modeBitFieldDescOpt = findModeBitField(padDesc, configLowRegisterDescriptor);

                    if (configBitFieldDescOpt.has_value()) {
                        configRegisterDescOpt = configLowRegisterDescOpt;
                    }
                }

                if (
                    (!configBitFieldDescOpt.has_value() || !modeBitFieldDescOpt.has_value())
                    && configHighRegisterDescOpt.has_value()
                ) {
                    const auto& configHighRegisterDescriptor = configHighRegisterDescOpt->get();

                    configBitFieldDescOpt = findConfigBitField(padDesc, configHighRegisterDescriptor);
                    modeBitFieldDescOpt = findModeBitField(padDesc, configHighRegisterDescriptor);

                    if (configBitFieldDescOpt.has_value()) {
                        configRegisterDescOpt = configHighRegisterDescOpt;
                    }
                }

                if (
                    (!configBitFieldDescOpt.has_value() || !modeBitFieldDescOpt.has_value())
                    && configExtendedRegisterDescOpt.has_value()
                ) {
                    const auto& configExtendedRegisterDescriptor = configExtendedRegisterDescOpt->get();

                    configBitFieldDescOpt = findConfigBitField(padDesc, configExtendedRegisterDescriptor);
                    modeBitFieldDescOpt = findModeBitField(padDesc, configExtendedRegisterDescriptor);

                    if (configBitFieldDescOpt.has_value()) {
                        configRegisterDescOpt = configExtendedRegisterDescOpt;
                    }
                }

                if (
                    !configRegisterDescOpt.has_value()
                    || !configBitFieldDescOpt.has_value()
                    || !modeBitFieldDescOpt.has_value()
                ) {
                    continue;
                }

                const auto inputDataRegisterDescOpt = portGroupDescriptor.tryGetRegisterDescriptor("indr");
                const auto outputDataRegisterDescOpt = portGroupDescriptor.tryGetRegisterDescriptor("outdr");

                if (!inputDataRegisterDescOpt.has_value() || !outputDataRegisterDescOpt.has_value()) {
                    continue;
                }

                const auto& inputDataRegisterDescriptor = inputDataRegisterDescOpt->get();
                const auto& outputDataRegisterDescriptor = outputDataRegisterDescOpt->get();

                const auto inputBitFieldDescOpt = findInputDataBitField(padDesc, inputDataRegisterDescriptor);
                const auto outputBitFieldDescOpt = findOutputDataBitField(padDesc, outputDataRegisterDescriptor);

                if (!inputBitFieldDescOpt.has_value() || !outputBitFieldDescOpt.has_value()) {
                    continue;
                }

                output.emplace(
                    padDesc.id,
                    GpioPadDescriptor{
                        .peripheralClockEnableBitFieldDescriptor = peripheralClockEnableBitFieldDescOpt->get(),
                        .configRegisterDescriptor = configRegisterDescOpt->get(),
                        .configBitFieldDescriptor = configBitFieldDescOpt->get(),
                        .modeBitFieldDescriptor = modeBitFieldDescOpt->get(),
                        .inputDataRegisterDescriptor = inputDataRegisterDescriptor,
                        .inputDataBitFieldDescriptor = inputBitFieldDescOpt->get(),
                        .outputDataRegisterDescriptor = outputDataRegisterDescriptor,
                        .outputDataBitFieldDescriptor = outputBitFieldDescOpt->get(),
                    }
                );

                break;
            }
        }

        return output;
    }
}
