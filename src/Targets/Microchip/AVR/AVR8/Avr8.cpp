#include "Avr8.hpp"

#include <cassert>
#include <bitset>
#include <limits>
#include <thread>
#include <algorithm>

#include "src/Logger/Logger.hpp"
#include "src/Services/PathService.hpp"
#include "src/Services/StringService.hpp"

#include "src/Exceptions/InvalidConfig.hpp"
#include "Exceptions/DebugWirePhysicalInterfaceError.hpp"

namespace Targets::Microchip::Avr::Avr8Bit
{
    using namespace Exceptions;

    Avr8::Avr8(const TargetConfig& targetConfig, TargetDescription::TargetDescriptionFile&& targetDescriptionFile)
        : targetConfig(Avr8TargetConfig(targetConfig))
        , targetDescriptionFile(std::move(targetDescriptionFile))
        , signature(this->targetDescriptionFile.getTargetSignature())
        , name(this->targetDescriptionFile.getTargetName())
        , family(this->targetDescriptionFile.getAvrFamily())
        , targetParameters(this->targetDescriptionFile.getTargetParameters())
        , physicalInterfaces(this->targetDescriptionFile.getPhysicalInterfaces())
        , padDescriptorsByName(this->targetDescriptionFile.getPadDescriptorsMappedByName())
        , targetVariantsById(this->targetDescriptionFile.getVariantsMappedById())
        , stackPointerRegisterDescriptor(
            TargetRegisterDescriptor(
                TargetRegisterType::STACK_POINTER,
                this->targetParameters.stackPointerRegisterLowAddress.value(),
                this->targetParameters.stackPointerRegisterSize.value(),
                TargetMemoryType::OTHER,
                "SP",
                "CPU",
                "Stack Pointer Register",
                TargetRegisterAccess(true, true)
            )
        )
        , statusRegisterDescriptor(
            TargetRegisterDescriptor(
                TargetRegisterType::STATUS_REGISTER,
                this->targetParameters.statusRegisterStartAddress.value(),
                this->targetParameters.statusRegisterSize.value(),
                TargetMemoryType::OTHER,
                "SREG",
                "CPU",
                "Status Register",
                TargetRegisterAccess(true, true)
            )
        )
        , fuseEnableStrategy(this->targetDescriptionFile.getFuseEnableStrategy().value_or(FuseEnableStrategy::CLEAR))
    {
        if (!this->physicalInterfaces.contains(this->targetConfig.physicalInterface)) {
            /*
             * The user has selected a physical interface that does not appear to be supported by the selected
             * target.
             *
             * Bloom's target description files provide a list of supported physical interfaces for each target
             * (which is how this->physicalInterfaces is populated), but it's possible that this list may
             * be wrong/incomplete. For this reason, we don't throw an exception here. Instead, we just present the
             * user with a warning and a list of physical interfaces known to be supported by their selected target.
             */
            const auto physicalInterfaceNames = getPhysicalInterfaceNames();

            const auto supportedPhysicalInterfaceList = std::accumulate(
                this->physicalInterfaces.begin(),
                this->physicalInterfaces.end(),
                std::string(),
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
                "\nThe selected target (" + this->name + ") does not support the selected physical interface ("
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

        this->loadTargetRegisterDescriptors();
        this->loadTargetMemoryDescriptors();
    }

    bool Avr8::supportsDebugTool(DebugTool* debugTool) {
        return debugTool->getAvr8DebugInterface(
            this->targetConfig,
            this->family,
            this->targetParameters,
            this->targetRegisterDescriptorsById
        ) != nullptr;
    }

    void Avr8::setDebugTool(DebugTool* debugTool) {
        this->targetPowerManagementInterface = debugTool->getTargetPowerManagementInterface();
        this->avr8DebugInterface = debugTool->getAvr8DebugInterface(
            this->targetConfig,
            this->family,
            this->targetParameters,
            this->targetRegisterDescriptorsById
        );

        this->avrIspInterface = debugTool->getAvrIspInterface(
            this->targetConfig
        );

        if (this->avrIspInterface != nullptr) {
            this->avrIspInterface->configure(this->targetConfig);
        }

        if (
            this->targetConfig.manageDwenFuseBit
            && this->avrIspInterface == nullptr
            && this->targetConfig.physicalInterface == TargetPhysicalInterface::DEBUG_WIRE
        ) {
            Logger::warning(
                "The connected debug tool (or associated driver) does not provide any ISP interface. "
                    "Bloom will be unable to manage the DWEN fuse bit."
            );
        }
    }

    void Avr8::activate() {
        if (this->isActivated()) {
            return;
        }

        this->avr8DebugInterface->init();

        try {
            this->avr8DebugInterface->activate();

        } catch (const Exceptions::DebugWirePhysicalInterfaceError& debugWireException) {
            // We failed to activate the debugWire physical interface. DWEN fuse bit may need updating.

            if (!this->targetConfig.manageDwenFuseBit) {
                throw TargetOperationFailure(
                    "Failed to activate debugWire physical interface - check target connection and DWEN fuse "
                        "bit. Bloom can manage the DWEN fuse bit automatically. For instructions on enabling this "
                        "function, see " + Services::PathService::homeDomainName() + "/docs/debugging-avr-debugwire"
                );
            }

            try {
                Logger::warning(
                    "Failed to activate the debugWire physical interface - attempting to access target via "
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
                throw Exception(
                    "Failed to access/update DWEN fuse bit via ISP interface - " + exception.getMessage()
                );
            }

            Logger::info("Retrying debugWire physical interface activation");
            this->avr8DebugInterface->activate();
        }

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
            throw Exception(
                "Failed to validate connected target - target signature mismatch.\nThe target signature"
                " (\"" + targetSignature.toHex() + "\") does not match the AVR8 target description signature (\""
                + this->signature.toHex() + "\"). This will likely be due to an incorrect target name in the "
                + "configuration file (bloom.yaml)."
            );
        }

        this->avr8DebugInterface->reset();
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

    TargetDescriptor Avr8::getDescriptor() {
        auto descriptor = TargetDescriptor(
            this->signature.toHex(),
            TargetFamily::AVR_8,
            this->name,
            "Microchip",
            this->targetMemoryDescriptorsByType,
            this->targetRegisterDescriptorsById,
            this->getBreakpointResources(),
            {},
            Targets::TargetMemoryType::FLASH
        );

        std::transform(
            this->targetVariantsById.begin(),
            this->targetVariantsById.end(),
            std::back_inserter(descriptor.variants),
            [] (auto& variantToIdPair) {
                return variantToIdPair.second;
            }
        );

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

    TargetRegisters Avr8::readRegisters(const Targets::TargetRegisterDescriptorIds& descriptorIds) {
        return this->avr8DebugInterface->readRegisters(descriptorIds);
    }

    void Avr8::writeRegisters(const TargetRegisters& registers) {
        this->avr8DebugInterface->writeRegisters(registers);
    }

    TargetMemoryBuffer Avr8::readMemory(
        TargetMemoryType memoryType,
        std::uint32_t startAddress,
        std::uint32_t bytes,
        const std::set<Targets::TargetMemoryAddressRange>& excludedAddressRanges
    ) {
        return this->avr8DebugInterface->readMemory(memoryType, startAddress, bytes, excludedAddressRanges);
    }

    void Avr8::writeMemory(TargetMemoryType memoryType, std::uint32_t startAddress, const TargetMemoryBuffer& buffer) {
        if (memoryType == TargetMemoryType::FLASH && !this->programmingModeEnabled()) {
            throw Exception("Attempted Flash memory write with no active programming session.");
        }

        this->avr8DebugInterface->writeMemory(memoryType, startAddress, buffer);
    }

    void Avr8::eraseMemory(TargetMemoryType memoryType) {
        if (memoryType == TargetMemoryType::FLASH) {
            if (this->targetConfig.physicalInterface == TargetPhysicalInterface::DEBUG_WIRE) {
                // debugWire targets do not need to be erased
                return;
            }

            if (!this->programmingModeEnabled()) {
                throw Exception("Attempted Flash memory erase with no active programming session.");
            }

            /*
             * For JTAG and UPDI targets, we must perform a chip erase. This means we could end up erasing EEPROM,
             * unless the EESAVE fuse bit has been programmed.
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
         * Debug tools do not have to support the erasing of RAM or EEPROM memory. We just implement this as a
         * write operation.
         */
        this->writeMemory(
            memoryType,
            memoryType == TargetMemoryType::RAM
                ? this->targetParameters.ramStartAddress.value()
                : this->targetParameters.eepromStartAddress.value(),
            TargetMemoryBuffer(
                memoryType == TargetMemoryType::RAM
                    ? this->targetParameters.ramSize.value()
                    : this->targetParameters.eepromSize.value(),
                0xFF
            )
        );
    }

    TargetState Avr8::getState() {
        return this->avr8DebugInterface->getTargetState();
    }

    std::uint32_t Avr8::getProgramCounter() {
        return this->avr8DebugInterface->getProgramCounter();
    }

    void Avr8::setProgramCounter(std::uint32_t programCounter) {
        this->avr8DebugInterface->setProgramCounter(programCounter);
    }

    std::uint32_t Avr8::getStackPointer() {
        const auto stackPointerRegister = this->readRegisters(
            {this->stackPointerRegisterDescriptor.id}
        ).front();

        std::uint32_t stackPointer = 0;
        for (std::size_t i = 0; i < stackPointerRegister.size() && i < 4; i++) {
            stackPointer = (stackPointer << (8 * i)) | stackPointerRegister.value[i];
        }

        return stackPointer;
    }

    std::map<int, TargetPinState> Avr8::getPinStates(int variantId) {
        const auto targetVariantIt = this->targetVariantsById.find(variantId);

        if (targetVariantIt == this->targetVariantsById.end()) {
            throw Exception("Invalid target variant ID");
        }

        std::map<int, TargetPinState> output;
        const auto& variant = targetVariantIt->second;

        /*
         * To prevent the number of memory reads we perform here, we cache the data and map it by start address.
         *
         * This way, we only perform 3 memory reads for a target variant with 3 ports - one per port (instead of one
         * per pin).
         *
         * We may be able to make this more efficient by combining reads for ports with aligned memory addresses. This
         * will be considered when the need for it becomes apparent.
         */
        std::map<std::uint16_t, TargetMemoryBuffer> cachedMemoryByStartAddress;
        const auto readMemoryBitset = [this, &cachedMemoryByStartAddress] (std::uint16_t startAddress) {
            auto cachedByteIt = cachedMemoryByStartAddress.find(startAddress);

            if (cachedByteIt == cachedMemoryByStartAddress.end()) {
                cachedByteIt = cachedMemoryByStartAddress.insert(
                    std::pair(
                        startAddress,
                        this->readMemory(TargetMemoryType::RAM, startAddress, 1)
                    )
                ).first;
            }

            return std::bitset<std::numeric_limits<unsigned char>::digits>(
                cachedByteIt->second.at(0)
            );
        };

        for (const auto& [pinNumber, pinDescriptor] : variant.pinDescriptorsByNumber) {
            const auto padIt = this->padDescriptorsByName.find(pinDescriptor.padName);

            if (padIt != this->padDescriptorsByName.end()) {
                const auto& pad = padIt->second;

                if (!pad.gpioPinNumber.has_value()) {
                    continue;
                }

                auto pinState = TargetPinState();

                if (pad.gpioDdrAddress.has_value()) {
                    const auto ddrValue = readMemoryBitset(pad.gpioDdrAddress.value());

                    pinState.ioDirection = ddrValue.test(pad.gpioPinNumber.value()) ?
                        TargetPinState::IoDirection::OUTPUT : TargetPinState::IoDirection::INPUT;

                    if (pinState.ioDirection == TargetPinState::IoDirection::OUTPUT
                        && pad.gpioPortAddress.has_value()
                    ) {
                        const auto portRegisterValueBitset = readMemoryBitset(pad.gpioPortAddress.value());
                        pinState.ioState = portRegisterValueBitset.test(pad.gpioPinNumber.value()) ?
                            TargetPinState::IoState::HIGH : TargetPinState::IoState::LOW;

                    } else if (pinState.ioDirection == TargetPinState::IoDirection::INPUT
                        && pad.gpioPortInputAddress.has_value()
                    ) {
                        const auto portInputRegisterValue = readMemoryBitset(pad.gpioPortInputAddress.value());
                        pinState.ioState = portInputRegisterValue.test(pad.gpioPinNumber.value()) ?
                            TargetPinState::IoState::HIGH : TargetPinState::IoState::LOW;
                    }
                }

                output.insert(std::pair(pinNumber, pinState));
            }
        }

        return output;
    }

    void Avr8::setPinState(const TargetPinDescriptor& pinDescriptor, const TargetPinState& state) {
        const auto targetVariantIt = this->targetVariantsById.find(pinDescriptor.variantId);

        if (targetVariantIt == this->targetVariantsById.end()) {
            throw Exception("Invalid target variant ID");
        }

        const auto padDescriptorIt = this->padDescriptorsByName.find(pinDescriptor.padName);

        if (padDescriptorIt == this->padDescriptorsByName.end()) {
            throw Exception("Unknown pad");
        }

        if (!state.ioDirection.has_value()) {
            throw Exception("Missing IO direction state");
        }

        const auto& variant = targetVariantIt->second;
        const auto& padDescriptor = padDescriptorIt->second;
        auto ioState = state.ioState;

        if (state.ioDirection == TargetPinState::IoDirection::INPUT) {
            // When setting the direction to INPUT, we must always set the IO pin state to LOW
            ioState = TargetPinState::IoState::LOW;
        }

        if (
            !padDescriptor.gpioDdrAddress.has_value()
            || !padDescriptor.gpioPortAddress.has_value()
            || !padDescriptor.gpioPinNumber.has_value()
        ) {
            throw Exception("Inadequate pad descriptor");
        }

        const auto pinNumber = padDescriptor.gpioPinNumber.value();
        const auto ddrAddress = padDescriptor.gpioDdrAddress.value();
        const auto ddrValue = this->readMemory(TargetMemoryType::RAM, ddrAddress, 1);

        if (ddrValue.empty()) {
            throw Exception("Failed to read DDR value");
        }

        auto ddrValueBitset = std::bitset<std::numeric_limits<unsigned char>::digits>(ddrValue.front());
        if (ddrValueBitset.test(pinNumber) != (state.ioDirection == TargetPinState::IoDirection::OUTPUT)) {
            // DDR needs updating
            ddrValueBitset.set(pinNumber, (state.ioDirection == TargetPinState::IoDirection::OUTPUT));

            this->writeMemory(
                TargetMemoryType::RAM,
                ddrAddress,
                {static_cast<unsigned char>(ddrValueBitset.to_ulong())}
            );
        }

        if (ioState.has_value()) {
            const auto portRegisterAddress = padDescriptor.gpioPortAddress.value();
            const auto portRegisterValue = this->readMemory(TargetMemoryType::RAM, portRegisterAddress, 1);

            if (portRegisterValue.empty()) {
                throw Exception("Failed to read PORT register value");
            }

            auto portRegisterValueBitset = std::bitset<std::numeric_limits<unsigned char>::digits>(
                portRegisterValue.front()
            );

            if (portRegisterValueBitset.test(pinNumber) != (ioState == TargetPinState::IoState::HIGH)) {
                // PORT set register needs updating
                portRegisterValueBitset.set(pinNumber, (ioState == TargetPinState::IoState::HIGH));

                this->writeMemory(
                    TargetMemoryType::RAM,
                    portRegisterAddress,
                    {static_cast<unsigned char>(portRegisterValueBitset.to_ulong())}
                );
            }
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
        this->activeProgrammingSession.reset();
    }

    bool Avr8::programmingModeEnabled() {
        return this->activeProgrammingSession.has_value();
    }

    void Avr8::loadTargetRegisterDescriptors() {
        this->targetRegisterDescriptorsById = this->targetDescriptionFile.getRegisterDescriptorsMappedById();

        /*
         * All AVR8 targets possess 32 general purpose CPU registers. These are not described in the TDF, so we
         * construct the descriptors for them here.
         */
        const auto gpRegisterStartAddress = this->targetParameters.gpRegisterStartAddress.value_or(0);
        for (std::uint8_t i = 0; i <= 31; i++) {
            auto generalPurposeRegisterDescriptor = TargetRegisterDescriptor(
                TargetRegisterType::GENERAL_PURPOSE_REGISTER,
                gpRegisterStartAddress + i,
                1,
                TargetMemoryType::OTHER,
                "r" + std::to_string(i),
                "CPU General Purpose",
                std::nullopt,
                TargetRegisterAccess(true, true)
            );

            this->targetRegisterDescriptorsById.emplace(
                generalPurposeRegisterDescriptor.id,
                std::move(generalPurposeRegisterDescriptor)
            );
        }

        this->targetRegisterDescriptorsById.emplace(
            this->stackPointerRegisterDescriptor.id,
            this->stackPointerRegisterDescriptor
        );
        this->targetRegisterDescriptorsById.emplace(
            this->statusRegisterDescriptor.id,
            this->statusRegisterDescriptor
        );
    }

    void Avr8::loadTargetMemoryDescriptors() {
        const auto ramStartAddress = this->targetParameters.ramStartAddress.value();
        const auto flashStartAddress = this->targetParameters.flashStartAddress.value();

        this->targetMemoryDescriptorsByType.insert(std::pair(
            TargetMemoryType::RAM,
            TargetMemoryDescriptor(
                TargetMemoryType::RAM,
                TargetMemoryAddressRange(
                    ramStartAddress,
                    ramStartAddress + this->targetParameters.ramSize.value() - 1
                ),
                TargetMemoryAccess(true, true, true)
            )
        ));

        this->targetMemoryDescriptorsByType.insert(std::pair(
            TargetMemoryType::FLASH,
            TargetMemoryDescriptor(
                TargetMemoryType::FLASH,
                TargetMemoryAddressRange(
                    flashStartAddress,
                    flashStartAddress + this->targetParameters.flashSize.value() - 1
                ),
                TargetMemoryAccess(true, true, false),
                this->targetParameters.flashPageSize
            )
        ));

        if (this->targetParameters.eepromStartAddress.has_value() && this->targetParameters.eepromSize.has_value()) {
            const auto eepromStartAddress = this->targetParameters.eepromStartAddress.value();

            this->targetMemoryDescriptorsByType.insert(std::pair(
                TargetMemoryType::EEPROM,
                TargetMemoryDescriptor(
                    TargetMemoryType::EEPROM,
                    TargetMemoryAddressRange(
                        eepromStartAddress,
                        eepromStartAddress + this->targetParameters.eepromSize.value() - 1
                    ),
                    TargetMemoryAccess(true, true, true)
                )
            ));
        }
    }

    BreakpointResources Avr8::getBreakpointResources() {
        auto maxHardwareBreakpoints = static_cast<std::uint16_t>(0);

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

        return BreakpointResources(
            maxHardwareBreakpoints,
            std::nullopt,
            std::min(
                static_cast<std::uint16_t>(this->targetConfig.reserveSteppingBreakpoint ? 1 : 0),
                maxHardwareBreakpoints
            )
        );
    }

    bool Avr8::isFuseEnabled(const FuseBitsDescriptor& descriptor, unsigned char fuseByteValue) const {
        const auto programmedValue = static_cast<unsigned char>(
            this->fuseEnableStrategy == FuseEnableStrategy::SET
                ? (0xFF & descriptor.bitMask)
                : 0
        );

        return (fuseByteValue & descriptor.bitMask) == programmedValue;
    }

    unsigned char Avr8::setFuseEnabled(
        const FuseBitsDescriptor& descriptor,
        unsigned char fuseByteValue,
        bool enabled
    ) const {
        return static_cast<unsigned char>(
            this->fuseEnableStrategy == FuseEnableStrategy::SET
                ? enabled
                    ? (fuseByteValue | descriptor.bitMask)
                    : fuseByteValue & ~(descriptor.bitMask)
                : enabled
                    ? fuseByteValue & ~(descriptor.bitMask)
                    : (fuseByteValue | descriptor.bitMask)
        );
    }

    void Avr8::updateDwenFuseBit(bool enable) {
        if (this->avrIspInterface == nullptr) {
            throw Exception(
                "Debug tool or driver does not provide access to an ISP interface - please confirm that the "
                    "debug tool supports ISP and then report this issue via " + Services::PathService::homeDomainName()
                    + "/report-issue"
            );
        }

        if (!this->physicalInterfaces.contains(TargetPhysicalInterface::DEBUG_WIRE)) {
            throw Exception(
                "Target does not support debugWire physical interface - check target configuration or "
                    "report this issue via " + Services::PathService::homeDomainName() + "/report-issue"
            );
        }

        const auto dwenFuseBitsDescriptor = this->targetDescriptionFile.getDwenFuseBitsDescriptor();
        const auto spienFuseBitsDescriptor = this->targetDescriptionFile.getSpienFuseBitsDescriptor();

        if (!dwenFuseBitsDescriptor.has_value()) {
            throw Exception("Could not find DWEN bit field in TDF.");
        }

        if (!spienFuseBitsDescriptor.has_value()) {
            throw Exception("Could not find SPIEN bit field in TDF.");
        }

        Logger::debug("Extracting ISP parameters from TDF");
        this->avrIspInterface->setIspParameters(this->targetDescriptionFile.getIspParameters());

        Logger::info("Initiating ISP interface");
        this->avrIspInterface->activate();

        /*
         * AVR fuses are used to control certain functions within the AVR (including the debugWire interface). Care
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
                throw Exception(
                    "AVR target signature mismatch - expected signature \"" + this->signature.toHex()
                        + "\" but got \"" + ispDeviceSignature.toHex() + "\". Please check target configuration."
                );
            }

            Logger::info("Target signature confirmed: " + ispDeviceSignature.toHex());

            const auto dwenFuseByte = this->avrIspInterface->readFuse(dwenFuseBitsDescriptor->fuseType).value;
            const auto spienFuseByte = (spienFuseBitsDescriptor->fuseType == dwenFuseBitsDescriptor->fuseType)
                ? dwenFuseByte
                : this->avrIspInterface->readFuse(spienFuseBitsDescriptor->fuseType).value;

            /*
             * Keep in mind that, for AVR fuses and lock bits, a set bit (0b1) means the fuse/lock is cleared, and a
             * cleared bit (0b0), means the fuse/lock is set.
             */

            if (!this->isFuseEnabled(*spienFuseBitsDescriptor, spienFuseByte)) {
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
                throw Exception(
                    "Invalid SPIEN fuse bit value - suspected inaccuracies in TDF data. Please report this to "
                        "Bloom developers as a matter of urgency, via " + Services::PathService::homeDomainName()
                        + "/report-issue"
                );
            }

            Logger::info("Current SPIEN fuse bit value confirmed");

            if (this->isFuseEnabled(*dwenFuseBitsDescriptor, dwenFuseByte) == enable) {
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
                throw Exception(
                    "At least one lock bit has been set - updating the DWEN fuse bit could potentially brick "
                        "the target."
                );
            }

            Logger::info("Cleared lock bits confirmed");

            const auto newFuse = Fuse(
                dwenFuseBitsDescriptor->fuseType,
                this->setFuseEnabled(*dwenFuseBitsDescriptor, dwenFuseByte, enable)
            );

            Logger::warning("Updating DWEN fuse bit");
            this->avrIspInterface->programFuse(newFuse);

            Logger::debug("Verifying DWEN fuse bit");
            if (this->avrIspInterface->readFuse(dwenFuseBitsDescriptor->fuseType).value != newFuse.value) {
                throw Exception("Failed to update DWEN fuse bit - post-update verification failed");
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
            throw Exception(
                "Target does not support JTAG physical interface - check target configuration or "
                    "report this issue via " + PathService::homeDomainName() + "/report-issue"
            );
        }

        const auto targetSignature = this->avr8DebugInterface->getDeviceId();
        const auto tdSignature = this->targetDescriptionFile.getTargetSignature();

        if (targetSignature != tdSignature) {
            throw Exception(
                "Failed to validate connected target - target signature mismatch.\nThe target signature"
                    " (\"" + targetSignature.toHex() + "\") does not match the AVR8 target description signature (\""
                    + tdSignature.toHex() + "\"). This will likely be due to an incorrect target name in the "
                    + "configuration file (bloom.yaml)."
            );
        }

        const auto ocdenFuseBitsDescriptor = this->targetDescriptionFile.getOcdenFuseBitsDescriptor();
        const auto jtagenFuseBitsDescriptor = this->targetDescriptionFile.getJtagenFuseBitsDescriptor();

        if (!ocdenFuseBitsDescriptor.has_value()) {
            throw Exception("Could not find OCDEN bit field in TDF.");
        }

        if (!jtagenFuseBitsDescriptor.has_value()) {
            throw Exception("Could not find JTAGEN bit field in TDF.");
        }

        const auto ocdenFuseByteValue = this->avr8DebugInterface->readMemory(
            TargetMemoryType::FUSES,
            ocdenFuseBitsDescriptor->byteAddress,
            1
        ).at(0);
        const auto jtagenFuseByteValue = jtagenFuseBitsDescriptor->byteAddress == ocdenFuseBitsDescriptor->byteAddress
            ? ocdenFuseByteValue
            : this->avr8DebugInterface->readMemory(
                TargetMemoryType::FUSES,
                jtagenFuseBitsDescriptor->byteAddress,
                1
            ).at(0)
        ;

        Logger::debug("OCDEN fuse byte value (before update): 0x" + StringService::toHex(ocdenFuseByteValue));

        if (!this->isFuseEnabled(*jtagenFuseBitsDescriptor, jtagenFuseByteValue)) {
            /*
             * If we get here, something has gone wrong. The JTAGEN fuse should always be programmed by this point.
             * We wouldn't have been able to activate the JTAG physical interface if the fuse wasn't programmed.
             *
             * This means the data we have on the JTAGEN fuse bit, from the TDF, is likely incorrect. And if that's
             * the case, we cannot rely on the data for the OCDEN fuse bit being any better.
             */
            throw Exception(
                "Invalid JTAGEN fuse bit value - suspected inaccuracies in TDF data. Please report this to "
                "Bloom developers as a matter of urgency, via " + PathService::homeDomainName() + "/report-issue"
            );
        }

        if (this->isFuseEnabled(*ocdenFuseBitsDescriptor, ocdenFuseByteValue) == enable) {
            Logger::debug("OCDEN fuse bit already set to desired value - aborting update operation");
            return;
        }

        const auto newValue = this->setFuseEnabled(*ocdenFuseBitsDescriptor, ocdenFuseByteValue, enable);

        Logger::debug("New OCDEN fuse byte value (to be written): 0x" + StringService::toHex(newValue));

        Logger::warning("Updating OCDEN fuse bit");
        this->avr8DebugInterface->writeMemory(
            TargetMemoryType::FUSES,
            ocdenFuseBitsDescriptor->byteAddress,
            {newValue}
        );

        Logger::debug("Verifying OCDEN fuse bit");
        const auto postUpdateOcdenByteValue = this->avr8DebugInterface->readMemory(
            TargetMemoryType::FUSES,
            ocdenFuseBitsDescriptor->byteAddress,
            1
        ).at(0);

        if (postUpdateOcdenByteValue != newValue) {
            throw Exception("Failed to update OCDEN fuse bit - post-update verification failed");
        }

        Logger::info("OCDEN fuse bit updated");

        this->disableProgrammingMode();
    }

    bool Avr8::updateEesaveFuseBit(bool enable) {
        using Services::StringService;

        const auto eesaveFuseBitsDescriptor = this->targetDescriptionFile.getEesaveFuseBitsDescriptor();

        if (!eesaveFuseBitsDescriptor.has_value()) {
            throw Exception("Could not find EESAVE bit field in TDF.");
        }

        const auto eesaveFuseByteValue = this->avr8DebugInterface->readMemory(
            TargetMemoryType::FUSES,
            eesaveFuseBitsDescriptor->byteAddress,
            1
        ).at(0);

        Logger::debug("EESAVE fuse byte value (before update): 0x" + StringService::toHex(eesaveFuseByteValue));

        if (this->isFuseEnabled(*eesaveFuseBitsDescriptor, eesaveFuseByteValue) == enable) {
            Logger::debug("EESAVE fuse bit already set to desired value - aborting update operation");
            return false;
        }

        const auto newValue = this->setFuseEnabled(*eesaveFuseBitsDescriptor, eesaveFuseByteValue, enable);

        Logger::debug("New EESAVE fuse byte value (to be written): 0x" + StringService::toHex(newValue));

        Logger::warning("Updating EESAVE fuse bit");
        this->avr8DebugInterface->writeMemory(
            TargetMemoryType::FUSES,
            eesaveFuseBitsDescriptor->byteAddress,
            {newValue}
        );

        Logger::debug("Verifying EESAVE fuse bit");
        const auto postUpdateEesaveByteValue = this->avr8DebugInterface->readMemory(
            TargetMemoryType::FUSES,
            eesaveFuseBitsDescriptor->byteAddress,
            1
        ).at(0);

        if (postUpdateEesaveByteValue != newValue) {
            throw Exception("Failed to update EESAVE fuse bit - post-update verification failed");
        }

        Logger::info("EESAVE fuse bit updated");

        return true;
    }

    ProgramMemorySection Avr8::getProgramMemorySectionFromAddress(std::uint32_t address) {
        return this->targetParameters.bootSectionStartAddress.has_value()
            && address >= this->targetParameters.bootSectionStartAddress.value()
            ? ProgramMemorySection::BOOT
            : ProgramMemorySection::APPLICATION;
    }
}
