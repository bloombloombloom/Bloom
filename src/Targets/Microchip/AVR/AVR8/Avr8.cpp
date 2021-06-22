#include <cstdint>
#include <QtCore>
#include <QJsonDocument>
#include <cassert>
#include <bitset>
#include <limits>

#include "Avr8.hpp"
#include "PadDescriptor.hpp"
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

/**
 * Initialises the target from config parameters extracted from user's config file.
 *
 * @see Application::extractConfig(); for more on config extraction.
 *
 * @param targetConfig
 */
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
    this->avr8Interface->setTargetParameters(this->getTargetParameters());
}

void Avr8::loadTargetDescriptionFile() {
    auto targetSignature = this->getId();
    auto targetDescriptionFile = TargetDescription::TargetDescriptionFile(
        targetSignature,
        (!this->name.empty()) ? std::optional(this->name) : std::nullopt
    );

    this->targetDescriptionFile = targetDescriptionFile;
    this->name = targetDescriptionFile.getTargetName();
    this->family = targetDescriptionFile.getFamily();

    this->loadPadDescriptors();
    this->loadTargetVariants();
}

void Avr8::loadPadDescriptors() {
    auto& targetParameters = this->getTargetParameters();

    /*
     * Every port address we extract from the target description will be stored in portAddresses, so that
     * we can extract the start (min) and end (max) for the target's IO port address
     * range (TargetParameters::ioPortAddressRangeStart & TargetParameters::ioPortAddressRangeEnd)
     */
    std::vector<std::uint32_t> portAddresses;

    auto& modules = this->targetDescriptionFile->getModulesMappedByName();
    auto portModule = (modules.contains("port")) ? std::optional(modules.find("port")->second) : std::nullopt;
    auto& peripheralModules = this->targetDescriptionFile->getPeripheralModulesMappedByName();

    if (peripheralModules.contains("port")) {
        auto portPeripheralModule = peripheralModules.find("port")->second;

        for (const auto& [instanceName, instance] : portPeripheralModule.instancesMappedByName) {
            if (instanceName.find("port") == 0) {
                auto portPeripheralRegisterGroup = (portPeripheralModule.registerGroupsMappedByName.contains(instanceName)) ?
                    std::optional(portPeripheralModule.registerGroupsMappedByName.find(instanceName)->second) :
                    std::nullopt;

                for (const auto& signal : instance.instanceSignals) {
                    if (!signal.index.has_value()) {
                        continue;
                    }

                    auto padDescriptor = PadDescriptor();
                    padDescriptor.name = signal.padName;
                    padDescriptor.gpioPinNumber = signal.index.value();

                    if (portModule.has_value() && portModule->registerGroupsMappedByName.contains(instanceName)) {
                        // We have register information for this port
                        auto registerGroup = portModule->registerGroupsMappedByName.find(instanceName)->second;

                        for (const auto& [registerName, portRegister] : registerGroup.registersMappedByName) {
                            if (registerName.find("port") == 0) {
                                // This is the data register for the port
                                padDescriptor.gpioPortSetAddress = portRegister.offset;
                                padDescriptor.gpioPortClearAddress = portRegister.offset;

                            } else if (registerName.find("pin") == 0) {
                                // This is the input data register for the port
                                padDescriptor.gpioPortInputAddress = portRegister.offset;

                            } else if (registerName.find("ddr") == 0) {
                                // This is the data direction register for the port
                                padDescriptor.ddrSetAddress = portRegister.offset;
                                padDescriptor.ddrClearAddress = portRegister.offset;
                            }
                        }

                    } else if (portModule.has_value() && portModule->registerGroupsMappedByName.contains("port")) {
                        // We have generic register information for all ports on the target
                        auto registerGroup = portModule->registerGroupsMappedByName.find("port")->second;

                        for (const auto& [registerName, portRegister] : registerGroup.registersMappedByName) {
                            if (registerName.find("outset") == 0) {
                                // Include the port register offset
                                padDescriptor.gpioPortSetAddress = (portPeripheralRegisterGroup.has_value()
                                    && portPeripheralRegisterGroup->offset.has_value()) ?
                                    portPeripheralRegisterGroup->offset.value_or(0) : 0;

                                padDescriptor.gpioPortSetAddress = padDescriptor.gpioPortSetAddress.value()
                                    + portRegister.offset;

                            } else if (registerName.find("outclr") == 0) {
                                padDescriptor.gpioPortClearAddress = (portPeripheralRegisterGroup.has_value()
                                    && portPeripheralRegisterGroup->offset.has_value()) ?
                                    portPeripheralRegisterGroup->offset.value_or(0) : 0;

                                padDescriptor.gpioPortClearAddress = padDescriptor.gpioPortClearAddress.value()
                                    + portRegister.offset;

                            } else if (registerName.find("dirset") == 0) {
                                padDescriptor.ddrSetAddress = (portPeripheralRegisterGroup.has_value()
                                    && portPeripheralRegisterGroup->offset.has_value()) ?
                                    portPeripheralRegisterGroup->offset.value_or(0) : 0;

                                padDescriptor.ddrSetAddress = padDescriptor.ddrSetAddress.value()
                                    + portRegister.offset;

                            } else if (registerName.find("dirclr") == 0) {
                                padDescriptor.ddrClearAddress = (portPeripheralRegisterGroup.has_value()
                                    && portPeripheralRegisterGroup->offset.has_value()) ?
                                    portPeripheralRegisterGroup->offset.value_or(0) : 0;

                                padDescriptor.ddrClearAddress = padDescriptor.ddrClearAddress.value()
                                    + portRegister.offset;

                            } else if (registerName == "in") {
                                padDescriptor.gpioPortInputAddress = (portPeripheralRegisterGroup.has_value()
                                    && portPeripheralRegisterGroup->offset.has_value()) ?
                                    portPeripheralRegisterGroup->offset.value_or(0) : 0;

                                padDescriptor.gpioPortInputAddress = padDescriptor.gpioPortInputAddress.value()
                                    + portRegister.offset;
                            }
                        }
                    }

                    if (padDescriptor.gpioPortSetAddress.has_value()) {
                        portAddresses.push_back(padDescriptor.gpioPortSetAddress.value());
                    }

                    if (padDescriptor.gpioPortClearAddress.has_value()) {
                        portAddresses.push_back(padDescriptor.gpioPortClearAddress.value());
                    }

                    if (padDescriptor.ddrSetAddress.has_value()) {
                        portAddresses.push_back(padDescriptor.ddrSetAddress.value());
                    }

                    if (padDescriptor.ddrClearAddress.has_value()) {
                        portAddresses.push_back(padDescriptor.ddrClearAddress.value());
                    }

                    this->padDescriptorsByName.insert(std::pair(padDescriptor.name, padDescriptor));
                }
            }
        }
    }

    // TODO: Move this into getTargetParameters()
    if (!portAddresses.empty()) {
        targetParameters.ioPortAddressRangeStart = *std::min_element(portAddresses.begin(), portAddresses.end());
        targetParameters.ioPortAddressRangeEnd = *std::max_element(portAddresses.begin(), portAddresses.end());
    }
}

void Avr8::loadTargetVariants() {
    auto tdVariants = this->targetDescriptionFile->getVariants();
    auto tdPinoutsByName = this->targetDescriptionFile->getPinoutsMappedByName();
    auto& modules = this->targetDescriptionFile->getModulesMappedByName();

    for (const auto& tdVariant : tdVariants) {
        if (tdVariant.disabled) {
            continue;
        }

        auto targetVariant = TargetVariant();
        targetVariant.id = static_cast<int>(this->targetVariantsById.size());
        targetVariant.name = tdVariant.name;
        targetVariant.packageName = tdVariant.package;

        if (tdVariant.package.find("QFP") == 0 || tdVariant.package.find("TQFP") == 0) {
            targetVariant.package = TargetPackage::QFP;

        } else if (tdVariant.package.find("PDIP") == 0 || tdVariant.package.find("DIP") == 0) {
            targetVariant.package = TargetPackage::DIP;

        } else if (tdVariant.package.find("QFN") == 0 || tdVariant.package.find("VQFN") == 0) {
            targetVariant.package = TargetPackage::QFN;

        } else if (tdVariant.package.find("SOIC") == 0) {
            targetVariant.package = TargetPackage::SOIC;

        } else if (tdVariant.package.find("SSOP") == 0) {
            targetVariant.package = TargetPackage::SSOP;
        }

        if (!tdPinoutsByName.contains(tdVariant.pinoutName)) {
            // Missing pinouts in the target description file
            continue;
        }

        auto tdPinout = tdPinoutsByName.find(tdVariant.pinoutName)->second;
        for (const auto& tdPin : tdPinout.pins) {
            auto targetPin = TargetPinDescriptor();
            targetPin.name = tdPin.pad;
            targetPin.padName = tdPin.pad;
            targetPin.number = tdPin.position;

            // TODO: REMOVE THIS:
            if (tdPin.pad.find("vcc") == 0 || tdPin.pad.find("avcc") == 0 || tdPin.pad.find("aref") == 0) {
                targetPin.type = TargetPinType::VCC;

            } else if (tdPin.pad.find("gnd") == 0) {
                targetPin.type = TargetPinType::GND;
            }

            if (this->padDescriptorsByName.contains(targetPin.padName)) {
                auto& pad = this->padDescriptorsByName.at(targetPin.padName);
                if (pad.gpioPortSetAddress.has_value() && pad.ddrSetAddress.has_value()) {
                    targetPin.type = TargetPinType::GPIO;
                }
            }

            targetVariant.pinDescriptorsByNumber.insert(std::pair(targetPin.number, targetPin));
        }

        this->targetVariantsById.insert(std::pair(targetVariant.id, targetVariant));
    }
}

TargetParameters& Avr8::getTargetParameters() {
    if (!this->targetParameters.has_value()) {
        assert(this->targetDescriptionFile.has_value());
        this->targetParameters = TargetParameters();
        auto& propertyGroups = this->targetDescriptionFile->getPropertyGroupsMappedByName();

        auto flashMemorySegment = this->targetDescriptionFile->getFlashMemorySegment();
        if (flashMemorySegment.has_value()) {
            this->targetParameters->flashSize = flashMemorySegment->size;
            this->targetParameters->flashStartAddress = flashMemorySegment->startAddress;

            if (flashMemorySegment->pageSize.has_value()) {
                this->targetParameters->flashPageSize = flashMemorySegment->pageSize.value();
            }
        }

        auto ramMemorySegment = this->targetDescriptionFile->getRamMemorySegment();
        if (ramMemorySegment.has_value()) {
            this->targetParameters->ramSize = ramMemorySegment->size;
            this->targetParameters->ramStartAddress = ramMemorySegment->startAddress;
        }

        auto registerMemorySegment = this->targetDescriptionFile->getRegisterMemorySegment();
        if (registerMemorySegment.has_value()) {
            this->targetParameters->gpRegisterSize = registerMemorySegment->size;
            this->targetParameters->gpRegisterStartAddress = registerMemorySegment->startAddress;
        }

        auto eepromMemorySegment = this->targetDescriptionFile->getEepromMemorySegment();
        if (eepromMemorySegment.has_value()) {
            this->targetParameters->eepromSize = eepromMemorySegment->size;

            if (eepromMemorySegment->pageSize.has_value()) {
                this->targetParameters->eepromPageSize = eepromMemorySegment->pageSize.value();
            }
        }

        auto firstBootSectionMemorySegment = this->targetDescriptionFile->getFirstBootSectionMemorySegment();
        if (firstBootSectionMemorySegment.has_value()) {
            this->targetParameters->bootSectionStartAddress = firstBootSectionMemorySegment->startAddress / 2;
            this->targetParameters->bootSectionSize = firstBootSectionMemorySegment->size;
        }

        // OCD attributes can be found in property groups
        if (propertyGroups.contains("ocd")) {
            auto& ocdProperties = propertyGroups.at("ocd").propertiesMappedByName;

            if (ocdProperties.find("ocd_revision") != ocdProperties.end()) {
                this->targetParameters->ocdRevision = ocdProperties.find("ocd_revision")->second.value.toUShort(nullptr, 10);
            }

            if (ocdProperties.find("ocd_datareg") != ocdProperties.end()) {
                this->targetParameters->ocdDataRegister = ocdProperties.find("ocd_datareg")->second.value.toUShort(nullptr, 16);
            }
        }

        auto statusRegister = this->targetDescriptionFile->getStatusRegister();
        if (statusRegister.has_value()) {
            this->targetParameters->statusRegisterStartAddress = statusRegister->offset;
            this->targetParameters->statusRegisterSize = statusRegister->size;
        }

        auto stackPointerRegister = this->targetDescriptionFile->getStackPointerRegister();
        if (stackPointerRegister.has_value()) {
            this->targetParameters->stackPointerRegisterStartAddress = stackPointerRegister->offset;
            this->targetParameters->stackPointerRegisterSize = stackPointerRegister->size;

        } else {
            // Sometimes the SP register is split into two register nodes, one for low, the other for high
            auto stackPointerLowRegister = this->targetDescriptionFile->getStackPointerLowRegister();
            auto stackPointerHighRegister = this->targetDescriptionFile->getStackPointerHighRegister();

            if (stackPointerLowRegister.has_value()) {
                this->targetParameters->stackPointerRegisterStartAddress = stackPointerLowRegister->offset;
                this->targetParameters->stackPointerRegisterSize = stackPointerLowRegister->size;
            }

            if (stackPointerHighRegister.has_value()) {
                this->targetParameters->stackPointerRegisterSize = (this->targetParameters->stackPointerRegisterSize.has_value()) ?
                   this->targetParameters->stackPointerRegisterSize.value() + stackPointerHighRegister->size :
                   stackPointerHighRegister->size;
            }
        }

        auto spmcsRegister = this->targetDescriptionFile->getSpmcsRegister();
        if (spmcsRegister.has_value()) {
            this->targetParameters->spmcRegisterStartAddress = spmcsRegister->offset;

        } else {
            auto spmcRegister = this->targetDescriptionFile->getSpmcRegister();
            if (spmcRegister.has_value()) {
                this->targetParameters->spmcRegisterStartAddress = spmcRegister->offset;
            }
        }

        auto osccalRegister = this->targetDescriptionFile->getOscillatorCalibrationRegister();
        if (osccalRegister.has_value()) {
            this->targetParameters->osccalAddress = osccalRegister->offset;
        }

        auto eepromAddressRegister = this->targetDescriptionFile->getEepromAddressRegister();
        if (eepromAddressRegister.has_value()) {
            this->targetParameters->eepromAddressRegisterLow = eepromAddressRegister->offset;
            this->targetParameters->eepromAddressRegisterHigh = (eepromAddressRegister->size == 2)
                ? eepromAddressRegister->offset + 1 : eepromAddressRegister->offset;

        } else {
            auto eepromAddressLowRegister = this->targetDescriptionFile->getEepromAddressLowRegister();
            if (eepromAddressLowRegister.has_value()) {
                this->targetParameters->eepromAddressRegisterLow = eepromAddressLowRegister->offset;
                auto eepromAddressHighRegister = this->targetDescriptionFile->getEepromAddressHighRegister();

                if (eepromAddressHighRegister.has_value()) {
                    this->targetParameters->eepromAddressRegisterHigh = eepromAddressHighRegister->offset;

                } else {
                    this->targetParameters->eepromAddressRegisterHigh = eepromAddressLowRegister->offset;
                }
            }
        }

        auto eepromDataRegister = this->targetDescriptionFile->getEepromDataRegister();
        if (eepromDataRegister.has_value()) {
            this->targetParameters->eepromDataRegisterAddress = eepromDataRegister->offset;
        }

        auto eepromControlRegister = this->targetDescriptionFile->getEepromControlRegister();
        if (eepromControlRegister.has_value()) {
            this->targetParameters->eepromControlRegisterAddress = eepromControlRegister->offset;
        }

        if (propertyGroups.contains("pdi_interface")) {
            auto& pdiInterfaceProperties = propertyGroups.at("pdi_interface").propertiesMappedByName;

            if (pdiInterfaceProperties.contains("app_section_offset")) {
                this->targetParameters->appSectionPdiOffset = pdiInterfaceProperties
                    .at("app_section_offset").value.toInt(nullptr, 16);
            }

            if (pdiInterfaceProperties.contains("boot_section_offset")) {
                this->targetParameters->bootSectionPdiOffset = pdiInterfaceProperties
                    .at("boot_section_offset").value.toInt(nullptr, 16);
            }

            if (pdiInterfaceProperties.contains("datamem_offset")) {
                this->targetParameters->ramPdiOffset = pdiInterfaceProperties
                    .at("datamem_offset").value.toInt(nullptr, 16);
            }

            if (pdiInterfaceProperties.contains("eeprom_offset")) {
                this->targetParameters->eepromPdiOffset = pdiInterfaceProperties
                    .at("eeprom_offset").value.toInt(nullptr, 16);
            }

            if (pdiInterfaceProperties.contains("user_signatures_offset")) {
                this->targetParameters->userSignaturesPdiOffset = pdiInterfaceProperties
                    .at("user_signatures_offset").value.toInt(nullptr, 16);
            }

            if (pdiInterfaceProperties.contains("prod_signatures_offset")) {
                this->targetParameters->productSignaturesPdiOffset = pdiInterfaceProperties
                    .at("prod_signatures_offset").value.toInt(nullptr, 16);
            }

            if (pdiInterfaceProperties.contains("fuse_registers_offset")) {
                this->targetParameters->fuseRegistersPdiOffset = pdiInterfaceProperties
                    .at("fuse_registers_offset").value.toInt(nullptr, 16);
            }

            if (pdiInterfaceProperties.contains("lock_registers_offset")) {
                this->targetParameters->lockRegistersPdiOffset = pdiInterfaceProperties
                    .at("lock_registers_offset").value.toInt(nullptr, 16);
            }

            auto& peripheralModules = this->targetDescriptionFile->getPeripheralModulesMappedByName();

            if (peripheralModules.contains("nvm")) {
                auto& nvmModule = peripheralModules.at("nvm");

                if (nvmModule.instancesMappedByName.contains("nvm")) {
                    auto& nvmInstance = nvmModule.instancesMappedByName.at("nvm");

                    if (nvmInstance.registerGroupsMappedByName.contains("nvm")) {
                        this->targetParameters->nvmBaseAddress = nvmInstance.registerGroupsMappedByName.at("nvm").offset;
                    }
                }
            }
        }
    }

    return this->targetParameters.value();
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

void Avr8::activate() {
    if (this->isActivated()) {
        return;
    }

    this->avr8Interface->init();
    this->avr8Interface->activate();
    this->activated = true;
}

void Avr8::deactivate() {
    try {
        this->avr8Interface->deactivate();
        this->activated = false;

    } catch (const Exception& exception) {
        Logger::error("Failed to deactivate AVR8 target - " + exception.getMessage());
    }
}

TargetSignature Avr8::getId() {
    if (!this->id.has_value()) {
        this->id = this->avr8Interface->getDeviceId();
    }

    return this->id.value();
}

TargetDescriptor Avr8Bit::Avr8::getDescriptor() {
    auto parameters = this->getTargetParameters();

    auto descriptor = TargetDescriptor();
    descriptor.id = this->getHumanReadableId();
    descriptor.name = this->getName();
    descriptor.ramSize = parameters.ramSize.value_or(0);

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
        static_cast<unsigned char>(programCounter),
        static_cast<unsigned char>(programCounter >> 8),
        static_cast<unsigned char>(programCounter >> 16),
        static_cast<unsigned char>(programCounter >> 24),
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

        if (padDescriptor.gpioPortClearAddress.has_value() && padDescriptor.gpioPortClearAddress != portSetAddress) {
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

bool Avr8::memoryAddressRangeClashesWithIoPortRegisters(TargetMemoryType memoryType, std::uint32_t startAddress, std::uint32_t endAddress) {
    auto& targetParameters = this->getTargetParameters();

    /*
     * We're making an assumption here; that all IO port addresses for all AVR8 targets are aligned. I have no idea
     * how well this will hold.
     *
     * If they're not aligned, this function may report false positives.
     */
    if (targetParameters.ioPortAddressRangeStart.has_value() && targetParameters.ioPortAddressRangeEnd.has_value()) {
        return (
            startAddress >= targetParameters.ioPortAddressRangeStart
            && startAddress <= targetParameters.ioPortAddressRangeEnd
        ) || (
            endAddress >= targetParameters.ioPortAddressRangeStart
            && endAddress <= targetParameters.ioPortAddressRangeEnd
        ) || (
            startAddress <= targetParameters.ioPortAddressRangeStart
            && endAddress >= targetParameters.ioPortAddressRangeStart
        );
    }

    return false;
}
