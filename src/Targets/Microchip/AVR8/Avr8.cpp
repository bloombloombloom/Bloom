#include "Avr8.hpp"

#include <cassert>
#include <bitset>
#include <limits>
#include <thread>
#include <algorithm>
#include <optional>
#include <functional>
#include <unordered_map>

#include "IspParameters.hpp"

#include "src/Logger/Logger.hpp"
#include "src/Services/PathService.hpp"
#include "src/Services/StringService.hpp"

#include "src/Exceptions/InvalidConfig.hpp"
#include "Exceptions/DebugWirePhysicalInterfaceError.hpp"

namespace Targets::Microchip::Avr8
{
    using namespace Exceptions;

    Avr8::Avr8(const TargetConfig& targetConfig, TargetDescriptionFile&& targetDescriptionFile)
        : targetConfig(Avr8TargetConfig{targetConfig})
        , targetDescriptionFile(std::move(targetDescriptionFile))
        , dataAddressSpaceDescriptor(this->targetDescriptionFile.getDataAddressSpaceDescriptor())
        , fuseAddressSpaceDescriptor(this->targetDescriptionFile.getFuseAddressSpaceDescriptor())
        , ramMemorySegmentDescriptor(this->targetDescriptionFile.getRamMemorySegmentDescriptor())
        , ioMemorySegmentDescriptor(this->targetDescriptionFile.getIoMemorySegmentDescriptor())
        , fuseMemorySegmentDescriptor(this->targetDescriptionFile.getFuseMemorySegmentDescriptor())
        , signature(this->targetDescriptionFile.getTargetSignature())
        , family(this->targetDescriptionFile.getAvrFamily())
        , physicalInterfaces(this->targetDescriptionFile.getPhysicalInterfaces())
        , gpioPortPeripheralDescriptors(this->targetDescriptionFile.gpioPortPeripheralDescriptors())
        , gpioPadDescriptorsByPadId(Avr8::generateGpioPadDescriptorMapping(this->gpioPortPeripheralDescriptors))
        , fuseEnableStrategy(this->targetDescriptionFile.getFuseEnableStrategy().value_or(FuseEnableStrategy::CLEAR))
    {
        const auto cpuPeripheralDescriptor = this->targetDescriptionFile.getTargetPeripheralDescriptor("cpu");
        const auto& cpuRegisterGroup = cpuPeripheralDescriptor.getRegisterGroupDescriptor("cpu");

        const auto spDescriptor = cpuRegisterGroup.tryGetRegisterDescriptor("sp");
        if (spDescriptor.has_value()) {
            this->spRegisterDescriptor.emplace(spDescriptor->get().clone());
        }

        const auto spLowDescriptor = cpuRegisterGroup.tryGetRegisterDescriptor("spl");
        if (spLowDescriptor.has_value()) {
            this->spLowRegisterDescriptor.emplace(spLowDescriptor->get().clone());
        }

        const auto spHighDescriptor = cpuRegisterGroup.tryGetRegisterDescriptor("sph");
        if (spHighDescriptor.has_value()) {
            this->spHighRegisterDescriptor.emplace(spHighDescriptor->get().clone());
        }

        if (!this->physicalInterfaces.contains(this->targetConfig.physicalInterface)) {
            /*
             * The user has selected a physical interface that does not appear to be supported by the selected
             * target.
             *
             * Bloom's target description files provide a list of supported physical interfaces for each target
             * (which is how this->physicalInterfaces is populated), but it's possible that this list may
             * be wrong/incomplete. For this reason, we don't throw an exception here. Instead, we just present the
             * user with a warning and a list of physical interfaces known to be supported by their selected target.
             *
             * If the target truly doesn't'support the physical interface, an exception will be thrown during
             * activation.
             */
            const auto physicalInterfaceNames = getPhysicalInterfaceNames();

            const auto supportedPhysicalInterfaceList = std::accumulate(
                this->physicalInterfaces.begin(),
                this->physicalInterfaces.end(),
                std::string{},
                [&physicalInterfaceNames] (const std::string& string, TargetPhysicalInterface physicalInterface) {
                    if (physicalInterface == TargetPhysicalInterface::ISP) {
                        /*
                         * Don't include the ISP interface in the list of supported interfaces, as doing so may
                         * mislead the user into thinking the ISP interface can be used for debugging operations.
                         */
                        return string;
                    }

                    return string + "\n - " + physicalInterfaceNames.at(physicalInterface);
                }
            );

            Logger::warning(
                "\nThe selected target does not support the selected physical interface ("
                    + physicalInterfaceNames.at(this->targetConfig.physicalInterface) + "). Target activation "
                    "will likely fail. The target supports the following physical interfaces: \n"
                    + supportedPhysicalInterfaceList + "\n\nFor physical interface configuration values, see "
                    + Services::PathService::homeDomainName() + "/docs/configuration/target-physical-interfaces."
            );
        }

        if (
            this->targetConfig.manageOcdenFuseBit
            && this->targetConfig.physicalInterface != TargetPhysicalInterface::JTAG
        ) {
            Logger::warning(
                "The 'manageOcdenFuseBit' parameter only applies to JTAG targets. It will be ignored in this session."
            );
        }
    }

    bool Avr8::supportsDebugTool(DebugTool* debugTool) {
        return debugTool->getAvr8DebugInterface(
            this->targetDescriptionFile,
            this->targetConfig
        ) != nullptr;
    }

    void Avr8::setDebugTool(DebugTool* debugTool) {
        this->targetPowerManagementInterface = debugTool->getTargetPowerManagementInterface();
        this->avr8DebugInterface = debugTool->getAvr8DebugInterface(this->targetDescriptionFile, this->targetConfig);

        if (this->physicalInterfaces.contains(TargetPhysicalInterface::ISP)) {
            this->avrIspInterface = debugTool->getAvrIspInterface(this->targetDescriptionFile, this->targetConfig);

            if (
                this->avrIspInterface == nullptr
                && this->targetConfig.manageDwenFuseBit
                && this->targetConfig.physicalInterface == TargetPhysicalInterface::DEBUG_WIRE
            ) {
                Logger::warning(
                    "The connected debug tool (or associated driver) does not provide any ISP interface. "
                        "Bloom will be unable to manage the DWEN fuse bit."
                );
            }
        }
    }

    void Avr8::activate() {
        if (this->activated) {
            return;
        }

        this->avr8DebugInterface->init();

        try {
            this->avr8DebugInterface->activate();

        } catch (const Exceptions::DebugWirePhysicalInterfaceError& debugWireException) {
            // We failed to activate the debugWIRE physical interface. DWEN fuse bit may need updating.

            if (!this->targetConfig.manageDwenFuseBit) {
                throw TargetOperationFailure{
                    "Failed to activate debugWIRE physical interface - check target connection and DWEN fuse "
                        "bit. Bloom can manage the DWEN fuse bit automatically. For instructions on enabling this "
                        "function, see " + Services::PathService::homeDomainName() + "/docs/debugging-avr-debugwire"
                };
            }

            try {
                Logger::warning(
                    "Failed to activate the debugWIRE physical interface - attempting to access target via "
                        "the ISP interface, for DWEN fuse bit inspection."
                );
                this->updateDwenFuseBit(true);

                // If the debug tool provides a TargetPowerManagementInterface, attempt to cycle the target power
                if (
                    this->targetPowerManagementInterface != nullptr
                    && this->targetConfig.cycleTargetPowerPostDwenUpdate
                ) {
                    Logger::info("Cycling target power");

                    Logger::debug("Disabling target power");
                    this->targetPowerManagementInterface->disableTargetPower();

                    Logger::debug(
                        "Holding power off for ~" + std::to_string(this->targetConfig.targetPowerCycleDelay.count())
                            + " ms"
                    );
                    std::this_thread::sleep_for(this->targetConfig.targetPowerCycleDelay);

                    Logger::debug("Enabling target power");
                    this->targetPowerManagementInterface->enableTargetPower();

                    Logger::debug(
                        "Waiting ~" + std::to_string(this->targetConfig.targetPowerCycleDelay.count())
                            + " ms for target power-up"
                    );
                    std::this_thread::sleep_for(this->targetConfig.targetPowerCycleDelay);
                }

            } catch (const Exception& exception) {
                throw Exception{
                    "Failed to access/update DWEN fuse bit via ISP interface - " + exception.getMessage()
                };
            }

            Logger::info("Retrying debugWIRE physical interface activation");
            this->avr8DebugInterface->activate();
        }

        this->stop();
        this->reset();

        if (
            this->targetConfig.physicalInterface == TargetPhysicalInterface::JTAG
            && this->targetConfig.manageOcdenFuseBit
        ) {
            Logger::debug("Attempting OCDEN fuse bit management");
            this->updateOcdenFuseBit(true);
        }

        this->activated = true;

        /*
         * Validate the target signature.
         *
         * The signature obtained from the device should match what we loaded from the target description file.
         */
        const auto targetSignature = this->avr8DebugInterface->getDeviceId();

        if (targetSignature != this->signature) {
            throw Exception{
                "Failed to validate connected target - target signature mismatch.\nThe target signature"
                    " (\"" + targetSignature.toHex() + "\") does not match the AVR8 target description signature (\""
                    + this->signature.toHex() + "\"). This will likely be due to an incorrect target name in the "
                    + "configuration file (bloom.yaml)."
            };
        }
    }

    void Avr8::deactivate() {
        try {
            if (this->avr8DebugInterface == nullptr) {
                return;
            }

            this->stop();
            this->clearAllBreakpoints();

            if (
                this->targetConfig.physicalInterface == TargetPhysicalInterface::JTAG
                && this->targetConfig.manageOcdenFuseBit
            ) {
                Logger::debug("Attempting OCDEN fuse bit management");
                this->updateOcdenFuseBit(false);

            } else {
                this->avr8DebugInterface->deactivate();
            }

            this->activated = false;

        } catch (const Exception& exception) {
            Logger::error("Failed to deactivate AVR8 target - " + exception.getMessage());
        }
    }

    void Avr8::postActivate() {
        Logger::info("AVR signature: " + this->avr8DebugInterface->getDeviceId().toHex());
    }

    TargetDescriptor Avr8::targetDescriptor() {
        auto descriptor = TargetDescriptor{
            this->targetDescriptionFile.getName(),
            this->targetDescriptionFile.getFamily(),
            this->signature.toHex(),
            this->targetDescriptionFile.tryGetVendorName().value_or("Microchip"),
            this->targetDescriptionFile.targetAddressSpaceDescriptorsByKey(),
            this->targetDescriptionFile.targetPeripheralDescriptorsByKey(),
            this->targetDescriptionFile.targetPadDescriptorsByKey(),
            this->targetDescriptionFile.targetPinoutDescriptorsByKey(),
            this->targetDescriptionFile.targetVariantDescriptorsByKey(),
            this->getBreakpointResources()
        };

        /*
         * General purpose CPU registers are not included in AVR8 TDFs, so we manually add them to the target
         * descriptor here.
         */
        const auto& registerFileAddressSpace = this->targetDescriptionFile.getRegisterFileAddressSpace();
        const auto& registerFileMemorySegment = registerFileAddressSpace.getMemorySegment("gp_registers");

        auto& gpPeripheral = descriptor.peripheralDescriptorsByKey.emplace(
            "cpu_gpr",
            TargetPeripheralDescriptor{
                "cpu_gpr",
                "GPRs",
                "CPU General Purpose Registers",
                {},
                {}
            }
        ).first->second;

        auto& gpRegisterGroup = gpPeripheral.registerGroupDescriptorsByKey.emplace(
            "gpr",
            TargetRegisterGroupDescriptor{
                "gpr",
                "gpr",
                "CPU General Purpose",
                gpPeripheral.key,
                registerFileAddressSpace.key,
                std::nullopt,
                {},
                {}
            }
        ).first->second;

        for (auto i = std::uint8_t{0}; i <= 31; ++i) {
            const auto key = "r" + std::to_string(i);
            gpRegisterGroup.registerDescriptorsByKey.emplace(
                key,
                TargetRegisterDescriptor{
                    key,
                    "R" + std::to_string(i),
                    gpRegisterGroup.absoluteKey,
                    gpPeripheral.key,
                    registerFileAddressSpace.key,
                    registerFileMemorySegment.startAddress + i,
                    1,
                    TargetRegisterType::GENERAL_PURPOSE_REGISTER,
                    TargetRegisterAccess(true, true),
                    std::nullopt,
                    {}
                }
            );
        }

        // The debug interface may have its own access restrictions for registers.
        for (auto& [peripheralKey, peripheral] : descriptor.peripheralDescriptorsByKey) {
            for (auto& [groupKey, registerGroup] : peripheral.registerGroupDescriptorsByKey) {
                this->applyDebugInterfaceRegisterAccessRestrictions(
                    registerGroup,
                    descriptor.getAddressSpaceDescriptor(registerGroup.addressSpaceKey)
                );
            }
        }

        return descriptor;
    }

    void Avr8::run(std::optional<TargetMemoryAddress> toAddress) {
        if (toAddress.has_value()) {
            return this->avr8DebugInterface->runTo(*toAddress);
        }

        this->avr8DebugInterface->run();
    }

    void Avr8::stop() {
        this->avr8DebugInterface->stop();
    }

    void Avr8::step() {
        this->avr8DebugInterface->step();
    }

    void Avr8::reset() {
        this->avr8DebugInterface->reset();
    }

    void Avr8::setSoftwareBreakpoint(TargetMemoryAddress address) {
        this->avr8DebugInterface->setSoftwareBreakpoint(address);
    }

    void Avr8::removeSoftwareBreakpoint(TargetMemoryAddress address) {
        this->avr8DebugInterface->clearSoftwareBreakpoint(address);
    }

    void Avr8::setHardwareBreakpoint(TargetMemoryAddress address) {
        this->avr8DebugInterface->setHardwareBreakpoint(address);
    }

    void Avr8::removeHardwareBreakpoint(TargetMemoryAddress address) {
        this->avr8DebugInterface->clearHardwareBreakpoint(address);
    }

    void Avr8::clearAllBreakpoints() {
        this->avr8DebugInterface->clearAllBreakpoints();
    }

    TargetRegisterDescriptorAndValuePairs Avr8::readRegisters(const Targets::TargetRegisterDescriptors& descriptors) {
        return this->avr8DebugInterface->readRegisters(descriptors);
    }

    void Avr8::writeRegisters(const TargetRegisterDescriptorAndValuePairs& registers) {
        this->avr8DebugInterface->writeRegisters(registers);
    }

    TargetMemoryBuffer Avr8::readMemory(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        TargetMemoryAddress startAddress,
        TargetMemorySize bytes,
        const std::set<Targets::TargetMemoryAddressRange>& excludedAddressRanges
    ) {
        return this->avr8DebugInterface->readMemory(
            addressSpaceDescriptor,
            memorySegmentDescriptor,
            startAddress,
            bytes,
            excludedAddressRanges
        );
    }

    void Avr8::writeMemory(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        std::uint32_t startAddress,
        TargetMemoryBufferSpan buffer
    ) {
        if (memorySegmentDescriptor.type == TargetMemorySegmentType::FLASH && !this->programmingModeEnabled()) {
            throw Exception{"Attempted Flash memory write in the absence of an active programming session."};
        }

        this->avr8DebugInterface->writeMemory(addressSpaceDescriptor, memorySegmentDescriptor, startAddress, buffer);
    }

    bool Avr8::isProgramMemory(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        TargetMemoryAddress startAddress,
        TargetMemorySize size
    ) {
        /*
         * On AVR8 targets, memory segments that are marked as executable are executable in their entirety.
         * No need for more granular checks here.
         */
        return memorySegmentDescriptor.executable;
    }

    void Avr8::eraseMemory(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor
    ) {
        if (memorySegmentDescriptor.type == TargetMemorySegmentType::FLASH) {
            if (!this->programmingModeEnabled()) {
                throw Exception{"Attempted flash memory erase in the absence of an active programming session"};
            }

            if (this->targetConfig.physicalInterface == TargetPhysicalInterface::DEBUG_WIRE) {
                // debugWIRE targets do not need to be erased
                return;
            }

            /*
             * To erase program memory on JTAG and UPDI targets, we must perform a chip erase. This means we could
             * end up erasing EEPROM, unless the EESAVE fuse bit has been programmed.
             *
             * If configured to do so, we will ensure that the EESAVE fuse bit has been programmed before we perform
             * the chip erase. The fuse will be restored to its original value at the end of the programming session.
             */
            if (
                this->targetConfig.physicalInterface == TargetPhysicalInterface::JTAG
                || this->targetConfig.physicalInterface == TargetPhysicalInterface::UPDI
            ) {
                if (this->targetConfig.preserveEeprom) {
                    Logger::debug("Inspecting EESAVE fuse bit");
                    this->activeProgrammingSession->managingEesaveFuseBit = this->updateEesaveFuseBit(true);

                } else {
                    Logger::warning(
                        "Performing chip-erase with preserveEeprom disabled. All EEPROM data will be lost!"
                    );
                }

                return this->avr8DebugInterface->eraseChip();
            }

            return this->avr8DebugInterface->eraseProgramMemory();
        }

        /*
         * The debug interface does not have to support the erasing of RAM or EEPROM memory. We just implement this as
         * a write operation.
         */
        this->writeMemory(
            addressSpaceDescriptor,
            memorySegmentDescriptor,
            memorySegmentDescriptor.addressRange.startAddress,
            TargetMemoryBuffer(memorySegmentDescriptor.size(), 0xFF)
        );
    }

    TargetExecutionState Avr8::getExecutionState() {
        return this->avr8DebugInterface->getExecutionState();
    }

    TargetMemoryAddress Avr8::getProgramCounter() {
        return this->avr8DebugInterface->getProgramCounter();
    }

    void Avr8::setProgramCounter(TargetMemoryAddress programCounter) {
        this->avr8DebugInterface->setProgramCounter(programCounter);
    }

    TargetStackPointer Avr8::getStackPointer() {
        auto descriptors = TargetRegisterDescriptors{};

        if (this->spRegisterDescriptor.has_value()) {
            descriptors.push_back(&*(this->spRegisterDescriptor));
        }

        if (this->spLowRegisterDescriptor.has_value()) {
            descriptors.push_back(&*(this->spLowRegisterDescriptor));
        }

        if (this->spHighRegisterDescriptor.has_value()) {
            descriptors.push_back(&*(this->spHighRegisterDescriptor));
        }

        auto output = TargetStackPointer{0};

        for (const auto& [descriptor, value] : this->readRegisters(descriptors)) {
            if (
                this->spHighRegisterDescriptor.has_value()
                && descriptor.startAddress == this->spHighRegisterDescriptor->startAddress
            ) {
                // SP high byte
                assert(value.size() == 1);
                output = (output & 0x000000FF) | static_cast<TargetStackPointer>(value[0] << 8);

            } else {
                assert(value.size() > 0 && value.size() <= 2);
                for (auto i = std::size_t{0}; i < value.size(); ++i) {
                    output = (output << (8 * i)) | value[i];
                }
            }
        }

        return output;
    }

    void Avr8::setStackPointer(TargetStackPointer stackPointer) {
        if (this->spRegisterDescriptor.has_value()) {
            this->writeRegister(
                *(this->spRegisterDescriptor),
                this->spRegisterDescriptor->size > 1
                    ? TargetMemoryBuffer({
                        static_cast<unsigned char>(stackPointer >> 8),
                        static_cast<unsigned char>(stackPointer)
                    })
                    : TargetMemoryBuffer({static_cast<unsigned char>(stackPointer)})
            );
        }

        if (this->spLowRegisterDescriptor.has_value()) {
            this->writeRegister(
                *(this->spLowRegisterDescriptor),
                TargetMemoryBuffer({static_cast<unsigned char>(stackPointer)})
            );
        }

        if (this->spHighRegisterDescriptor.has_value()) {
            this->writeRegister(
                *(this->spHighRegisterDescriptor),
                TargetMemoryBuffer({static_cast<unsigned char>(stackPointer >> 8)})
            );
        }
    }

    TargetGpioPadDescriptorAndStatePairs Avr8::getGpioPadStates(const TargetPadDescriptors& padDescriptors) {
        auto output = TargetGpioPadDescriptorAndStatePairs{};

        // To reduce the number of memory reads we perform here, we cache the data and map it by start address.
        auto cachedRegsByStartAddress = std::unordered_map<TargetMemoryAddress, unsigned char>{};
        const auto readGpioReg = [this, &cachedRegsByStartAddress] (const TargetRegisterDescriptor& descriptor) {
            assert(descriptor.size == 1);

            auto cachedRegIt = cachedRegsByStartAddress.find(descriptor.startAddress);
            if (cachedRegIt == cachedRegsByStartAddress.end()) {
                cachedRegIt = cachedRegsByStartAddress.emplace(
                    descriptor.startAddress,
                    this->readRegister(descriptor).at(0)
                ).first;
            }

            return cachedRegIt->second;
        };

        for (const auto* padDescriptor : padDescriptors) {
            if (padDescriptor->type != TargetPadType::GPIO) {
                continue;
            }

            const auto gpioPadDescriptorIt = this->gpioPadDescriptorsByPadId.find(padDescriptor->id);
            if (gpioPadDescriptorIt == this->gpioPadDescriptorsByPadId.end()) {
                continue;
            }

            const auto& gpioPadDescriptor = gpioPadDescriptorIt->second;
            const auto ddrValue = (
                readGpioReg(gpioPadDescriptor.dataDirectionRegisterDescriptor) & gpioPadDescriptor.registerMask
            ) != 0 ? TargetGpioPadState::DataDirection::OUTPUT : TargetGpioPadState::DataDirection::INPUT;

            const auto& stateRegisterDescriptor = ddrValue == TargetGpioPadState::DataDirection::OUTPUT
                ? gpioPadDescriptor.outputRegisterDescriptor
                : gpioPadDescriptor.inputRegisterDescriptor;

            output.emplace_back(
                TargetGpioPadDescriptorAndStatePair{
                    *padDescriptor,
                    TargetGpioPadState{
                        (readGpioReg(stateRegisterDescriptor) & gpioPadDescriptor.registerMask) != 0
                            ? TargetGpioPadState::State::HIGH
                            : TargetGpioPadState::State::LOW,
                        ddrValue
                    }
                }
            );
        }

        return output;
    }

    void Avr8::setGpioPadState(const TargetPadDescriptor& padDescriptor, const TargetGpioPadState& state) {
        using DataDirection = TargetGpioPadState::DataDirection;
        using GpioState = TargetGpioPadState::State;

        const auto gpioPadDescriptorIt = this->gpioPadDescriptorsByPadId.find(padDescriptor.id);
        if (gpioPadDescriptorIt == this->gpioPadDescriptorsByPadId.end()) {
            throw Exception{"Unknown pad"};
        }

        const auto& gpioPadDescriptor = gpioPadDescriptorIt->second;

        const auto currentDdrValue = this->readRegister(gpioPadDescriptor.dataDirectionRegisterDescriptor).at(0);
        this->writeRegister(
            gpioPadDescriptor.dataDirectionRegisterDescriptor,
            {
                static_cast<unsigned char>(
                    state.direction == DataDirection::OUTPUT
                        ? (currentDdrValue | gpioPadDescriptor.registerMask)
                        : (currentDdrValue & ~(gpioPadDescriptor.registerMask))
                )
            }
        );

        if (state.direction == DataDirection::OUTPUT) {
            const auto currentOutputValue = this->readRegister(gpioPadDescriptor.outputRegisterDescriptor).at(0);
            this->writeRegister(
                gpioPadDescriptor.outputRegisterDescriptor,
                {
                    static_cast<unsigned char>(
                        state.value == GpioState::HIGH
                            ? (currentOutputValue | gpioPadDescriptor.registerMask)
                            : (currentOutputValue & ~(gpioPadDescriptor.registerMask))
                    )
                }
            );
        }
    }

    void Avr8::enableProgrammingMode() {
        if (this->activeProgrammingSession.has_value()) {
            return;
        }

        this->avr8DebugInterface->enableProgrammingMode();
        this->activeProgrammingSession = ProgrammingSession();
    }

    void Avr8::disableProgrammingMode() {
        if (!this->activeProgrammingSession.has_value()) {
            return;
        }

        if (this->activeProgrammingSession->managingEesaveFuseBit) {
            this->updateEesaveFuseBit(false);
        }

        this->avr8DebugInterface->disableProgrammingMode();
        this->stop();
        this->activeProgrammingSession.reset();
    }

    bool Avr8::programmingModeEnabled() {
        return this->activeProgrammingSession.has_value();
    }

    std::map<TargetPadId, GpioPadDescriptor> Avr8::generateGpioPadDescriptorMapping(
        const std::vector<TargetPeripheralDescriptor>& portPeripheralDescriptors
    ) {
        auto output = std::map<TargetPadId, GpioPadDescriptor>{};

        for (const auto& peripheralDescriptor : portPeripheralDescriptors) {
            if (peripheralDescriptor.registerGroupDescriptorsByKey.empty()) {
                continue;
            }

            for (const auto& signalDescriptor : peripheralDescriptor.signalDescriptors) {
                if (!signalDescriptor.index.has_value()) {
                    continue;
                }

                if (output.contains(signalDescriptor.padId)) {
                    continue;
                }

                const auto registerMask = static_cast<std::uint8_t>(0x01 << *(signalDescriptor.index));

                /*
                 * All port peripherals should only have a single register group instance pointing to the port register
                 * group. This is enforced in the TDF validation script.
                 *
                 * The key of the register group instance varies across peripherals, which is why we use begin() as
                 * opposed to performing a key lookup.
                 */
                const auto& portRegisterGroup = peripheralDescriptor.registerGroupDescriptorsByKey.begin()->second;

                // From a register layout perspective, there are two types of GPIO port modules on AVR8 targets.
                if (portRegisterGroup.registerDescriptorsByKey.contains("outset")) {
                    output.emplace(
                        signalDescriptor.padId,
                        GpioPadDescriptor{
                            signalDescriptor.padKey,
                            registerMask,
                            portRegisterGroup.getRegisterDescriptor("dir"),
                            portRegisterGroup.getRegisterDescriptor("in"),
                            portRegisterGroup.getRegisterDescriptor("out")
                        }
                    );

                    continue;
                }

                /*
                 * The older GPIO port module is a little trickier, as it has a dedicated register group for each
                 * instance of the module (e.g. one for PORTA, another for PORTB, etc.).
                 *
                 * And the register keys are inconsistent ("ddra", "ddrb", etc.).
                 */
                auto ddrDescriptor = std::optional<std::reference_wrapper<const TargetRegisterDescriptor>>{};
                auto inputDescriptor = std::optional<std::reference_wrapper<const TargetRegisterDescriptor>>{};
                auto outputDescriptor = std::optional<std::reference_wrapper<const TargetRegisterDescriptor>>{};

                for (const auto& [registerKey, registerDescriptor] : portRegisterGroup.registerDescriptorsByKey) {
                    if (registerKey.find("ddr") == 0) {
                        ddrDescriptor = std::cref(registerDescriptor);
                        continue;
                    }

                    if (registerKey.find("pin") == 0) {
                        inputDescriptor = std::cref(registerDescriptor);
                        continue;
                    }

                    if (registerKey.find("port") == 0) {
                        outputDescriptor = std::cref(registerDescriptor);
                    }
                }

                if (ddrDescriptor.has_value() && inputDescriptor.has_value() && outputDescriptor.has_value()) {
                    output.emplace(
                        signalDescriptor.padId,
                        GpioPadDescriptor{
                            signalDescriptor.padKey,
                            registerMask,
                            ddrDescriptor->get(),
                            inputDescriptor->get(),
                            outputDescriptor->get()
                        }
                    );
                }
            }
        }

        return output;
    }

    TargetMemoryBuffer Avr8::readRegister(const TargetRegisterDescriptor& descriptor) {
        return this->readRegisters({&descriptor}).at(0).second;
    }

    void Avr8::writeRegister(const TargetRegisterDescriptor& descriptor, const TargetMemoryBuffer& value) {
        this->writeRegisters({{descriptor, value}});
    }

    void Avr8::applyDebugInterfaceRegisterAccessRestrictions(
        TargetRegisterGroupDescriptor& groupDescriptor,
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor
    ) {
        for (auto& [subgroupKey, subgroupDescriptor] : groupDescriptor.subgroupDescriptorsByKey) {
            this->applyDebugInterfaceRegisterAccessRestrictions(subgroupDescriptor, addressSpaceDescriptor);
        }

        for (auto& [registerKey, registerDescriptor] : groupDescriptor.registerDescriptorsByKey) {
            if (!registerDescriptor.access.readable && !registerDescriptor.access.writable) {
                // This register is already inaccessible - no need to check for further restrictions
                continue;
            }

            const auto access = this->avr8DebugInterface->getRegisterAccess(registerDescriptor, addressSpaceDescriptor);
            registerDescriptor.access.readable = registerDescriptor.access.readable && access.readable;
            registerDescriptor.access.writable = registerDescriptor.access.writable && access.writable;
        }
    }

    BreakpointResources Avr8::getBreakpointResources() {
        auto maxHardwareBreakpoints = std::uint16_t{0};

        switch (this->targetConfig.physicalInterface) {
            case TargetPhysicalInterface::JTAG: {
                maxHardwareBreakpoints = this->family == Family::XMEGA ? 2 : 3;
                break;
            }
            case TargetPhysicalInterface::PDI: {
                maxHardwareBreakpoints = 2;
                break;
            }
            case TargetPhysicalInterface::UPDI: {
                maxHardwareBreakpoints = 1;
                break;
            }
            default: {
                break;
            }
        }

        return BreakpointResources{
            maxHardwareBreakpoints,
            std::nullopt,
            std::min(
                static_cast<std::uint16_t>(this->targetConfig.reserveSteppingBreakpoint.value_or(true) ? 1 : 0),
                maxHardwareBreakpoints
            )
        };
    }

    bool Avr8::isFuseEnabled(const TargetBitFieldDescriptor& bitFieldDescriptor, FuseValue value) const {
        const auto programmedValue = static_cast<unsigned char>(
            this->fuseEnableStrategy == FuseEnableStrategy::SET
                ? (0xFF & bitFieldDescriptor.mask)
                : 0
        );

        return (value & bitFieldDescriptor.mask) == programmedValue;
    }

    FuseValue Avr8::setFuseEnabled(
        const TargetBitFieldDescriptor& bitFieldDescriptor,
        FuseValue value,
        bool enabled
    ) const {
        return static_cast<FuseValue>(
            this->fuseEnableStrategy == FuseEnableStrategy::SET
                ? enabled
                    ? (value | bitFieldDescriptor.mask)
                    : value & ~(bitFieldDescriptor.mask)
                : enabled
                    ? value & ~(bitFieldDescriptor.mask)
                    : (value | bitFieldDescriptor.mask)
        );
    }

    void Avr8::updateDwenFuseBit(bool enable) {
        if (this->avrIspInterface == nullptr) {
            throw Exception{
                "Debug tool or driver does not provide access to an ISP interface - please confirm that the "
                    "debug tool supports ISP and then report this issue via " + Services::PathService::homeDomainName()
                    + "/report-issue"
            };
        }

        if (!this->physicalInterfaces.contains(TargetPhysicalInterface::DEBUG_WIRE)) {
            throw Exception{
                "Target does not support debugWIRE physical interface - check target configuration or "
                    "report this issue via " + Services::PathService::homeDomainName() + "/report-issue"
            };
        }

        const auto dwenFuseBitFieldPair = this->targetDescriptionFile.getFuseRegisterBitFieldDescriptorPair("dwen");
        const auto& dwenRegisterDescriptor = dwenFuseBitFieldPair.first;
        const auto& dwenBitFieldDescriptor = dwenFuseBitFieldPair.second;

        const auto spienFuseBitFieldPair = this->targetDescriptionFile.getFuseRegisterBitFieldDescriptorPair("spien");
        const auto& spienRegisterDescriptor = spienFuseBitFieldPair.first;
        const auto& spienBitFieldDescriptor = spienFuseBitFieldPair.second;

        assert(dwenRegisterDescriptor.size == 1);
        assert(spienRegisterDescriptor.size == 1);

        Logger::info("Initiating ISP interface");
        this->avrIspInterface->activate();

        /*
         * AVR fuses are used to control certain functions within the AVR (including the debugWIRE interface). Care
         * must be taken when updating these fuse bytes, as an incorrect value could render the AVR inaccessible to
         * standard programmers.
         *
         * When updating the DWEN fuse, Bloom relies on data from the target description file (TDF). But there is no
         * guarantee that this data is correct. For this reason, we perform additional checks in an attempt to reduce
         * the likelihood of bricking the target:
         *
         *  - Confirm target signature match - We read the AVR signature from the connected target and compare it to
         *    what we have in the TDF. The operation will be aborted if the signatures do not match.
         *
         *  - SPIEN fuse bit check - we can be certain that the SPIEN fuse bit is set, because we couldn't have gotten
         *    this far (post ISP activation) if it wasn't. We use this axiom to verify the validity of the data in the
         *    TDF. If the SPIEN fuse bit appears to be cleared, we can be fairly certain that the data we have on the
         *    SPIEN fuse bit is incorrect. From this, we assume that the data for the DWEN fuse bit is also incorrect,
         *    and abort the operation.
         *
         *  - Lock bits check - we read the lock bit byte from the target and confirm that all lock bits are cleared.
         *    If any lock bits are set, we abort the operation.
         *
         *  - DWEN fuse bit check - if the DWEN fuse bit is already set to the desired value, then there is no need
         *    to update it. But we may be checking the wrong bit (if the TDF data is incorrect) - either way, we will
         *    abort the operation.
         *
         * The precautions described above may reduce the likelihood of Bloom bricking the connected target, but there
         * is still a chance that all of the checks pass, and we still brick the device. Now would be a good time to
         * remind the user of liabilities in regard to Bloom and its contributors.
         */
        Logger::warning(
            "Updating the DWEN fuse bit is a potentially dangerous operation. Bloom is provided \"AS IS\", "
                "without warranty of any kind. You are using Bloom at your own risk. In no event shall the copyright "
                "owner or contributors be liable for any damage caused as a result of using Bloom. For more details, "
                "see the Bloom license at " + Services::PathService::homeDomainName() + "/license"
        );

        try {
            Logger::info("Reading target signature via ISP");
            const auto ispDeviceSignature = this->avrIspInterface->getDeviceId();

            if (ispDeviceSignature != this->signature) {
                throw Exception{
                    "AVR target signature mismatch - expected signature \"" + this->signature.toHex()
                        + "\" but got \"" + ispDeviceSignature.toHex() + "\". Please check target configuration."
                };
            }

            Logger::info("Target signature confirmed: " + ispDeviceSignature.toHex());

            const auto dwenFuseByte = this->avrIspInterface->readFuse(dwenRegisterDescriptor);
            const auto spienFuseByte = (spienRegisterDescriptor == dwenRegisterDescriptor)
                ? dwenFuseByte
                : this->avrIspInterface->readFuse(spienRegisterDescriptor);

            /*
             * Keep in mind that, for AVR fuses and lock bits, a set bit (0b1) means the fuse/lock is cleared, and a
             * cleared bit (0b0), means the fuse/lock is set.
             */

            if (!this->isFuseEnabled(spienBitFieldDescriptor, spienFuseByte)) {
                /*
                 * If we get here, something is very wrong. The SPIEN (SPI enable) fuse bit appears to be cleared, but
                 * this is not possible because we're connected to the target via the SPI (the ISP interface uses a
                 * physical SPI between the debug tool and the target).
                 *
                 * This could be (and likely is) caused by incorrect data for the SPIEN fuse bit, in the TDF (which was
                 * used to construct the spienFuseBitsDescriptor). And if the data for the SPIEN fuse bit is incorrect,
                 * then what's to say the data for the DWEN fuse bit (dwenFuseBitsDescriptor) is any better?
                 *
                 * We must assume the worst and abort the operation. Otherwise, we risk bricking the user's hardware.
                 */
                throw Exception{
                    "Invalid SPIEN fuse bit value - suspected inaccuracies in TDF data. Please report this to "
                        "Bloom developers as a matter of urgency, via " + Services::PathService::homeDomainName()
                        + "/report-issue"
                };
            }

            Logger::info("Current SPIEN fuse bit value confirmed");

            if (this->isFuseEnabled(dwenBitFieldDescriptor, dwenFuseByte) == enable) {
                /*
                 * The DWEN fuse appears to already be set to the desired value. This may be a result of incorrect data
                 * in the TDF, but we're not taking any chances.
                 *
                 * We don't throw an exception here, because we don't know if this is due to an error, or if the fuse
                 * bit is simply already set to the desired value.
                 */
                Logger::debug("DWEN fuse bit already set to desired value - aborting update operation");

                this->avrIspInterface->deactivate();
                return;
            }

            const auto lockBitByte = this->avrIspInterface->readLockBitByte();
            if (lockBitByte != 0xFF) {
                /*
                 * There is at least one lock bit that is set. Setting the DWEN fuse bit with the lock bits set may
                 * brick the device. We must abort.
                 */
                throw Exception{
                    "At least one lock bit has been set - updating the DWEN fuse bit could potentially brick "
                        "the target."
                };
            }

            Logger::info("Cleared lock bits confirmed");

            const auto newFuseValue = this->setFuseEnabled(dwenBitFieldDescriptor, dwenFuseByte, enable);

            Logger::warning("Updating DWEN fuse bit");
            this->avrIspInterface->programFuse(dwenRegisterDescriptor, newFuseValue);

            Logger::debug("Verifying DWEN fuse bit");
            if (this->avrIspInterface->readFuse(dwenRegisterDescriptor) != newFuseValue) {
                throw Exception{"Failed to update DWEN fuse bit - post-update verification failed"};
            }

            Logger::info("DWEN fuse bit successfully updated");

            this->avrIspInterface->deactivate();

        } catch (const Exception& exception) {
            this->avrIspInterface->deactivate();
            throw exception;
        }
    }

    void Avr8::updateOcdenFuseBit(bool enable) {
        using Services::PathService;
        using Services::StringService;

        if (!this->physicalInterfaces.contains(TargetPhysicalInterface::JTAG)) {
            throw Exception{
                "Target does not support JTAG physical interface - check target configuration or "
                    "report this issue via " + PathService::homeDomainName() + "/report-issue"
            };
        }

        const auto targetSignature = this->avr8DebugInterface->getDeviceId();
        const auto tdSignature = this->targetDescriptionFile.getTargetSignature();

        if (targetSignature != tdSignature) {
            throw Exception{
                "Failed to validate connected target - target signature mismatch.\nThe target signature"
                    " (\"" + targetSignature.toHex() + "\") does not match the AVR8 target description signature (\""
                    + tdSignature.toHex() + "\"). This will likely be due to an incorrect target name in the "
                    + "configuration file (bloom.yaml)."
            };
        }

        const auto ocdenFuseBitFieldPair = this->targetDescriptionFile.getFuseRegisterBitFieldDescriptorPair("ocden");
        const auto& ocdenRegisterDescriptor = ocdenFuseBitFieldPair.first;
        const auto& ocdenBitFieldDescriptor = ocdenFuseBitFieldPair.second;

        const auto jtagenFuseBitFieldPair = this->targetDescriptionFile.getFuseRegisterBitFieldDescriptorPair("jtagen");
        const auto& jtagenRegisterDescriptor = jtagenFuseBitFieldPair.first;
        const auto& jtagenBitFieldDescriptor = jtagenFuseBitFieldPair.second;

        assert(ocdenRegisterDescriptor.size == 1);
        assert(jtagenRegisterDescriptor.size == 1);

        const auto ocdenFuseByteValue = this->avr8DebugInterface->readMemory(
            this->fuseAddressSpaceDescriptor,
            this->fuseMemorySegmentDescriptor,
            ocdenRegisterDescriptor.startAddress,
            1
        ).at(0);
        const auto jtagenFuseByteValue = jtagenRegisterDescriptor == ocdenRegisterDescriptor
            ? ocdenFuseByteValue
            : this->avr8DebugInterface->readMemory(
                this->fuseAddressSpaceDescriptor,
                this->fuseMemorySegmentDescriptor,
                jtagenRegisterDescriptor.startAddress,
                1
            ).at(0);

        Logger::debug("OCDEN fuse byte value (before update): 0x" + StringService::toHex(ocdenFuseByteValue));

        if (!this->isFuseEnabled(jtagenBitFieldDescriptor, jtagenFuseByteValue)) {
            /*
             * If we get here, something has gone wrong. The JTAGEN fuse should always be programmed by this point.
             * We wouldn't have been able to activate the JTAG physical interface if the fuse wasn't programmed.
             *
             * This means the data we have on the JTAGEN fuse bit, from the TDF, is likely incorrect. And if that's
             * the case, we cannot rely on the data for the OCDEN fuse bit being any better.
             */
            throw Exception{
                "Invalid JTAGEN fuse bit value - suspected inaccuracies in TDF data. Please report this to "
                    "Bloom developers as a matter of urgency, via " + PathService::homeDomainName() + "/report-issue"
            };
        }

        if (this->isFuseEnabled(ocdenBitFieldDescriptor, ocdenFuseByteValue) == enable) {
            Logger::debug("OCDEN fuse bit already set to desired value - aborting update operation");
            return;
        }

        const auto newValue = this->setFuseEnabled(ocdenBitFieldDescriptor, ocdenFuseByteValue, enable);

        Logger::debug("New OCDEN fuse byte value (to be written): 0x" + StringService::toHex(newValue));

        Logger::warning("Updating OCDEN fuse bit");
        this->avr8DebugInterface->writeMemory(
            this->fuseAddressSpaceDescriptor,
            this->fuseMemorySegmentDescriptor,
            ocdenRegisterDescriptor.startAddress,
            TargetMemoryBuffer({newValue})
        );

        Logger::debug("Verifying OCDEN fuse bit");
        const auto postUpdateOcdenByteValue = this->avr8DebugInterface->readMemory(
            this->fuseAddressSpaceDescriptor,
            this->fuseMemorySegmentDescriptor,
            ocdenRegisterDescriptor.startAddress,
            1
        ).at(0);

        if (postUpdateOcdenByteValue != newValue) {
            throw Exception{"Failed to update OCDEN fuse bit - post-update verification failed"};
        }

        Logger::info("OCDEN fuse bit updated");

        this->disableProgrammingMode();
    }

    bool Avr8::updateEesaveFuseBit(bool enable) {
        using Services::StringService;

        const auto eesaveFuseBitFieldPair = this->targetDescriptionFile.getFuseRegisterBitFieldDescriptorPair("eesave");
        const auto& eesaveRegisterDescriptor = eesaveFuseBitFieldPair.first;
        const auto& eesaveBitFieldDescriptor = eesaveFuseBitFieldPair.second;

        assert(eesaveRegisterDescriptor.size == 1);

        const auto eesaveFuseByteValue = this->avr8DebugInterface->readMemory(
            this->fuseAddressSpaceDescriptor,
            this->fuseMemorySegmentDescriptor,
            eesaveRegisterDescriptor.startAddress,
            1
        ).at(0);

        Logger::debug("EESAVE fuse byte value (before update): 0x" + StringService::toHex(eesaveFuseByteValue));

        if (this->isFuseEnabled(eesaveBitFieldDescriptor, eesaveFuseByteValue) == enable) {
            Logger::debug("EESAVE fuse bit already set to desired value - aborting update operation");
            return false;
        }

        const auto newValue = this->setFuseEnabled(eesaveBitFieldDescriptor, eesaveFuseByteValue, enable);

        Logger::debug("New EESAVE fuse byte value (to be written): 0x" + StringService::toHex(newValue));

        Logger::warning("Updating EESAVE fuse bit");
        this->avr8DebugInterface->writeMemory(
            this->fuseAddressSpaceDescriptor,
            this->fuseMemorySegmentDescriptor,
            eesaveRegisterDescriptor.startAddress,
            TargetMemoryBuffer({newValue})
        );

        Logger::debug("Verifying EESAVE fuse bit");
        const auto postUpdateEesaveByteValue = this->avr8DebugInterface->readMemory(
            this->fuseAddressSpaceDescriptor,
            this->fuseMemorySegmentDescriptor,
            eesaveRegisterDescriptor.startAddress,
            1
        ).at(0);

        if (postUpdateEesaveByteValue != newValue) {
            throw Exception{"Failed to update EESAVE fuse bit - post-update verification failed"};
        }

        Logger::info("EESAVE fuse bit updated");
        return true;
    }
}
