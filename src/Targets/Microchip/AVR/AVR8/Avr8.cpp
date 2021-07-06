#include <cstdint>
#include <QtCore>
#include <QJsonDocument>
#include <cassert>
#include <bitset>
#include <limits>

#include "Avr8.hpp"
#include "PadDescriptor.hpp"
#include "PhysicalInterface.hpp"
#include "src/Logger/Logger.hpp"
#include "src/Exceptions/InvalidConfig.hpp"
#include "src/Targets/TargetRegister.hpp"
#include "src/Targets/Microchip/AVR/AVR8/TargetDescription/TargetDescriptionFile.hpp"

// Derived AVR8 targets
#include "XMega/XMega.hpp"
#include "Mega/Mega.hpp"
#include "Tiny/Tiny.hpp"

using namespace Bloom;
using namespace Bloom::Targets;
using namespace Bloom::Targets::Microchip::Avr;
using namespace Bloom::Targets::Microchip::Avr::Avr8Bit;
using namespace Exceptions;

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
}

TargetSignature Avr8::getId() {
    if (!this->id.has_value()) {
        this->id = this->avr8Interface->getDeviceId();
    }

    return this->id.value();
}

void Avr8::preActivationConfigure(const TargetConfig& targetConfig) {
    Target::preActivationConfigure(targetConfig);

    if (this->family.has_value()) {
        this->avr8Interface->setFamily(this->family.value());
    }

    this->avr8Interface->configure(targetConfig);
}

void Avr8::postActivationConfigure() {
    if (!this->targetDescriptionFile.has_value()) {
        this->loadTargetDescriptionFile();
        this->initFromTargetDescriptionFile();
    }

    /*
     * The signature obtained from the device should match what is in the target description file
     *
     * We don't use this->getId() here as that could return the ID that was extracted from the target description file
     * (which it would, if the user specified the exact target name in their project config - see Avr8::getId() and
     * TargetController::getSupportedTargets() for more).
     */
    auto targetSignature = this->avr8Interface->getDeviceId();
    auto tdSignature = this->targetDescriptionFile->getTargetSignature();

    if (targetSignature != tdSignature) {
        throw Exception("Failed to validate connected target - target signature mismatch.\nThe target signature"
            " (\"" + targetSignature.toHex() + "\") does not match the AVR8 target description signature (\""
            + tdSignature.toHex() + "\"). This will likely be due to an incorrect target name in the configuration file"
            + " (bloom.json)."
        );
    }
}

void Avr8::postPromotionConfigure() {
    if (!this->family.has_value()) {
        throw Exception("Failed to resolve AVR8 family");
    }

    this->avr8Interface->setFamily(this->family.value());
    this->avr8Interface->setTargetParameters(this->targetParameters.value());
}

void Avr8::activate() {
    if (this->isActivated()) {
        return;
    }

    this->avr8Interface->init();

    if (this->targetDescriptionFile.has_value()) {
        this->avr8Interface->setTargetParameters(this->targetParameters.value());
    }

    this->avr8Interface->activate();
    this->activated = true;
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

void Avr8::deactivate() {
    try {
        this->avr8Interface->deactivate();
        this->activated = false;

    } catch (const Exception& exception) {
        Logger::error("Failed to deactivate AVR8 target - " + exception.getMessage());
    }
}

TargetDescriptor Avr8Bit::Avr8::getDescriptor() {
    auto descriptor = TargetDescriptor();
    descriptor.id = this->getHumanReadableId();
    descriptor.name = this->getName();
    descriptor.ramSize = this->targetParameters.value().ramSize.value_or(0);

    std::transform(
        this->targetVariantsById.begin(),
        this->targetVariantsById.end(),
        std::back_inserter(descriptor.variants),
        [](auto& variantToIdPair) {
            return variantToIdPair.second;
        }
    );

    return descriptor;
}

void Avr8::run() {
    this->avr8Interface->run();
}

void Avr8::stop() {
    this->avr8Interface->stop();
}

void Avr8::step() {
    this->avr8Interface->step();
}

void Avr8::reset() {
    this->avr8Interface->reset();
}

void Avr8::setBreakpoint(std::uint32_t address) {
    this->avr8Interface->setBreakpoint(address);
}

void Avr8::removeBreakpoint(std::uint32_t address) {
    this->avr8Interface->clearBreakpoint(address);
}

void Avr8::clearAllBreakpoints() {
    this->avr8Interface->clearAllBreakpoints();
}

TargetRegisters Avr8::readGeneralPurposeRegisters(std::set<std::size_t> registerIds) {
    return this->avr8Interface->readGeneralPurposeRegisters(registerIds);
}

void Avr8::writeRegisters(const TargetRegisters& registers) {
    TargetRegisters gpRegisters;

    for (const auto& targetRegister : registers) {
        if (targetRegister.descriptor.type == TargetRegisterType::GENERAL_PURPOSE_REGISTER
            && targetRegister.descriptor.id.has_value()) {
            gpRegisters.push_back(targetRegister);

        } else if (targetRegister.descriptor.type == TargetRegisterType::PROGRAM_COUNTER) {
            auto programCounterBytes = targetRegister.value;

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

        } else if (targetRegister.descriptor.type == TargetRegisterType::STATUS_REGISTER) {
            this->avr8Interface->setStatusRegister(targetRegister);

        } else if (targetRegister.descriptor.type == TargetRegisterType::STACK_POINTER) {
            this->avr8Interface->setStackPointerRegister(targetRegister);
        }
    }

    if (!gpRegisters.empty()) {
        this->avr8Interface->writeGeneralPurposeRegisters(gpRegisters);
    }
}

TargetRegisters Avr8::readRegisters(const TargetRegisterDescriptors& descriptors) {
    TargetRegisters registers;
    std::set<std::size_t> gpRegisterIds;

    for (const auto& descriptor : descriptors) {
        if (descriptor.type == TargetRegisterType::GENERAL_PURPOSE_REGISTER && descriptor.id.has_value()) {
            gpRegisterIds.insert(descriptor.id.value());

        } else if (descriptor.type == TargetRegisterType::PROGRAM_COUNTER) {
            registers.push_back(this->getProgramCounterRegister());

        } else if (descriptor.type == TargetRegisterType::STATUS_REGISTER) {
            registers.push_back(this->getStatusRegister());

        } else if (descriptor.type == TargetRegisterType::STACK_POINTER) {
            registers.push_back(this->getStackPointerRegister());
        }
    }

    if (!gpRegisterIds.empty()) {
        auto gpRegisters = this->readGeneralPurposeRegisters(gpRegisterIds);
        registers.insert(registers.end(), gpRegisters.begin(), gpRegisters.end());
    }

    return registers;
}

TargetMemoryBuffer Avr8::readMemory(TargetMemoryType memoryType, std::uint32_t startAddress, std::uint32_t bytes) {
    return this->avr8Interface->readMemory(memoryType, startAddress, bytes);
}

void Avr8::writeMemory(TargetMemoryType memoryType, std::uint32_t startAddress, const TargetMemoryBuffer& buffer) {
    this->avr8Interface->writeMemory(memoryType, startAddress, buffer);
}

TargetState Avr8::getState() {
    return this->avr8Interface->getTargetState();
}

std::uint32_t Avr8::getProgramCounter() {
    return this->avr8Interface->getProgramCounter();
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

TargetRegister Avr8::getStackPointerRegister() {
    return this->avr8Interface->getStackPointerRegister();
}

TargetRegister Avr8::getStatusRegister() {
    return this->avr8Interface->getStatusRegister();
}

void Avr8::setProgramCounter(std::uint32_t programCounter) {
    this->avr8Interface->setProgramCounter(programCounter);
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
     * We may be able to make this more efficient by combining reads for ports with aligned memory addresses. This will
     * be considered when the need for it becomes apparent.
     */
    std::map<std::uint16_t, TargetMemoryBuffer> cachedMemoryByStartAddress;
    auto readMemoryBitset = [this, &cachedMemoryByStartAddress](std::uint16_t startAddress) {
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

                if (pinState.ioDirection == TargetPinState::IoDirection::OUTPUT && pad.gpioPortSetAddress.has_value()) {
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

void Avr8::setPinState(int variantId, const TargetPinDescriptor& pinDescriptor, const TargetPinState& state) {
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

    if (!padDescriptor.ddrSetAddress.has_value()
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
            auto portSetRegisterValue = this->readMemory(TargetMemoryType::RAM, portSetAddress, 1);

            if (portSetRegisterValue.empty()) {
                throw Exception("Failed to read PORT register value");
            }

            auto portSetBitset = std::bitset<std::numeric_limits<unsigned char>::digits>(portSetRegisterValue.front());
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
            auto portClearRegisterValue = this->readMemory(TargetMemoryType::RAM, portClearAddress, 1);

            if (portClearRegisterValue.empty()) {
                throw Exception("Failed to read PORT (OUTSET) register value");
            }

            auto portClearBitset = std::bitset<std::numeric_limits<unsigned char>::digits>(portClearRegisterValue.front());
            if (portClearBitset.test(pinNumber) == (ioState == TargetPinState::IoState::LOW)) {
                // PORT clear register needs updating
                portClearBitset.set(pinNumber, (ioState == TargetPinState::IoState::LOW));

                this->writeMemory(
                    TargetMemoryType::RAM,
                    portClearAddress,
                    {static_cast<unsigned char>(portClearBitset.to_ulong())}
                );
            }
        }
    }
}

bool Avr8::memoryAddressRangeClashesWithIoPortRegisters(
    TargetMemoryType memoryType,
    std::uint32_t startAddress,
    std::uint32_t endAddress
) {
    auto& targetParameters = this->targetParameters.value();
    if (targetParameters.mappedIoSegmentStartAddress.has_value() && targetParameters.mappedIoSegmentSize.has_value()) {
        auto mappedIoSegmentStart = targetParameters.mappedIoSegmentStartAddress.value();
        auto mappedIoSegmentEnd = mappedIoSegmentStart + targetParameters.mappedIoSegmentSize.value();

        return (startAddress >= mappedIoSegmentStart && startAddress <= mappedIoSegmentEnd)
            || (endAddress >= mappedIoSegmentStart && endAddress <= mappedIoSegmentEnd)
            || (startAddress <= mappedIoSegmentStart && endAddress >= mappedIoSegmentStart)
        ;
    }

    return false;
}
