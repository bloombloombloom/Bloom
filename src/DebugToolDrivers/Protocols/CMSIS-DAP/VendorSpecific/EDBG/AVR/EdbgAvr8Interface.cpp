#include "EdbgAvr8Interface.hpp"

#include <cstdint>
#include <thread>
#include <cmath>

#include "src/Logger/Logger.hpp"
#include "src/Helpers/Paths.hpp"

#include "src/Exceptions/InvalidConfig.hpp"
#include "src/TargetController/Exceptions/DeviceInitializationFailure.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/Exceptions/Avr8CommandFailure.hpp"
#include "src/Targets/Microchip/AVR/AVR8/Exceptions/DebugWirePhysicalInterfaceError.hpp"

// Command frames
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/SetParameter.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/GetParameter.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/ActivatePhysical.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/DeactivatePhysical.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/Attach.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/Detach.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/Stop.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/Step.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/Run.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/RunTo.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/GetDeviceId.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/Reset.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/ReadMemory.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/WriteMemory.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/GetProgramCounter.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/SetProgramCounter.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/DisableDebugWire.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/SetSoftwareBreakpoints.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/SetXmegaSoftwareBreakpoint.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/ClearAllSoftwareBreakpoints.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/ClearSoftwareBreakpoints.hpp"

// AVR events
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/Events/AVR8Generic/BreakEvent.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr
{
    using namespace Bloom::Targets::Microchip::Avr;
    using namespace Avr8Bit;
    using namespace Bloom::Exceptions;

    using Bloom::Targets::TargetState;
    using Bloom::Targets::TargetMemoryType;
    using Bloom::Targets::TargetMemoryBuffer;
    using Bloom::Targets::TargetRegister;
    using Bloom::Targets::TargetRegisterDescriptor;
    using Bloom::Targets::TargetRegisterDescriptors;
    using Bloom::Targets::TargetRegisterType;
    using Bloom::Targets::TargetRegisters;

    void EdbgAvr8Interface::configure(const Targets::Microchip::Avr::Avr8Bit::Avr8TargetConfig& targetConfig) {
        this->targetConfig = targetConfig;

        this->configVariant = this->resolveConfigVariant().value_or(Avr8ConfigVariant::NONE);
    }

    void EdbgAvr8Interface::setTargetParameters(const Avr8Bit::TargetParameters& config) {
        this->targetParameters = config;

        if (!config.stackPointerRegisterLowAddress.has_value()) {
            throw DeviceInitializationFailure("Failed to find stack pointer register start address");
        }

        if (!config.stackPointerRegisterSize.has_value()) {
            throw DeviceInitializationFailure("Failed to find stack pointer register size");
        }

        if (!config.statusRegisterStartAddress.has_value()) {
            throw DeviceInitializationFailure("Failed to find status register start address");
        }

        if (!config.statusRegisterSize.has_value()) {
            throw DeviceInitializationFailure("Failed to find status register size");
        }

        if (this->configVariant == Avr8ConfigVariant::NONE) {
            auto configVariant = this->resolveConfigVariant();

            if (!configVariant.has_value()) {
                throw DeviceInitializationFailure("Failed to resolve config variant for the selected "
                    "physical interface and AVR8 family. The selected physical interface is not known to be supported "
                    "by the AVR8 family."
                );
            }

            this->configVariant = configVariant.value();
        }

        if (
            this->configVariant != Avr8ConfigVariant::XMEGA
            && this->configVariant != Avr8ConfigVariant::UPDI
            && config.flashPageSize.has_value() && this->maximumMemoryAccessSizePerRequest.has_value()
            && config.flashPageSize > this->maximumMemoryAccessSizePerRequest
        ) {
            throw DeviceInitializationFailure("Flash page size for target ("
                + std::to_string(config.flashPageSize.value())
                + " bytes) exceeds maximum memory access size for EdbgAvr8Interface ("
                + std::to_string(this->maximumMemoryAccessSizePerRequest.value())
                + " bytes)."
            );
        }

        switch (this->configVariant) {
            case Avr8ConfigVariant::DEBUG_WIRE:
            case Avr8ConfigVariant::MEGAJTAG: {
                this->setDebugWireAndJtagParameters();
                break;
            }
            case Avr8ConfigVariant::XMEGA: {
                this->setPdiParameters();
                break;
            }
            case Avr8ConfigVariant::UPDI: {
                this->setUpdiParameters();
                break;
            }
            default: {
                break;
            }
        }
    }

    void EdbgAvr8Interface::init() {
        if (this->configVariant == Avr8ConfigVariant::XMEGA) {
            // Default PDI clock to 4MHz
            // TODO: Make this adjustable via a target config parameter
            this->setParameter(Avr8EdbgParameters::PDI_CLOCK_SPEED, static_cast<std::uint16_t>(0x0FA0));
        }

        if (this->configVariant == Avr8ConfigVariant::UPDI) {
            // Default UPDI clock to 1.8MHz
            this->setParameter(Avr8EdbgParameters::PDI_CLOCK_SPEED, static_cast<std::uint16_t>(0x0708));
            this->setParameter(Avr8EdbgParameters::ENABLE_HIGH_VOLTAGE_UPDI, static_cast<std::uint8_t>(0x00));
        }

        if (this->configVariant == Avr8ConfigVariant::MEGAJTAG) {
            // Default clock value for mega debugging is 2KHz
            // TODO: Make this adjustable via a target config parameter
            this->setParameter(Avr8EdbgParameters::MEGA_DEBUG_CLOCK, static_cast<std::uint16_t>(0x00C8));
            this->setParameter(Avr8EdbgParameters::JTAG_DAISY_CHAIN_SETTINGS, static_cast<std::uint32_t>(0));
        }

        this->setParameter(
            Avr8EdbgParameters::CONFIG_VARIANT,
            static_cast<std::uint8_t>(this->configVariant)
        );

        this->setParameter(
            Avr8EdbgParameters::CONFIG_FUNCTION,
            static_cast<std::uint8_t>(this->configFunction)
        );

        this->setParameter(
            Avr8EdbgParameters::PHYSICAL_INTERFACE,
            getAvr8PhysicalInterfaceToIdMapping().at(this->targetConfig->physicalInterface)
        );
    }

    void EdbgAvr8Interface::stop() {
        auto commandFrame = CommandFrames::Avr8Generic::Stop();

        auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
        if (response.getResponseId() == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Stop target command failed", response);
        }

        if (this->getTargetState() == TargetState::RUNNING) {
            this->waitForStoppedEvent();
        }
    }

    void EdbgAvr8Interface::run() {
        this->clearEvents();
        auto commandFrame = CommandFrames::Avr8Generic::Run();

        auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
        if (response.getResponseId() == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Run command failed", response);
        }

        this->targetState = TargetState::RUNNING;
    }

    void EdbgAvr8Interface::runTo(std::uint32_t address) {
        this->clearEvents();
        auto commandFrame = CommandFrames::Avr8Generic::RunTo(address);

        auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
        if (response.getResponseId() == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Run-to command failed", response);
        }

        this->targetState = TargetState::RUNNING;
    }

    void EdbgAvr8Interface::step() {
        auto commandFrame = CommandFrames::Avr8Generic::Step();

        auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
        if (response.getResponseId() == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Step target command failed", response);
        }

        this->targetState = TargetState::RUNNING;
    }

    void EdbgAvr8Interface::reset() {
        auto commandFrame = CommandFrames::Avr8Generic::Reset();

        auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
        if (response.getResponseId() == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Reset target command failed", response);
        }

        try {
            // Wait for stopped event
            this->waitForStoppedEvent();

        } catch (const Exception& exception) {
            throw Exception("Failed to reset AVR8 target - missing stopped event.");
        }
    }

    void EdbgAvr8Interface::activate() {
        if (!this->physicalInterfaceActivated) {
            try {
                this->activatePhysical();

            } catch (const Avr8CommandFailure& activationException) {
                if (this->targetConfig->physicalInterface == PhysicalInterface::DEBUG_WIRE
                    && activationException.code == Avr8CommandFailureCode::DEBUGWIRE_PHYSICAL_ERROR
                ) {
                    throw DebugWirePhysicalInterfaceError(
                        "Failed to activate the debugWire physical interface - check target connection. "
                        "If the target was recently programmed via ISP, try cycling the target power. See "
                        + Paths::homeDomainName() + "/docs/debugging-avr-debugwire for more information."
                    );
                }

                throw activationException;
            }
        }

        if (!this->targetAttached) {
            this->attach();
        }
    }

    void EdbgAvr8Interface::deactivate() {
        if (this->targetAttached) {
            if (
                this->targetConfig->physicalInterface == PhysicalInterface::DEBUG_WIRE
                && this->targetConfig->disableDebugWireOnDeactivate
            ) {
                try {
                    this->disableDebugWire();
                    Logger::warning(
                        "Successfully disabled debugWire on the AVR8 target - this is only temporary - "
                            "the debugWire module has lost control of the RESET pin. Bloom may no longer be able to "
                            "interface with the target until the next power cycle."
                    );

                } catch (const Exception& exception) {
                    // Failing to disable debugWire should never prevent us from proceeding with target deactivation.
                    Logger::error(exception.getMessage());
                }
            }

            this->stop();
            this->clearAllBreakpoints();
            this->run();

            this->detach();
        }

        if (this->physicalInterfaceActivated) {
            this->deactivatePhysical();
        }
    }

    std::uint32_t EdbgAvr8Interface::getProgramCounter() {
        if (this->targetState != TargetState::STOPPED) {
            this->stop();
        }

        auto commandFrame = CommandFrames::Avr8Generic::GetProgramCounter();
        auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
        if (response.getResponseId() == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Get program counter command failed", response);
        }

        return response.extractProgramCounter();
    }

    void EdbgAvr8Interface::setProgramCounter(std::uint32_t programCounter) {
        if (this->targetState != TargetState::STOPPED) {
            this->stop();
        }

        /*
         * The program counter will be given in byte address form, but the EDBG tool will be expecting it in word
         * address (16-bit) form. This is why we divide it by 2.
         */
        auto commandFrame = CommandFrames::Avr8Generic::SetProgramCounter(programCounter / 2);
        auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
        if (response.getResponseId() == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Set program counter command failed", response);
        }
    }

    TargetSignature EdbgAvr8Interface::getDeviceId() {
        if (this->configVariant == Avr8ConfigVariant::UPDI) {
            /*
             * When using the UPDI physical interface, the 'Get device ID' command behaves in an odd manner, where it
             * doesn't actually return the target signature, but instead a fixed four byte string reading:
             * 'A', 'V', 'R' and ' ' (white space).
             *
             * So it appears we cannot use that command for UPDI sessions. As an alternative, we will just read the
             * signature from memory using the signature base address.
             *
             * TODO: Currently, we're assuming the signature will always only ever be three bytes in size, but we may
             *       want to consider pulling the size from the TDF.
             */
            auto signatureMemory = this->readMemory(
                Avr8MemoryType::SRAM,
                this->targetParameters.signatureSegmentStartAddress.value(),
                3
            );

            if (signatureMemory.size() != 3) {
                throw Exception("Failed to read AVR8 signature from target - unexpected response size");
            }

            return TargetSignature(signatureMemory[0], signatureMemory[1], signatureMemory[2]);
        }

        auto commandFrame = CommandFrames::Avr8Generic::GetDeviceId();

        auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
        if (response.getResponseId() == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Get device ID command failed", response);
        }

        return response.extractSignature(this->targetConfig->physicalInterface);
    }

    void EdbgAvr8Interface::setBreakpoint(std::uint32_t address) {
        auto commandFrame = CommandFrames::Avr8Generic::SetSoftwareBreakpoints({address});

        auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
        if (response.getResponseId() == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Set software breakpoint command failed", response);
        }
    }

    void EdbgAvr8Interface::clearBreakpoint(std::uint32_t address) {
        auto commandFrame = CommandFrames::Avr8Generic::ClearSoftwareBreakpoints({address});

        auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
        if (response.getResponseId() == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Clear software breakpoint command failed", response);
        }
    }

    void EdbgAvr8Interface::clearAllBreakpoints() {
        auto commandFrame = CommandFrames::Avr8Generic::ClearAllSoftwareBreakpoints();

        auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
        if (response.getResponseId() == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Clear all software breakpoints command failed", response);
        }
    }

    TargetRegisters EdbgAvr8Interface::readRegisters(const TargetRegisterDescriptors& descriptors) {
        /*
         * This function needs to be fast. Insight eagerly requests the values of all known registers that it can
         * present to the user. It does this on numerous occasions (target stopped, user clicked refresh, etc). This
         * means we will be frequently loading over 100 register values in a single instance.
         *
         * For the above reason, we do not read each register value individually. That would take far too long if we
         * have over 100 registers to read. Instead, we group the register descriptors into collections by register
         * type, and resolve the address range for each collection. We then perform a single read operation for
         * each collection and hold the memory buffer in a random access container (std::vector). Finally, we extract
         * the data for each register descriptor, from the memory buffer, and construct the relevant TargetRegister
         * object.
         *
         * TODO: We should be grouping the register descriptors by memory type, as opposed to register type. This
         *       isn't much of a problem ATM, as currently, we only work with registers that are stored in the data
         *       address space or the register file. This will need to be addressed before we can work with any other
         *       registers stored elsewhere.
         */
        auto output = TargetRegisters();

        // Group descriptors by type and resolve the address range for each type
        auto descriptorsByType = std::map<TargetRegisterType, std::set<const TargetRegisterDescriptor*>>();

        /*
         * An address range is just an std::pair of integers - the first being the start address, the second being the
         * end address.
         */
        using AddressRange = std::pair<std::uint32_t, std::uint32_t>;
        auto addressRangeByType = std::map<TargetRegisterType, AddressRange>();

        for (const auto& descriptor : descriptors) {
            if (!descriptor.startAddress.has_value()) {
                Logger::debug(
                    "Attempted to read register in the absence of a start address - register name: "
                        + descriptor.name.value_or("unknown")
                );
                continue;
            }

            descriptorsByType[descriptor.type].insert(&descriptor);

            const auto startAddress = descriptor.startAddress.value();
            const auto endAddress = startAddress + (descriptor.size - 1);

            if (!addressRangeByType.contains(descriptor.type)) {
                auto addressRange = AddressRange();
                addressRange.first = startAddress;
                addressRange.second = endAddress;
                addressRangeByType[descriptor.type] = addressRange;

            } else {
                auto& addressRange = addressRangeByType[descriptor.type];

                if (startAddress < addressRange.first) {
                    addressRange.first = startAddress;
                }

                if (endAddress > addressRange.second) {
                    addressRange.second = endAddress;
                }
            }
        }

        /*
         * Now that we have our address ranges and grouped descriptors, we can perform a single read call for each
         * register type.
         */
        for (const auto&[registerType, descriptors] : descriptorsByType) {
            const auto& addressRange = addressRangeByType[registerType];
            const auto startAddress = addressRange.first;
            const auto endAddress = addressRange.second;
            const auto bufferSize = (endAddress - startAddress) + 1;

            const auto memoryType = (registerType != TargetRegisterType::GENERAL_PURPOSE_REGISTER) ?
                Avr8MemoryType::SRAM
                : (this->configVariant == Avr8ConfigVariant::XMEGA || this->configVariant == Avr8ConfigVariant::UPDI
                ? Avr8MemoryType::REGISTER_FILE : Avr8MemoryType::SRAM);

            /*
             * When reading the entire range, we must avoid any attempts to access the OCD data register (OCDDR), as
             * the debug tool will reject the command and respond with a 0x36 error code (invalid address error).
             *
             * For this reason, we specify the OCDDR address as an excluded address. This will mean
             * the EdbgAvr8Interface::readMemory() function will employ the masked read memory command, as opposed to
             * the general read memory command. The masked read memory command allows us to specify which addresses to
             * read and which ones to ignore. For ignored addresses, the debug tool will just return a 0x00 byte.
             * For more info, see section 7.1.22 titled 'Memory Read Masked', in the EDBG protocol document.
             *
             * Interestingly, the masked read memory command doesn't seem to require us to explicitly specify the OCDDR
             * address as an excluded address. It seems to exclude the OCDDR automatically, even if we've not
             * instructed it to do so. This is plausible, as we send the OCDDR address to the debug tool during target
             * initialisation (see EdbgAvr8Interface::setDebugWireAndJtagParameters()). So this means we don't have to
             * specify the OCDDR address as an excluded address, but the EdbgAvr8Interface::readMemory() function will
             * only employ the masked read memory command when we supply at least one excluded address. For this reason,
             * we still pass the OCDDR address to EdbgAvr8Interface::readMemory(), as an excluded address (provided we
             * have it).
             *
             * See CommandFrames::Avr8Generic::ReadMemory(); and the Microchip EDBG documentation for more.
             */
            auto excludedAddresses = std::set<std::uint32_t>();
            if (memoryType == Avr8MemoryType::SRAM && this->targetParameters.ocdDataRegister.has_value()) {
                excludedAddresses.insert(
                    this->targetParameters.ocdDataRegister.value()
                        + this->targetParameters.mappedIoSegmentStartAddress.value_or(0)
                );
            }

            const auto flatMemoryBuffer = this->readMemory(
                memoryType,
                startAddress,
                bufferSize,
                excludedAddresses
            );

            if (flatMemoryBuffer.size() != bufferSize) {
                throw Exception(
                    "Failed to read memory within register type address range (" + std::to_string(startAddress)
                        + " - " + std::to_string(endAddress) + "). Expected " + std::to_string(bufferSize)
                        + " bytes, got " + std::to_string(flatMemoryBuffer.size())
                );
            }

            // Construct our TargetRegister objects directly from the flat memory buffer
            for (const auto& descriptor : descriptors) {
                /*
                 * Multi-byte AVR8 registers are stored in LSB form.
                 *
                 * This is why we use reverse iterators when extracting our data from flatMemoryBuffer. Doing so allows
                 * us to extract the data in MSB form (as is expected for all register values held in TargetRegister
                 * objects).
                 */
                const auto bufferStartIt = flatMemoryBuffer.rend() - (descriptor->startAddress.value() - startAddress)
                    - descriptor->size;

                output.emplace_back(
                    TargetRegister(
                        *descriptor,
                        TargetMemoryBuffer(bufferStartIt, bufferStartIt + descriptor->size)
                    )
                );
            }
        }

        return output;
    }

    void EdbgAvr8Interface::writeRegisters(const Targets::TargetRegisters& registers) {
        for (const auto& reg : registers) {
            const auto& registerDescriptor = reg.descriptor;
            auto registerValue = reg.value;

            if (registerValue.empty()) {
                throw Exception("Cannot write empty register value");
            }

            if (registerValue.size() > registerDescriptor.size) {
                throw Exception("Register value exceeds size specified by register descriptor.");
            }

            if (registerValue.size() < registerDescriptor.size) {
                // Fill the missing most-significant bytes with 0x00
                registerValue.insert(registerValue.begin(), registerDescriptor.size - registerValue.size(), 0x00);
            }

            if (registerValue.size() > 1) {
                // AVR8 registers are stored in LSB
                std::reverse(registerValue.begin(), registerValue.end());
            }

            auto memoryType = Avr8MemoryType::SRAM;
            if (
                registerDescriptor.type == TargetRegisterType::GENERAL_PURPOSE_REGISTER
                && (this->configVariant == Avr8ConfigVariant::XMEGA || this->configVariant == Avr8ConfigVariant::UPDI)
            ) {
                memoryType = Avr8MemoryType::REGISTER_FILE;
            }

            // TODO: This can be inefficient when updating many registers, maybe do something a little smarter here.
            this->writeMemory(
                memoryType,
                registerDescriptor.startAddress.value(),
                registerValue
            );
        }
    }

    TargetMemoryBuffer EdbgAvr8Interface::readMemory(
        TargetMemoryType memoryType,
        std::uint32_t startAddress,
        std::uint32_t bytes,
        const std::set<Targets::TargetMemoryAddressRange>& excludedAddressRanges
    ) {
        auto avr8MemoryType = Avr8MemoryType::SRAM;

        switch (memoryType) {
            case TargetMemoryType::RAM: {
                avr8MemoryType = Avr8MemoryType::SRAM;
                break;
            }
            case TargetMemoryType::FLASH: {
                if (this->configVariant == Avr8ConfigVariant::DEBUG_WIRE) {
                    avr8MemoryType = Avr8MemoryType::FLASH_PAGE;

                } else if (
                    this->configVariant == Avr8ConfigVariant::XMEGA
                    || this->configVariant == Avr8ConfigVariant::UPDI
                ) {
                    avr8MemoryType = Avr8MemoryType::APPL_FLASH;

                } else {
                    avr8MemoryType = Avr8MemoryType::SPM;
                }
                break;
            }
            case TargetMemoryType::EEPROM: {
                avr8MemoryType = Avr8MemoryType::EEPROM;
            }
            default: {
                break;
            }
        }

        /*
         * The internal readMemory() function accepts excluded addresses in the form of a set of addresses, as
         * opposed to a set of address ranges.
         *
         * We will perform the conversion here.
         */
        auto excludedAddresses = std::set<std::uint32_t>();
        auto endAddress = startAddress + bytes - 1;

        for (const auto& addressRange : excludedAddressRanges) {
            if (addressRange.startAddress > endAddress) {
                // This address range is outside of the range from which we will be reading
                continue;
            }

            for (auto i = addressRange.startAddress; i <= addressRange.endAddress; i++) {
                excludedAddresses.insert(i);
            }
        }

        return this->readMemory(avr8MemoryType, startAddress, bytes, excludedAddresses);
    }

    void EdbgAvr8Interface::writeMemory(
        TargetMemoryType memoryType,
        std::uint32_t startAddress,
        const TargetMemoryBuffer& buffer
    ) {
        auto avr8MemoryType = Avr8MemoryType::SRAM;

        switch (memoryType) {
            case TargetMemoryType::RAM: {
                avr8MemoryType = Avr8MemoryType::SRAM;
                break;
            }
            case TargetMemoryType::FLASH: {
                if (this->configVariant == Avr8ConfigVariant::DEBUG_WIRE) {
                    avr8MemoryType = Avr8MemoryType::FLASH_PAGE;

                } else if (this->configVariant == Avr8ConfigVariant::MEGAJTAG) {
                    avr8MemoryType = Avr8MemoryType::FLASH_PAGE;
                    // TODO: Enable programming mode

                } else if (this->configVariant == Avr8ConfigVariant::XMEGA
                    || this->configVariant == Avr8ConfigVariant::UPDI) {
                    avr8MemoryType = Avr8MemoryType::APPL_FLASH;

                } else {
                    avr8MemoryType = Avr8MemoryType::SPM;
                }
                break;
            }
            case TargetMemoryType::EEPROM: {
                avr8MemoryType = Avr8MemoryType::EEPROM;
            }
            default: {
                break;
            }
        }

        return this->writeMemory(avr8MemoryType, startAddress, buffer);
    }

    TargetState EdbgAvr8Interface::getTargetState() {
        /*
         * We are not informed when a target goes from a stopped state to a running state, so there is no need
         * to query the tool when we already know the target has stopped.
         *
         * This means we have to rely on the assumption that the target cannot enter a running state without
         * our instruction.
         */
        if (this->targetState != TargetState::STOPPED) {
            this->refreshTargetState();
        }

        return this->targetState;
    }

    void EdbgAvr8Interface::setParameter(const Avr8EdbgParameter& parameter, const std::vector<unsigned char>& value) {
        auto commandFrame = CommandFrames::Avr8Generic::SetParameter(parameter, value);

        auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
        if (response.getResponseId() == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("Failed to set parameter on device!", response);
        }
    }

    std::vector<unsigned char> EdbgAvr8Interface::getParameter(const Avr8EdbgParameter& parameter, std::uint8_t size) {
        auto commandFrame = CommandFrames::Avr8Generic::GetParameter(parameter, size);

        auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
        if (response.getResponseId() == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("Failed to get parameter from device!", response);
        }

        return response.getPayloadData();
    }

    void EdbgAvr8Interface::setDebugWireAndJtagParameters() {
        if (this->targetParameters.flashPageSize.has_value()) {
            Logger::debug("Setting DEVICE_FLASH_PAGE_SIZE AVR8 parameter");
            this->setParameter(
                Avr8EdbgParameters::DEVICE_FLASH_PAGE_SIZE,
                this->targetParameters.flashPageSize.value()
            );
        }

        if (this->targetParameters.flashSize.has_value()) {
            Logger::debug("Setting DEVICE_FLASH_SIZE AVR8 parameter");
            this->setParameter(
                Avr8EdbgParameters::DEVICE_FLASH_SIZE,
                this->targetParameters.flashSize.value()
            );
        }

        if (this->targetParameters.flashStartAddress.has_value()) {
            Logger::debug("Setting DEVICE_FLASH_BASE AVR8 parameter");
            this->setParameter(
                Avr8EdbgParameters::DEVICE_FLASH_BASE,
                this->targetParameters.flashStartAddress.value()
            );
        }

        if (this->targetParameters.ramStartAddress.has_value()) {
            Logger::debug("Setting DEVICE_SRAM_START AVR8 parameter");
            this->setParameter(
                Avr8EdbgParameters::DEVICE_SRAM_START,
                this->targetParameters.ramStartAddress.value()
            );
        }

        if (this->targetParameters.eepromSize.has_value()) {
            Logger::debug("Setting DEVICE_EEPROM_SIZE AVR8 parameter");
            this->setParameter(
                Avr8EdbgParameters::DEVICE_EEPROM_SIZE,
                this->targetParameters.eepromSize.value()
            );
        }

        if (this->targetParameters.eepromPageSize.has_value()) {
            Logger::debug("Setting DEVICE_EEPROM_PAGE_SIZE AVR8 parameter");
            this->setParameter(
                Avr8EdbgParameters::DEVICE_EEPROM_PAGE_SIZE,
                this->targetParameters.eepromPageSize.value()
            );
        }

        if (this->targetParameters.ocdRevision.has_value()) {
            Logger::debug("Setting DEVICE_OCD_REVISION AVR8 parameter");
            this->setParameter(
                Avr8EdbgParameters::DEVICE_OCD_REVISION,
                this->targetParameters.ocdRevision.value()
            );
        }

        if (this->targetParameters.ocdDataRegister.has_value()) {
            Logger::debug("Setting DEVICE_OCD_DATA_REGISTER AVR8 parameter");
            this->setParameter(
                Avr8EdbgParameters::DEVICE_OCD_DATA_REGISTER,
                this->targetParameters.ocdDataRegister.value()
            );
        }

        if (this->targetParameters.spmcRegisterStartAddress.has_value()) {
            Logger::debug("Setting DEVICE_SPMCR_REGISTER AVR8 parameter");
            this->setParameter(
                Avr8EdbgParameters::DEVICE_SPMCR_REGISTER,
                this->targetParameters.spmcRegisterStartAddress.value()
            );
        }

        if (this->targetParameters.bootSectionStartAddress.has_value()) {
            Logger::debug("Setting DEVICE_BOOT_START_ADDR AVR8 parameter");
            this->setParameter(
                Avr8EdbgParameters::DEVICE_BOOT_START_ADDR,
                this->targetParameters.bootSectionStartAddress.value()
            );
        }

        /*
         * All addresses for registers that reside in the mapped IO memory segment include the mapped IO segment offset
         * (start address). But the EDBG protocol requires *some* of these addresses to be stripped of this offset
         * before sending them as target parameters.
         *
         * This applies to the following addresses:
         *
         *  - OSCALL Address
         *  - EEARL Address
         *  - EEARH Address
         *  - EECR Address
         *  - EEDR Address
         *
         * It *doesn't* seem to apply to the SPMCR or OCDDR address.
         */
        auto mappedIoStartAddress = this->targetParameters.mappedIoSegmentStartAddress.value_or(0);

        if (this->targetParameters.osccalAddress.has_value()) {
            Logger::debug("Setting DEVICE_OSCCAL_ADDR AVR8 parameter");
            this->setParameter(
                Avr8EdbgParameters::DEVICE_OSCCAL_ADDR,
                static_cast<std::uint8_t>(
                    this->targetParameters.osccalAddress.value() - mappedIoStartAddress
                )
            );
        }

        if (this->targetParameters.eepromAddressRegisterLow.has_value()) {
            Logger::debug("Setting DEVICE_EEARL_ADDR AVR8 parameter");
            this->setParameter(
                Avr8EdbgParameters::DEVICE_EEARL_ADDR,
                static_cast<std::uint8_t>(
                    this->targetParameters.eepromAddressRegisterLow.value() - mappedIoStartAddress
                )
            );
        }

        if (this->targetParameters.eepromAddressRegisterHigh.has_value()) {
            Logger::debug("Setting DEVICE_EEARH_ADDR AVR8 parameter");
            this->setParameter(
                Avr8EdbgParameters::DEVICE_EEARH_ADDR,
                static_cast<std::uint8_t>(
                    this->targetParameters.eepromAddressRegisterHigh.value() - mappedIoStartAddress
                )
            );
        }

        if (this->targetParameters.eepromControlRegisterAddress.has_value()) {
            Logger::debug("Setting DEVICE_EECR_ADDR AVR8 parameter");
            this->setParameter(
                Avr8EdbgParameters::DEVICE_EECR_ADDR,
                static_cast<std::uint8_t>(
                    this->targetParameters.eepromControlRegisterAddress.value() - mappedIoStartAddress
                )
            );
        }

        if (this->targetParameters.eepromDataRegisterAddress.has_value()) {
            Logger::debug("Setting DEVICE_EEDR_ADDR AVR8 parameter");
            this->setParameter(
                Avr8EdbgParameters::DEVICE_EEDR_ADDR,
                static_cast<std::uint8_t>(
                    this->targetParameters.eepromDataRegisterAddress.value() - mappedIoStartAddress
                )
            );
        }
    }

    void EdbgAvr8Interface::setPdiParameters() {
        if (!this->targetParameters.appSectionPdiOffset.has_value()) {
            throw DeviceInitializationFailure("Missing required parameter: APPL_BASE_ADDR");
        }

        if (!this->targetParameters.bootSectionPdiOffset.has_value()) {
            throw DeviceInitializationFailure("Missing required parameter: BOOT_BASE_ADDR");
        }

        if (!this->targetParameters.bootSectionSize.has_value()) {
            throw DeviceInitializationFailure("Missing required parameter: BOOT_BYTES");
        }

        if (!this->targetParameters.flashSize.has_value()) {
            throw DeviceInitializationFailure("Missing required parameter: APPLICATION_BYTES");
        }

        if (!this->targetParameters.eepromPdiOffset.has_value()) {
            throw DeviceInitializationFailure("Missing required parameter: EEPROM_BASE_ADDR");
        }

        if (!this->targetParameters.fuseRegistersPdiOffset.has_value()) {
            throw DeviceInitializationFailure("Missing required parameter: FUSE_BASE_ADDR");
        }

        if (!this->targetParameters.lockRegistersPdiOffset.has_value()) {
            throw DeviceInitializationFailure("Missing required parameter: LOCKBIT_BASE_ADDR");
        }

        if (!this->targetParameters.userSignaturesPdiOffset.has_value()) {
            throw DeviceInitializationFailure("Missing required parameter: USER_SIGN_BASE_ADDR");
        }

        if (!this->targetParameters.productSignaturesPdiOffset.has_value()) {
            throw DeviceInitializationFailure("Missing required parameter: PROD_SIGN_BASE_ADDR");
        }

        if (!this->targetParameters.ramPdiOffset.has_value()) {
            throw DeviceInitializationFailure("Missing required parameter: DATA_BASE_ADDR");
        }

        if (!this->targetParameters.flashPageSize.has_value()) {
            throw DeviceInitializationFailure("Missing required parameter: FLASH_PAGE_BYTES");
        }

        if (!this->targetParameters.eepromSize.has_value()) {
            throw DeviceInitializationFailure("Missing required parameter: EEPROM_SIZE");
        }

        if (!this->targetParameters.eepromPageSize.has_value()) {
            throw DeviceInitializationFailure("Missing required parameter: EEPROM_PAGE_SIZE");
        }

        if (!this->targetParameters.nvmBaseAddress.has_value()) {
            throw DeviceInitializationFailure("Missing required parameter: NVM_BASE");
        }

        Logger::debug("Setting APPL_BASE_ADDR AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_XMEGA_APPL_BASE_ADDR,
            this->targetParameters.appSectionPdiOffset.value()
        );

        Logger::debug("Setting BOOT_BASE_ADDR AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_XMEGA_BOOT_BASE_ADDR,
            this->targetParameters.bootSectionPdiOffset.value()
        );

        Logger::debug("Setting EEPROM_BASE_ADDR AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_XMEGA_EEPROM_BASE_ADDR,
            this->targetParameters.eepromPdiOffset.value()
        );

        Logger::debug("Setting FUSE_BASE_ADDR AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_XMEGA_FUSE_BASE_ADDR,
            this->targetParameters.fuseRegistersPdiOffset.value()
        );

        Logger::debug("Setting LOCKBIT_BASE_ADDR AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_XMEGA_LOCKBIT_BASE_ADDR,
            this->targetParameters.lockRegistersPdiOffset.value()
        );

        Logger::debug("Setting USER_SIGN_BASE_ADDR AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_XMEGA_USER_SIGN_BASE_ADDR,
            this->targetParameters.userSignaturesPdiOffset.value()
        );

        Logger::debug("Setting PROD_SIGN_BASE_ADDR AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_XMEGA_PROD_SIGN_BASE_ADDR,
            this->targetParameters.productSignaturesPdiOffset.value()
        );

        Logger::debug("Setting DATA_BASE_ADDR AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_XMEGA_DATA_BASE_ADDR,
            this->targetParameters.ramPdiOffset.value()
        );

        Logger::debug("Setting APPLICATION_BYTES AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_XMEGA_APPLICATION_BYTES,
            this->targetParameters.flashSize.value()
        );

        Logger::debug("Setting BOOT_BYTES AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_XMEGA_BOOT_BYTES,
            this->targetParameters.bootSectionSize.value()
        );

        Logger::debug("Setting FLASH_PAGE_BYTES AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_XMEGA_FLASH_PAGE_BYTES,
            this->targetParameters.flashPageSize.value()
        );

        Logger::debug("Setting EEPROM_SIZE AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_XMEGA_EEPROM_SIZE,
            this->targetParameters.eepromSize.value()
        );

        Logger::debug("Setting EEPROM_PAGE_SIZE AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_XMEGA_EEPROM_PAGE_SIZE,
            static_cast<std::uint8_t>(this->targetParameters.eepromPageSize.value())
        );

        Logger::debug("Setting NVM_BASE AVR8 parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_XMEGA_NVM_BASE,
            this->targetParameters.nvmBaseAddress.value()
        );
    }

    void EdbgAvr8Interface::setUpdiParameters() {
        if (!this->targetParameters.signatureSegmentStartAddress.has_value()) {
            throw DeviceInitializationFailure("Missing required parameter: SIGNATURE BASE ADDRESS");
        }

        if (this->targetParameters.programMemoryUpdiStartAddress.has_value()) {
            /*
             * The program memory base address field for UPDI sessions (DEVICE_UPDI_PROGMEM_BASE_ADDR) seems to be
             * limited to two bytes in size, as opposed to the four byte size for the debugWire, JTAG and PDI
             * equivalent fields. This is why, I suspect, another field was required for the most significant byte of
             * the program memory base address (DEVICE_UPDI_PROGMEM_BASE_ADDR_MSB).
             *
             * The additional DEVICE_UPDI_PROGMEM_BASE_ADDR_MSB field is only one byte in size, so it brings the total
             * capacity for the program memory base address to three bytes. Because of this, we ensure that all TDFs,
             * for targets that support UPDI, specify an address that does not exceed the maximum value of a 24 bit
             * unsigned integer. This is done in our TDF validation script (see src/Targets/TargetDescription/README.md
             * for more).
             */
            const auto programMemBaseAddress = this->targetParameters.programMemoryUpdiStartAddress.value();
            Logger::debug("Setting DEVICE_UPDI_PROGMEM_BASE_ADDR AVR8 parameter");
            this->setParameter(
                Avr8EdbgParameters::DEVICE_UPDI_PROGMEM_BASE_ADDR,
                static_cast<std::uint16_t>(programMemBaseAddress)
            );

            Logger::debug("Setting DEVICE_UPDI_PROGMEM_BASE_ADDR_MSB AVR8 parameter");
            this->setParameter(
                Avr8EdbgParameters::DEVICE_UPDI_PROGMEM_BASE_ADDR_MSB,
                static_cast<std::uint8_t>(programMemBaseAddress >> 16)
            );
        }

        if (this->targetParameters.flashPageSize.has_value()) {
            /*
             * See the comment above regarding capacity limitations of the DEVICE_UPDI_PROGMEM_BASE_ADDR field.
             *
             * The same applies here, for the flash page size field (DEVICE_UPDI_FLASH_PAGE_SIZE).
             */
            auto flashPageSize = this->targetParameters.flashPageSize.value();
            Logger::debug("Setting DEVICE_UPDI_FLASH_PAGE_SIZE AVR8 parameter");
            this->setParameter(
                Avr8EdbgParameters::DEVICE_UPDI_FLASH_PAGE_SIZE,
                static_cast<std::uint8_t>(flashPageSize)
            );

            Logger::debug("Setting DEVICE_UPDI_FLASH_PAGE_SIZE_MSB AVR8 parameter");
            this->setParameter(
                Avr8EdbgParameters::DEVICE_UPDI_FLASH_PAGE_SIZE_MSB,
                static_cast<std::uint8_t>(flashPageSize >> 8)
            );
        }

        if (this->targetParameters.eepromPageSize.has_value()) {
            Logger::debug("Setting DEVICE_UPDI_EEPROM_PAGE_SIZE AVR8 parameter");
            this->setParameter(
                Avr8EdbgParameters::DEVICE_UPDI_EEPROM_PAGE_SIZE,
                this->targetParameters.eepromPageSize.value()
            );
        }

        if (this->targetParameters.nvmBaseAddress.has_value()) {
            Logger::debug("Setting DEVICE_UPDI_NVMCTRL_ADDR AVR8 parameter");
            this->setParameter(
                Avr8EdbgParameters::DEVICE_UPDI_NVMCTRL_ADDR,
                this->targetParameters.nvmBaseAddress.value()
            );
        }

        if (this->targetParameters.ocdModuleAddress.has_value()) {
            Logger::debug("Setting DEVICE_UPDI_OCD_ADDR AVR8 parameter");
            this->setParameter(
                Avr8EdbgParameters::DEVICE_UPDI_OCD_ADDR,
                this->targetParameters.ocdModuleAddress.value()
            );
        }

        if (this->targetParameters.flashSize.has_value()) {
            Logger::debug("Setting DEVICE_UPDI_FLASH_SIZE AVR8 parameter");
            this->setParameter(
                Avr8EdbgParameters::DEVICE_UPDI_FLASH_SIZE,
                this->targetParameters.flashSize.value()
            );
        }

        if (this->targetParameters.eepromSize.has_value()) {
            Logger::debug("Setting DEVICE_UPDI_EEPROM_SIZE AVR8 parameter");
            this->setParameter(
                Avr8EdbgParameters::DEVICE_UPDI_EEPROM_SIZE,
                this->targetParameters.eepromSize.value()
            );
        }

        if (this->targetParameters.eepromStartAddress.has_value()) {
            Logger::debug("Setting DEVICE_UPDI_EEPROM_BASE_ADDR AVR8 parameter");
            this->setParameter(
                Avr8EdbgParameters::DEVICE_UPDI_EEPROM_BASE_ADDR,
                this->targetParameters.eepromStartAddress.value()
            );
        }

        if (this->targetParameters.signatureSegmentStartAddress.has_value()) {
            Logger::debug("Setting DEVICE_UPDI_SIG_BASE_ADDR AVR8 parameter");
            this->setParameter(
                Avr8EdbgParameters::DEVICE_UPDI_SIG_BASE_ADDR,
                this->targetParameters.signatureSegmentStartAddress.value()
            );
        }

        if (this->targetParameters.fuseSegmentStartAddress.has_value()) {
            Logger::debug("Setting DEVICE_UPDI_FUSE_BASE_ADDR AVR8 parameter");
            this->setParameter(
                Avr8EdbgParameters::DEVICE_UPDI_FUSE_BASE_ADDR,
                this->targetParameters.fuseSegmentStartAddress.value()
            );
        }

        if (this->targetParameters.fuseSegmentSize.has_value()) {
            Logger::debug("Setting DEVICE_UPDI_FUSE_SIZE AVR8 parameter");
            this->setParameter(
                Avr8EdbgParameters::DEVICE_UPDI_FUSE_SIZE,
                this->targetParameters.fuseSegmentSize.value()
            );
        }

        if (this->targetParameters.lockbitsSegmentStartAddress.has_value()) {
            Logger::debug("Setting DEVICE_UPDI_LOCK_BASE_ADDR AVR8 parameter");
            this->setParameter(
                Avr8EdbgParameters::DEVICE_UPDI_LOCK_BASE_ADDR,
                this->targetParameters.lockbitsSegmentStartAddress.value()
            );
        }

        this->setParameter(
            Avr8EdbgParameters::DEVICE_UPDI_24_BIT_ADDRESSING_ENABLE,
            this->targetParameters.programMemoryUpdiStartAddress.value_or(0) > 0xFFFF ?
            static_cast<std::uint8_t>(1) : static_cast<std::uint8_t>(0)
        );
    }

    void EdbgAvr8Interface::activatePhysical(bool applyExternalReset) {
        auto commandFrame = CommandFrames::Avr8Generic::ActivatePhysical(applyExternalReset);

        auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
        if (response.getResponseId() == Avr8ResponseId::FAILED) {
            if (!applyExternalReset) {
                // Try again with external reset applied
                Logger::debug("Failed to activate physical interface on AVR8 target "
                    "- retrying with external reset applied.");
                return this->activatePhysical(true);
            }

            throw Avr8CommandFailure("AVR8 Activate physical interface command failed", response);
        }

        this->physicalInterfaceActivated = true;
    }

    void EdbgAvr8Interface::deactivatePhysical() {
        auto commandFrame = CommandFrames::Avr8Generic::DeactivatePhysical();

        auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
        if (response.getResponseId() == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Deactivate physical interface command failed", response);
        }

        this->physicalInterfaceActivated = false;
    }

    void EdbgAvr8Interface::attach() {
        /*
         * When attaching an ATmega target that is connected via JTAG, we must not set the breakAfterAttach flag, as
         * this results in a timeout.
         *
         * However, in this case the attach command seems to _sometimes_ halt the target anyway, regardless of the
         * value of the breakAfterAttach flag. So we still expect a stop event to be received shortly after issuing
         * the attach command.
         */
        auto commandFrame = CommandFrames::Avr8Generic::Attach(
            this->configVariant != Avr8ConfigVariant::MEGAJTAG
        );

        auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
        if (response.getResponseId() == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Attach command failed", response);
        }

        this->targetAttached = true;

        try {
            // Wait for stopped event
            this->waitForStoppedEvent();

        } catch (const Exception& exception) {
            Logger::warning(
                "Execution on AVR8 target could not be halted post attach - " + exception.getMessage()
            );
        }
    }

    void EdbgAvr8Interface::detach() {
        auto commandFrame = CommandFrames::Avr8Generic::Detach();

        auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
        if (response.getResponseId() == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Detach command failed", response);
        }

        this->targetAttached = false;
    }

    std::unique_ptr<AvrEvent> EdbgAvr8Interface::getAvrEvent() {
        auto event = this->edbgInterface.requestAvrEvent();

        if (!event.has_value()) {
            return nullptr;
        }

        switch (event->getEventId()) {
            case AvrEventId::AVR8_BREAK_EVENT: {
                // Break event
                return std::make_unique<BreakEvent>(event.value());
            }
            default: {
                /*
                 * TODO: This isn't very nice as we're performing an unnecessary copy. Maybe requestAvrEvents should
                 *       return a unique_ptr instead?
                 */
                return std::make_unique<AvrEvent>(event.value());
            }
        }
    }

    void EdbgAvr8Interface::clearEvents() {
        while (this->getAvrEvent() != nullptr) {}
    }

    TargetMemoryBuffer EdbgAvr8Interface::readMemory(
        Avr8MemoryType type,
        std::uint32_t startAddress,
        std::uint32_t bytes,
        const std::set<std::uint32_t>& excludedAddresses
    ) {
        if (!excludedAddresses.empty() && (this->avoidMaskedMemoryRead || type != Avr8MemoryType::SRAM)) {
            /*
             * Driver-side masked memory read.
             *
             * Split the read into numerous reads, whenever we encounter an excluded address.
             *
             * All values for bytes located at excluded addresses will be returned as 0x00 - this mirrors the behaviour
             * of the masked read memory EDBG command.
             */
            auto output = TargetMemoryBuffer();
            output.reserve(bytes);

            auto segmentStartAddress = startAddress;
            const auto endAddress = startAddress + bytes - 1;

            for (const auto excludedAddress : excludedAddresses) {
                if (excludedAddress < startAddress || excludedAddress > endAddress) {
                    // This excluded address is outside of the range from which we are reading, so it can be ignored.
                    continue;
                }

                const auto segmentSize = excludedAddress - segmentStartAddress;
                if (segmentSize > 0) {
                    auto segmentBuffer = this->readMemory(
                        type,
                        segmentStartAddress,
                        segmentSize
                    );

                    output.insert(output.end(), segmentBuffer.begin(), segmentBuffer.end());
                }

                output.emplace_back(0x00);

                segmentStartAddress = excludedAddress + 1;
            }

            // Read final segment
            const auto finalReadBytes = (endAddress - segmentStartAddress) + 1;
            if (finalReadBytes > 0) {
                auto segmentBuffer = this->readMemory(
                    type,
                    segmentStartAddress,
                    finalReadBytes
                );

                output.insert(output.end(), segmentBuffer.begin(), segmentBuffer.end());
            }

            return output;
        }

        if (type == Avr8MemoryType::FLASH_PAGE) {
            // Flash reads must be done in pages
            auto pageSize = this->targetParameters.flashPageSize.value_or(0);

            if (pageSize < 1
                || (
                    this->maximumMemoryAccessSizePerRequest.has_value()
                    && pageSize > this->maximumMemoryAccessSizePerRequest
                )
            ) {
                throw Exception("Missing/invalid flash page size parameter");
            }

            if ((bytes % pageSize) != 0 || (startAddress % pageSize) != 0) {
                /*
                 * The number of bytes to read and/or the start address are not aligned.
                 *
                 * Align both and call this function again.
                 */
                auto alignedAddress = startAddress;
                auto alignedBytesToRead = bytes;

                if ((bytes % pageSize) != 0) {
                    auto pagesRequired = static_cast<std::uint32_t>(std::ceil(
                        static_cast<float>(bytes) / static_cast<float>(pageSize)
                    ));

                    alignedBytesToRead = (pagesRequired * pageSize);
                }

                if ((startAddress % pageSize) != 0) {
                    alignedAddress = static_cast<std::uint32_t>(std::floor(
                        static_cast<float>(startAddress) / static_cast<float>(pageSize)
                    ) * pageSize);

                    /*
                     * Given that we've pushed the start address back, this must be accounted for in the number of
                     * bytes to read. We'll need to include the difference as those are the bytes we're actually
                     * interested in.
                     *
                     * For example:
                     *
                     * Given: page size = 4
                     * Given: dummy memory (each character represents one byte, with first byte at 0x00) = aaaabbbbccccdddd
                     * Given: requested start address = 0x05
                     * Given: requested bytes to read = 4
                     *
                     * The start address (0x05) would be:
                     * aaaabbbbccccdddd
                     *      ^
                     * Because only 4 bytes were requested, starting at address 0x05, we're only interested in the bytes
                     * at addresses 0x05, 0x06, 0x07 and 0x08 (that's bytes bbbc).
                     *
                     * But the start address isn't aligned, so we need to align it by pushing it back to the beginning
                     * of the page (so we'd set it to 0x04, for this example), which is what we do above, when setting
                     * alignedAddress:
                     * aaaabbbbccccdddd
                     *     ^
                     * But now we'll only be reading 4 bytes from start address 0x04, meaning we won't be reading
                     * that 4th byte (0x08). So we need to account for this by adding the difference of the requested
                     * start address and the aligned start address to the number of bytes to read, to ensure that we're
                     * reading all of the bytes that we're interested in. But this will throw off the aligned bytes!!
                     * So we need to also account for this by aligning the additional bytes before adding them to
                     * alignedBytesToRead.
                     *
                     * However, we could simply get away with just adding the bytes without aligning
                     * them (alignedBytesToRead += (address - alignedAddress);), as the subsequent recursive call will
                     * align them for us, but it will result in an unnecessary recursion, so we'll just align the
                     * additional bytes here.
                     */
                    if ((startAddress - alignedAddress) > (alignedBytesToRead - bytes)) {
                        alignedBytesToRead += static_cast<std::uint32_t>(std::ceil(
                            static_cast<float>(startAddress - alignedAddress) / static_cast<float>(pageSize)
                        )) * pageSize;
                    }
                }

                /*
                 * Now that the start address and bytes to read have been aligned, we can simply invoke this function
                 * for a second time, with the aligned values. Then, return the requested data and discard the rest.
                 */
                auto memoryBuffer = this->readMemory(type, alignedAddress, alignedBytesToRead, excludedAddresses);
                return TargetMemoryBuffer(
                    memoryBuffer.begin() + (startAddress - alignedAddress),
                    memoryBuffer.begin() + (startAddress - alignedAddress) + bytes
                );
            }

            // We can only read one flash page at a time.
            if (bytes > pageSize) {
                // bytes should always be a multiple of pageSize (given the code above)
                assert(bytes % pageSize == 0);
                int pagesRequired = static_cast<int>(bytes / pageSize);
                TargetMemoryBuffer memoryBuffer;

                for (auto i = 1; i <= pagesRequired; i++) {
                    auto pageBuffer = this->readMemory(type, startAddress + (pageSize * i), pageSize);
                    memoryBuffer.insert(memoryBuffer.end(), pageBuffer.begin(), pageBuffer.end());
                }

                return memoryBuffer;
            }

        }

        /*
         * Enforce a maximum memory access request size.
         *
         * See the comment for EdbgAvr8Interface::setMaximumMemoryAccessSizePerRequest() for more on this.
         */
        if (this->maximumMemoryAccessSizePerRequest.has_value() && bytes > this->maximumMemoryAccessSizePerRequest) {
            auto maximumRequestSize = this->maximumMemoryAccessSizePerRequest.value();
            auto totalReadsRequired = std::ceil(static_cast<float>(bytes) / static_cast<float>(maximumRequestSize));
            auto output = std::vector<unsigned char>();
            output.reserve(bytes);

            for (float i = 1; i <= totalReadsRequired; i++) {
                auto bytesToRead = static_cast<std::uint32_t>((bytes - output.size()) > maximumRequestSize ?
                                                              maximumRequestSize : bytes - output.size());
                auto data = this->readMemory(
                    type,
                    static_cast<std::uint32_t>(startAddress + output.size()),
                    bytesToRead,
                    excludedAddresses
                );
                output.insert(output.end(), data.begin(), data.end());
            }

            return output;
        }

        if (type != Avr8MemoryType::FLASH_PAGE) {
            /*
             * EDBG AVR8 debug tools behave in a really weird way when responding with more than two packets
             * for a single read (non-flash) memory command. The data they return in this case appears to be of little
             * use.
             *
             * To address this, we make sure we only issue read memory commands that will result in no more than two
             * response packets. For calls that require more than this, we simply split them into numerous calls.
             */

            /*
             * The subtraction of 20 bytes here is just to account for any other bytes included in the response
             * that isn't actually the memory data (like the command ID, version bytes, etc). I could have sought the
             * actual value but who has the time. It won't exceed 20 bytes. Bite me.
             */
            auto singlePacketSize = static_cast<std::uint32_t>(this->edbgInterface.getUsbHidInputReportSize() - 20);
            auto totalResponsePackets = std::ceil(static_cast<float>(bytes) / static_cast<float>(singlePacketSize));
            auto totalReadsRequired = std::ceil(static_cast<float>(totalResponsePackets) / 2);

            if (totalResponsePackets > 2) {
                /*
                 * This call to readMemory() will result in more than two response packets, so split it into multiple
                 * calls that will result in no more than two response packets per call.
                 */
                auto output = std::vector<unsigned char>();

                for (float i = 1; i <= totalReadsRequired; i++) {
                    auto bytesToRead = static_cast<std::uint32_t>(
                        (bytes - output.size()) > (singlePacketSize * 2) ? (singlePacketSize * 2)
                        : bytes - output.size()
                    );
                    auto data = this->readMemory(
                        type,
                        static_cast<std::uint32_t>(startAddress + output.size()),
                        bytesToRead,
                        excludedAddresses
                    );
                    output.insert(output.end(), data.begin(), data.end());
                }

                return output;
            }
        }

        auto commandFrame = CommandFrames::Avr8Generic::ReadMemory(
            type,
            startAddress,
            bytes,
            excludedAddresses
        );

        auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
        if (response.getResponseId() == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Read memory command failed", response);
        }

        return response.getMemoryBuffer();
    }

    void EdbgAvr8Interface::writeMemory(Avr8MemoryType type, std::uint32_t address, const TargetMemoryBuffer& buffer) {
        if (type == Avr8MemoryType::FLASH_PAGE) {
            // TODO: Implement support for writing to flash
            throw Exception("Writing to flash memory is not supported.");
        }

        auto commandFrame = CommandFrames::Avr8Generic::WriteMemory(
            type,
            address,
            buffer
        );

        auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
        if (response.getResponseId() == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Write memory command failed", response);
        }
    }

    void EdbgAvr8Interface::refreshTargetState() {
        auto avrEvent = this->getAvrEvent();

        if (avrEvent != nullptr && avrEvent->getEventId() == AvrEventId::AVR8_BREAK_EVENT) {
            auto* breakEvent = dynamic_cast<BreakEvent*>(avrEvent.get());

            if (breakEvent == nullptr) {
                throw Exception("Failed to process AVR8 break event");
            }

            this->targetState = TargetState::STOPPED;
            return;
        }

        this->targetState = TargetState::RUNNING;
    }

    void EdbgAvr8Interface::disableDebugWire() {
        auto commandFrame = CommandFrames::Avr8Generic::DisableDebugWire();

        auto response = this->edbgInterface.sendAvrCommandFrameAndWaitForResponseFrame(commandFrame);
        if (response.getResponseId() == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Disable debugWire command failed", response);
        }
    }

    void EdbgAvr8Interface::waitForStoppedEvent() {
        auto breakEvent = this->waitForAvrEvent<BreakEvent>();

        if (breakEvent == nullptr) {
            throw Exception("Failed to receive break event for AVR8 target");
        }

        this->targetState = TargetState::STOPPED;
    }
}
