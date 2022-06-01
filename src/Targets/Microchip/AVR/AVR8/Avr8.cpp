#include "Avr8.hpp"

#include <cassert>
#include <bitset>
#include <limits>
#include <thread>

#include "src/Logger/Logger.hpp"
#include "src/Helpers/Paths.hpp"

#include "src/Exceptions/InvalidConfig.hpp"
#include "Exceptions/DebugWirePhysicalInterfaceError.hpp"
#include "src/Targets/TargetRegister.hpp"

#include "src/Targets/Microchip/AVR/Fuse.hpp"

// Derived AVR8 targets
#include "XMega/XMega.hpp"
#include "Mega/Mega.hpp"
#include "Tiny/Tiny.hpp"

namespace Bloom::Targets::Microchip::Avr::Avr8Bit
{
    using namespace Exceptions;

    void Avr8::preActivationConfigure(const TargetConfig& targetConfig) {
        Target::preActivationConfigure(targetConfig);

        this->targetConfig = Avr8TargetConfig(targetConfig);

        if (this->family.has_value()) {
            this->avr8DebugInterface->setFamily(this->family.value());

        } else {
            if (this->targetConfig->physicalInterface == PhysicalInterface::JTAG) {
                throw InvalidConfig(
                    "The JTAG physical interface cannot be used with an ambiguous target name"
                    " - please specify the exact name of the target in your configuration file. "
                    "See " + Paths::homeDomainName() + "/docs/supported-targets"
                );
            }

            if (this->targetConfig->physicalInterface == PhysicalInterface::UPDI) {
                throw InvalidConfig(
                    "The UPDI physical interface cannot be used with an ambiguous target name"
                    " - please specify the exact name of the target in your configuration file. "
                    "See " + Paths::homeDomainName() + "/docs/supported-targets"
                );
            }
        }

        if (
            this->targetConfig->updateDwenFuseBit && this->avrIspInterface == nullptr
            && this->targetConfig->physicalInterface == PhysicalInterface::DEBUG_WIRE
        ) {
            Logger::warning(
                "The connected debug tool (or associated driver) does not provide any ISP interface. "
                "Bloom will be unable to update the DWEN fuse bit in the event of a debugWire activation failure."
            );
        }

        this->avr8DebugInterface->configure(this->targetConfig.value());

        if (this->avrIspInterface != nullptr) {
            this->avrIspInterface->configure(targetConfig);
        }
    }

    void Avr8::postActivationConfigure() {
        if (!this->targetDescriptionFile.has_value()) {
            this->loadTargetDescriptionFile();
            this->initFromTargetDescriptionFile();
        }

        /*
         * The signature obtained from the device should match what is in the target description file
         *
         * We don't use this->getId() here as that could return the ID that was extracted from the target description
         * file (which it would, if the user specified the exact target name in their project config - see
         * Avr8::getId() and TargetControllerComponent::getSupportedTargets() for more).
         */
        auto targetSignature = this->avr8DebugInterface->getDeviceId();
        auto tdSignature = this->targetDescriptionFile->getTargetSignature();

        if (targetSignature != tdSignature) {
            throw Exception(
                "Failed to validate connected target - target signature mismatch.\nThe target signature"
                " (\"" + targetSignature.toHex() + "\") does not match the AVR8 target description signature (\""
                + tdSignature.toHex() + "\"). This will likely be due to an incorrect target name in the configuration"
                + " file (bloom.json)."
            );
        }
    }

    void Avr8::postPromotionConfigure() {
        if (!this->family.has_value()) {
            throw Exception("Failed to resolve AVR8 family");
        }

        this->avr8DebugInterface->setFamily(this->family.value());
        this->avr8DebugInterface->setTargetParameters(this->targetParameters.value());
    }

    void Avr8::activate() {
        if (this->isActivated()) {
            return;
        }

        this->avr8DebugInterface->init();

        if (this->targetDescriptionFile.has_value()) {
            this->avr8DebugInterface->setTargetParameters(this->targetParameters.value());
        }

        try {
            this->avr8DebugInterface->activate();

        } catch (const Exceptions::DebugWirePhysicalInterfaceError& debugWireException) {
            if (!this->targetConfig->updateDwenFuseBit) {
                throw TargetOperationFailure(
                    "Failed to activate debugWire physical interface - check target connection and DWEN fuse "
                        "bit. Bloom can manage the DWEN fuse bit automatically. For instructions on enabling this "
                        "function, see " + Paths::homeDomainName() + "/docs/debugging-avr-debugwire"
                );
            }

            try {
                Logger::warning(
                    "Failed to activate the debugWire physical interface - attempting to access target via "
                        "the ISP interface, for DWEN fuse bit inspection."
                );
                this->writeDwenFuseBit(true);

                // If the debug tool provides a TargetPowerManagementInterface, attempt to cycle the target power
                if (
                    this->targetPowerManagementInterface != nullptr
                    && this->targetConfig->cycleTargetPowerPostDwenUpdate
                ) {
                    Logger::info("Cycling target power");

                    Logger::debug("Disabling target power");
                    this->targetPowerManagementInterface->disableTargetPower();

                    Logger::debug(
                        "Holding power off for ~"
                            + std::to_string(this->targetConfig->targetPowerCycleDelay.count())
                            + " ms"
                    );
                    std::this_thread::sleep_for(this->targetConfig->targetPowerCycleDelay);

                    Logger::debug("Enabling target power");
                    this->targetPowerManagementInterface->enableTargetPower();

                    Logger::debug(
                        "Waiting ~" + std::to_string(this->targetConfig->targetPowerCycleDelay.count())
                            + " ms for target power-up"
                    );
                    std::this_thread::sleep_for(this->targetConfig->targetPowerCycleDelay);
                }

            } catch (const Exception& exception) {
                throw Exception(
                    "Failed to access/update DWEN fuse bit via ISP interface - " + exception.getMessage()
                );
            }

            Logger::info("Retrying debugWire physical interface activation");
            this->avr8DebugInterface->activate();
        }

        this->activated = true;
        this->avr8DebugInterface->reset();
    }

    void Avr8::deactivate() {
        try {
            this->avr8DebugInterface->deactivate();
            this->activated = false;

        } catch (const Exception& exception) {
            Logger::error("Failed to deactivate AVR8 target - " + exception.getMessage());
        }
    }

    std::unique_ptr<Targets::Target> Avr8::promote() {
        std::unique_ptr<Targets::Target> promoted = nullptr;

        if (this->family.has_value()) {
            // Promote generic AVR8 target to correct family.
            switch (this->family.value()) {
                case Family::XMEGA: {
                    Logger::info("AVR8 target promoted to XMega target");
                    promoted = std::make_unique<XMega>(*this);
                    break;
                }
                case Family::MEGA: {
                    Logger::info("AVR8 target promoted to megaAVR target");
                    promoted = std::make_unique<Mega>(*this);
                    break;
                }
                case Family::TINY: {
                    Logger::info("AVR8 target promoted to tinyAVR target");
                    promoted = std::make_unique<Tiny>(*this);
                    break;
                }
                default: {
                    break;
                }
            }
        }

        return promoted;
    }

    TargetDescriptor Avr8Bit::Avr8::getDescriptor() {
        auto descriptor = TargetDescriptor();
        descriptor.id = this->getHumanReadableId();
        descriptor.name = this->getName();
        descriptor.programMemoryType = Targets::TargetMemoryType::FLASH;
        descriptor.registerDescriptorsByType = this->targetRegisterDescriptorsByType;
        descriptor.memoryDescriptorsByType = this->targetMemoryDescriptorsByType;

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

    void Avr8::run() {
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

    void Avr8::setBreakpoint(std::uint32_t address) {
        this->avr8DebugInterface->setBreakpoint(address);
    }

    void Avr8::removeBreakpoint(std::uint32_t address) {
        this->avr8DebugInterface->clearBreakpoint(address);
    }

    void Avr8::clearAllBreakpoints() {
        this->avr8DebugInterface->clearAllBreakpoints();
    }

    void Avr8::writeRegisters(TargetRegisters registers) {
        for (auto registerIt = registers.begin(); registerIt != registers.end();) {
            if (registerIt->descriptor.type == TargetRegisterType::PROGRAM_COUNTER) {
                auto programCounterBytes = registerIt->value;

                if (programCounterBytes.size() < 4) {
                    // All PC register values should be at least 4 bytes in size
                    programCounterBytes.insert(programCounterBytes.begin(), 4 - programCounterBytes.size(), 0x00);
                }

                this->setProgramCounter(static_cast<std::uint32_t>(
                    programCounterBytes[0] << 24
                    | programCounterBytes[1] << 16
                    | programCounterBytes[2] << 8
                    | programCounterBytes[3]
                ));

                registerIt = registers.erase(registerIt);

            } else {
                registerIt++;
            }
        }

        if (!registers.empty()) {
            this->avr8DebugInterface->writeRegisters(registers);
        }
    }

    TargetRegisters Avr8::readRegisters(TargetRegisterDescriptors descriptors) {
        TargetRegisters registers;

        for (auto registerDescriptorIt = descriptors.begin(); registerDescriptorIt != descriptors.end();) {
            const auto& descriptor = *registerDescriptorIt;

            if (descriptor.type == TargetRegisterType::PROGRAM_COUNTER) {
                registers.push_back(this->getProgramCounterRegister());

                registerDescriptorIt = descriptors.erase(registerDescriptorIt);

            } else {
                registerDescriptorIt++;
            }
        }

        if (!descriptors.empty()) {
            auto otherRegisters = this->avr8DebugInterface->readRegisters(descriptors);
            registers.insert(registers.end(), otherRegisters.begin(), otherRegisters.end());
        }

        return registers;
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
        this->avr8DebugInterface->writeMemory(memoryType, startAddress, buffer);
    }

    TargetState Avr8::getState() {
        return this->avr8DebugInterface->getTargetState();
    }

    std::uint32_t Avr8::getProgramCounter() {
        return this->avr8DebugInterface->getProgramCounter();
    }

    TargetRegister Avr8::getProgramCounterRegister() {
        auto programCounter = this->getProgramCounter();

        return TargetRegister(TargetRegisterDescriptor(TargetRegisterType::PROGRAM_COUNTER), {
            static_cast<unsigned char>(programCounter >> 24),
            static_cast<unsigned char>(programCounter >> 16),
            static_cast<unsigned char>(programCounter >> 8),
            static_cast<unsigned char>(programCounter),
        });
    }

    void Avr8::setProgramCounter(std::uint32_t programCounter) {
        this->avr8DebugInterface->setProgramCounter(programCounter);
    }

    std::uint32_t Avr8::getStackPointer() {
        const auto stackPointerRegister = this->readRegisters(
            {this->targetRegisterDescriptorsByType.at(TargetRegisterType::STACK_POINTER)}
        ).front();

        std::uint32_t stackPointer = 0;
        for (std::size_t i = 0; i < stackPointerRegister.size() && i < 4; i++) {
            stackPointer = (stackPointer << (8 * i)) | stackPointerRegister.value[i];
        }

        return stackPointer;
    }

    std::map<int, TargetPinState> Avr8::getPinStates(int variantId) {
        if (!this->targetVariantsById.contains(variantId)) {
            throw Exception("Invalid target variant ID");
        }

        std::map<int, TargetPinState> output;
        auto& variant = this->targetVariantsById.at(variantId);

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
        auto readMemoryBitset = [this, &cachedMemoryByStartAddress] (std::uint16_t startAddress) {
            if (!cachedMemoryByStartAddress.contains(startAddress)) {
                cachedMemoryByStartAddress.insert(
                    std::pair(
                        startAddress,
                        this->readMemory(TargetMemoryType::RAM, startAddress, 1)
                    )
                );
            }

            return std::bitset<std::numeric_limits<unsigned char>::digits>(
                cachedMemoryByStartAddress.at(startAddress).at(0)
            );
        };

        for (const auto& [pinNumber, pinDescriptor] : variant.pinDescriptorsByNumber) {
            if (this->padDescriptorsByName.contains(pinDescriptor.padName)) {
                auto& pad = this->padDescriptorsByName.at(pinDescriptor.padName);

                if (!pad.gpioPinNumber.has_value()) {
                    continue;
                }

                auto pinState = TargetPinState();

                if (pad.ddrSetAddress.has_value()) {
                    auto dataDirectionRegisterValue = readMemoryBitset(pad.ddrSetAddress.value());
                    pinState.ioDirection = dataDirectionRegisterValue.test(pad.gpioPinNumber.value()) ?
                        TargetPinState::IoDirection::OUTPUT : TargetPinState::IoDirection::INPUT;

                    if (pinState.ioDirection == TargetPinState::IoDirection::OUTPUT
                        && pad.gpioPortSetAddress.has_value()
                    ) {
                        auto portRegisterValue = readMemoryBitset(pad.gpioPortSetAddress.value());
                        pinState.ioState = portRegisterValue.test(pad.gpioPinNumber.value()) ?
                            TargetPinState::IoState::HIGH : TargetPinState::IoState::LOW;

                    } else if (pinState.ioDirection == TargetPinState::IoDirection::INPUT
                        && pad.gpioPortInputAddress.has_value()
                    ) {
                        auto portInputRegisterValue = readMemoryBitset(pad.gpioPortInputAddress.value());
                        auto h = portInputRegisterValue.to_string();
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
        auto variantId = pinDescriptor.variantId;
        if (!this->targetVariantsById.contains(variantId)) {
            throw Exception("Invalid target variant ID");
        }

        if (!this->padDescriptorsByName.contains(pinDescriptor.padName)) {
            throw Exception("Unknown pad");
        }

        if (!state.ioDirection.has_value()) {
            throw Exception("Missing IO direction state");
        }

        auto& variant = this->targetVariantsById.at(variantId);
        auto& padDescriptor = this->padDescriptorsByName.at(pinDescriptor.padName);
        auto ioState = state.ioState;

        if (state.ioDirection == TargetPinState::IoDirection::INPUT) {
            // When setting the direction to INPUT, we must always set the IO pinstate to LOW
            ioState = TargetPinState::IoState::LOW;
        }

        if (
            !padDescriptor.ddrSetAddress.has_value()
            || !padDescriptor.gpioPortSetAddress.has_value()
            || !padDescriptor.gpioPinNumber.has_value()
        ) {
            throw Exception("Inadequate pad descriptor");
        }

        auto pinNumber = padDescriptor.gpioPinNumber.value();
        auto ddrSetAddress = padDescriptor.ddrSetAddress.value();
        auto ddrSetValue = this->readMemory(TargetMemoryType::RAM, ddrSetAddress, 1);

        if (ddrSetValue.empty()) {
            throw Exception("Failed to read DDSR value");
        }

        auto ddrSetBitset = std::bitset<std::numeric_limits<unsigned char>::digits>(ddrSetValue.front());
        if (ddrSetBitset.test(pinNumber) != (state.ioDirection == TargetPinState::IoDirection::OUTPUT)) {
            // DDR needs updating
            ddrSetBitset.set(pinNumber, (state.ioDirection == TargetPinState::IoDirection::OUTPUT));

            this->writeMemory(
                TargetMemoryType::RAM,
                ddrSetAddress,
                {static_cast<unsigned char>(ddrSetBitset.to_ulong())}
            );
        }

        if (padDescriptor.ddrClearAddress.has_value() && padDescriptor.ddrClearAddress != ddrSetAddress) {
            // We also need to ensure the data direction clear register value is correct
            auto ddrClearAddress = padDescriptor.ddrClearAddress.value();
            auto ddrClearValue = this->readMemory(TargetMemoryType::RAM, ddrClearAddress, 1);

            if (ddrClearValue.empty()) {
                throw Exception("Failed to read DDCR value");
            }

            auto ddrClearBitset = std::bitset<std::numeric_limits<unsigned char>::digits>(ddrClearValue.front());
            if (ddrClearBitset.test(pinNumber) == (state.ioDirection == TargetPinState::IoDirection::INPUT)) {
                ddrClearBitset.set(pinNumber, (state.ioDirection == TargetPinState::IoDirection::INPUT));

                this->writeMemory(
                    TargetMemoryType::RAM,
                    ddrClearAddress,
                    {static_cast<unsigned char>(ddrClearBitset.to_ulong())}
                );
            }
        }

        if (ioState.has_value()) {
            auto portSetAddress = padDescriptor.gpioPortSetAddress.value();

            if (ioState == TargetPinState::IoState::HIGH
                || !padDescriptor.gpioPortClearAddress.has_value()
                || padDescriptor.gpioPortClearAddress == portSetAddress
            ) {
                if (padDescriptor.gpioPortClearAddress != portSetAddress) {
                    /*
                     * We don't need to read the SET register if the SET and CLEAR operations are performed via
                     * different registers.
                     *
                     * Instead, we can just set the appropriate bit against the SET register.
                     */
                    this->writeMemory(
                        TargetMemoryType::RAM,
                        portSetAddress,
                        {static_cast<unsigned char>(0x01 << pinNumber)}
                    );

                } else {
                    auto portSetRegisterValue = this->readMemory(
                        TargetMemoryType::RAM,
                        portSetAddress,
                        1
                    );

                    if (portSetRegisterValue.empty()) {
                        throw Exception("Failed to read PORT register value");
                    }

                    auto portSetBitset = std::bitset<std::numeric_limits<unsigned char>::digits>(
                        portSetRegisterValue.front()
                    );
                    if (portSetBitset.test(pinNumber) != (ioState == TargetPinState::IoState::HIGH)) {
                        // PORT set register needs updating
                        portSetBitset.set(pinNumber, (ioState == TargetPinState::IoState::HIGH));

                        this->writeMemory(
                            TargetMemoryType::RAM,
                            portSetAddress,
                            {static_cast<unsigned char>(portSetBitset.to_ulong())}
                        );
                    }
                }
            }

            /*
             * We only need to update the PORT clear register if the IO state was set to LOW, and the clear register is
             * not the same register as the set register.
             */
            if (ioState == TargetPinState::IoState::LOW
                && padDescriptor.gpioPortClearAddress.has_value()
                && padDescriptor.gpioPortClearAddress != portSetAddress
            ) {
                // We also need to ensure the PORT clear register value is correct
                auto portClearAddress = padDescriptor.gpioPortClearAddress.value();

                this->writeMemory(
                    TargetMemoryType::RAM,
                    portClearAddress,
                    {static_cast<unsigned char>(0x01 << pinNumber)}
                );
            }
        }
    }

    void Avr8::enableProgrammingMode() {
        this->avr8DebugInterface->enableProgrammingMode();
        this->programmingModeActive = true;
    }

    void Avr8::disableProgrammingMode() {
        this->avr8DebugInterface->disableProgrammingMode();
        this->programmingModeActive = false;
    }

    bool Avr8::programmingModeEnabled() {
        return this->programmingModeActive;
    }

    void Avr8::loadTargetDescriptionFile() {
        this->targetDescriptionFile = TargetDescription::TargetDescriptionFile(
            this->getId(),
            (!this->name.empty()) ? std::optional(this->name) : std::nullopt
        );
    }

    void Avr8::initFromTargetDescriptionFile() {
        assert(this->targetDescriptionFile.has_value());
        this->name = this->targetDescriptionFile->getTargetName();
        this->family = this->targetDescriptionFile->getFamily();

        this->targetParameters = this->targetDescriptionFile->getTargetParameters();
        this->padDescriptorsByName = this->targetDescriptionFile->getPadDescriptorsMappedByName();
        this->targetVariantsById = this->targetDescriptionFile->getVariantsMappedById();

        if (!this->targetParameters->stackPointerRegisterLowAddress.has_value()) {
            throw Exception(
                "Failed to load sufficient AVR8 target paramters - missting stack pointer start address"
            );
        }

        if (!this->targetParameters->statusRegisterStartAddress.has_value()) {
            throw Exception(
                "Failed to load sufficient AVR8 target parameters - missting status register start address"
            );
        }

        this->loadTargetRegisterDescriptors();
        this->loadTargetMemoryDescriptors();
    }

    void Avr8::loadTargetRegisterDescriptors() {
        this->targetRegisterDescriptorsByType = this->targetDescriptionFile->getRegisterDescriptorsMappedByType();

        /*
         * All AVR8 targets possess 32 general purpose CPU registers. These are not described in the TDF, so we
         * construct the descriptors for them here.
         */
        auto gpRegisterStartAddress = this->targetParameters->gpRegisterStartAddress.value_or(0);
        for (std::uint8_t i = 0; i <= 31; i++) {
            auto generalPurposeRegisterDescriptor = TargetRegisterDescriptor();
            generalPurposeRegisterDescriptor.startAddress = gpRegisterStartAddress + i;
            generalPurposeRegisterDescriptor.size = 1;
            generalPurposeRegisterDescriptor.type = TargetRegisterType::GENERAL_PURPOSE_REGISTER;
            generalPurposeRegisterDescriptor.name = "r" + std::to_string(i);
            generalPurposeRegisterDescriptor.groupName = "general purpose cpu";
            generalPurposeRegisterDescriptor.readable = true;
            generalPurposeRegisterDescriptor.writable = true;

            this->targetRegisterDescriptorsByType[generalPurposeRegisterDescriptor.type].insert(
                generalPurposeRegisterDescriptor
            );
        }

        /*
         * The SP and SREG registers are described in the TDF, so we could just use the descriptors extracted from the
         * TDF. The problem with that is, sometimes the SP register consists of two bytes; an SPL and an SPH. These need
         * to be combined into one register descriptor. This is why we just use what we already have in
         * this->targetParameters.
         */
        auto stackPointerRegisterDescriptor = TargetRegisterDescriptor();
        stackPointerRegisterDescriptor.type = TargetRegisterType::STACK_POINTER;
        stackPointerRegisterDescriptor.startAddress = this->targetParameters->stackPointerRegisterLowAddress.value();
        stackPointerRegisterDescriptor.size = this->targetParameters->stackPointerRegisterSize.value();
        stackPointerRegisterDescriptor.name = "SP";
        stackPointerRegisterDescriptor.groupName = "CPU";
        stackPointerRegisterDescriptor.description = "Stack Pointer Register";
        stackPointerRegisterDescriptor.readable = true;
        stackPointerRegisterDescriptor.writable = true;

        auto statusRegisterDescriptor = TargetRegisterDescriptor();
        statusRegisterDescriptor.type = TargetRegisterType::STATUS_REGISTER;
        statusRegisterDescriptor.startAddress = this->targetParameters->statusRegisterStartAddress.value();
        statusRegisterDescriptor.size = this->targetParameters->statusRegisterSize.value();
        statusRegisterDescriptor.name = "SREG";
        statusRegisterDescriptor.groupName = "CPU";
        statusRegisterDescriptor.description = "Status Register";
        statusRegisterDescriptor.readable = true;
        statusRegisterDescriptor.writable = true;

        auto programCounterRegisterDescriptor = TargetRegisterDescriptor();
        programCounterRegisterDescriptor.type = TargetRegisterType::PROGRAM_COUNTER;
        programCounterRegisterDescriptor.size = 4;
        programCounterRegisterDescriptor.name = "PC";
        programCounterRegisterDescriptor.groupName = "CPU";
        programCounterRegisterDescriptor.description = "Program Counter";
        programCounterRegisterDescriptor.readable = true;
        programCounterRegisterDescriptor.writable = true;

        this->targetRegisterDescriptorsByType[stackPointerRegisterDescriptor.type].insert(
            stackPointerRegisterDescriptor
        );
        this->targetRegisterDescriptorsByType[statusRegisterDescriptor.type].insert(
            statusRegisterDescriptor
        );
        this->targetRegisterDescriptorsByType[programCounterRegisterDescriptor.type].insert(
            programCounterRegisterDescriptor
        );
    }

    void Avr8::loadTargetMemoryDescriptors() {
        const auto ramStartAddress = this->targetParameters->ramStartAddress.value();
        const auto flashStartAddress = this->targetParameters->flashStartAddress.value();

        this->targetMemoryDescriptorsByType.insert(std::pair(
            TargetMemoryType::RAM,
            TargetMemoryDescriptor(
                TargetMemoryType::RAM,
                TargetMemoryAddressRange(
                    ramStartAddress,
                    ramStartAddress + this->targetParameters->ramSize.value() - 1
                )
            )
        ));

        this->targetMemoryDescriptorsByType.insert(std::pair(
            TargetMemoryType::FLASH,
            TargetMemoryDescriptor(
                TargetMemoryType::FLASH,
                TargetMemoryAddressRange(
                    flashStartAddress,
                    flashStartAddress + this->targetParameters->flashSize.value() - 1
                ),
                this->targetParameters->flashPageSize
            )
        ));

        if (this->targetParameters->eepromStartAddress.has_value() && this->targetParameters->eepromSize.has_value()) {
            const auto eepromStartAddress = this->targetParameters->eepromStartAddress.value();

            this->targetMemoryDescriptorsByType.insert(std::pair(
                TargetMemoryType::EEPROM,
                TargetMemoryDescriptor(
                    TargetMemoryType::EEPROM,
                    TargetMemoryAddressRange(
                        eepromStartAddress,
                        eepromStartAddress + this->targetParameters->eepromSize.value() - 1
                    )
                )
            ));
        }
    }

    TargetSignature Avr8::getId() {
        if (!this->id.has_value()) {
            this->id = this->avr8DebugInterface->getDeviceId();
        }

        return this->id.value();
    }

    void Avr8::writeDwenFuseBit(bool setFuse) {
        if (this->avrIspInterface == nullptr) {
            throw Exception(
                "Debug tool or driver does not provide access to an ISP interface - please confirm that the "
                    "debug tool supports ISP and then report this issue via " + Paths::homeDomainName()
                    + "/report-issue"
            );
        }

        if (!this->targetDescriptionFile.has_value() || !this->id.has_value()) {
            throw Exception(
                "Insufficient target information for ISP interface - do not use the generic \"avr8\" "
                    "target name in conjunction with the ISP interface. Please update your target configuration."
            );
        }

        const auto& supportedPhysicalInterfaces = this->targetDescriptionFile->getSupportedDebugPhysicalInterfaces();
        if (!supportedPhysicalInterfaces.contains(PhysicalInterface::DEBUG_WIRE)) {
            throw Exception(
                "Target does not support debugWire physical interface - check target configuration or "
                    "report this issue via " + Paths::homeDomainName() + "/report-issue"
            );
        }

        const auto dwenFuseBitsDescriptor = this->targetDescriptionFile->getDwenFuseBitsDescriptor();
        const auto spienFuseBitsDescriptor = this->targetDescriptionFile->getSpienFuseBitsDescriptor();

        if (!dwenFuseBitsDescriptor.has_value()) {
            throw Exception("Could not find DWEN bit field in TDF.");
        }

        if (!spienFuseBitsDescriptor.has_value()) {
            throw Exception("Could not find SPIEN bit field in TDF.");
        }

        Logger::debug("Extracting ISP parameters from TDF");
        this->avrIspInterface->setIspParameters(this->targetDescriptionFile->getIspParameters());

        Logger::info("Initiating ISP interface");
        this->avrIspInterface->activate();

        /*
         * It is crucial that we understand the potential consequences of this operation.
         *
         * AVR fuses are used to control certain functions within the AVR (including the debugWire interface). Care
         * must be taken when updating these fuse bytes, as an incorrect value could render the AVR inaccessible to
         * standard programmers.
         *
         * For example, consider the SPI enable (SPIEN) fuse bit. This fuse bit is used to enable/disable the SPI for
         * serial programming. If the SPIEN fuse bit is cleared, most programming tools will not be able to gain access
         * to the target via the SPI. This isn't too bad, if there is some other way for the programming tool to gain
         * access (such as the debugWire interface). But now consider the DWEN fuse bit (which is used to enable/disable
         * the debugWire interface). What if both the SPIEN *and* the DWEN fuse bits are cleared? Both interfaces will
         * be disabled. Effectively, the AVR will be bricked, and the only course for recovery would be to use
         * high-voltage programming.
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
         * remind the user of liabilities in regards to Bloom and its contributors.
         */
        Logger::warning(
            "Updating the DWEN fuse bit is a potentially dangerous operation. Bloom is provided \"AS IS\", "
                "without warranty of any kind. You are using Bloom at your own risk. In no event shall the copyright "
                "owner or contributors be liable for any damage caused as a result of using Bloom. For more details, "
                "see the Bloom license at " + Paths::homeDomainName() + "/license"
        );

        try {
            Logger::info("Reading target signature via ISP");
            const auto ispDeviceId = this->avrIspInterface->getDeviceId();

            if (ispDeviceId != this->id) {
                throw Exception(
                    "AVR target signature mismatch - expected signature \"" + this->id->toHex()
                        + "\" but got \"" + ispDeviceId.toHex() + "\". Please check target configuration."
                );
            }

            Logger::info("Target signature confirmed: " + ispDeviceId.toHex());

            const auto dwenFuseByte = this->avrIspInterface->readFuse(dwenFuseBitsDescriptor->fuseType).value;
            const auto spienFuseByte = (spienFuseBitsDescriptor->fuseType == dwenFuseBitsDescriptor->fuseType)
                ? dwenFuseByte
                : this->avrIspInterface->readFuse(spienFuseBitsDescriptor->fuseType).value;

            /*
             * Keep in mind that, for AVR fuses and lock bits, a set bit (0b1) means the fuse/lock is cleared, and a
             * cleared bit (0b0), means the fuse/lock is set.
             */

            if ((spienFuseByte & spienFuseBitsDescriptor->bitMask) != 0) {
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
                        "Bloom developers as a matter of urgency, via " + Paths::homeDomainName() + "/report-issue"
                );
            }

            Logger::info("Current SPIEN fuse bit value confirmed");

            if (static_cast<bool>(dwenFuseByte & dwenFuseBitsDescriptor->bitMask) == !setFuse) {
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
                (setFuse) ? static_cast<unsigned char>(dwenFuseByte & ~(dwenFuseBitsDescriptor->bitMask))
                    : static_cast<unsigned char>(dwenFuseByte | dwenFuseBitsDescriptor->bitMask)
            );

            Logger::warning("Programming DWEN fuse bit");
            this->avrIspInterface->programFuse(newFuse);

            if (this->avrIspInterface->readFuse(dwenFuseBitsDescriptor->fuseType).value != newFuse.value) {
                throw Exception("Failed to program fuse bit - post-program value check failed");
            }

            Logger::info("DWEN fuse bit successfully updated");

            this->avrIspInterface->deactivate();

        } catch (const Exception& exception) {
            this->avrIspInterface->deactivate();
            throw exception;
        }
    }
}
