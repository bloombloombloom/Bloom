#include "EdbgAvr8Interface.hpp"

#include <thread>
#include <cassert>
#include <cmath>

#include "src/Services/PathService.hpp"
#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"

#include "Exceptions/Avr8CommandFailure.hpp"
#include "src/TargetController/Exceptions/DeviceInitializationFailure.hpp"
#include "src/Targets/Microchip/AVR/AVR8/Exceptions/DebugWirePhysicalInterfaceError.hpp"

// Command frames
#include "CommandFrames/AVR8Generic/SetParameter.hpp"
#include "CommandFrames/AVR8Generic/GetParameter.hpp"
#include "CommandFrames/AVR8Generic/ActivatePhysical.hpp"
#include "CommandFrames/AVR8Generic/DeactivatePhysical.hpp"
#include "CommandFrames/AVR8Generic/Attach.hpp"
#include "CommandFrames/AVR8Generic/Detach.hpp"
#include "CommandFrames/AVR8Generic/Stop.hpp"
#include "CommandFrames/AVR8Generic/Step.hpp"
#include "CommandFrames/AVR8Generic/Run.hpp"
#include "CommandFrames/AVR8Generic/RunTo.hpp"
#include "CommandFrames/AVR8Generic/GetDeviceId.hpp"
#include "CommandFrames/AVR8Generic/Reset.hpp"
#include "CommandFrames/AVR8Generic/ReadMemory.hpp"
#include "CommandFrames/AVR8Generic/WriteMemory.hpp"
#include "CommandFrames/AVR8Generic/GetProgramCounter.hpp"
#include "CommandFrames/AVR8Generic/SetProgramCounter.hpp"
#include "CommandFrames/AVR8Generic/DisableDebugWire.hpp"
#include "CommandFrames/AVR8Generic/SetSoftwareBreakpoints.hpp"
#include "CommandFrames/AVR8Generic/ClearAllSoftwareBreakpoints.hpp"
#include "CommandFrames/AVR8Generic/ClearSoftwareBreakpoints.hpp"
#include "CommandFrames/AVR8Generic/SetHardwareBreakpoint.hpp"
#include "CommandFrames/AVR8Generic/ClearHardwareBreakpoint.hpp"
#include "CommandFrames/AVR8Generic/EnterProgrammingMode.hpp"
#include "CommandFrames/AVR8Generic/LeaveProgrammingMode.hpp"
#include "CommandFrames/AVR8Generic/EraseMemory.hpp"

#include "Parameters/AVR8Generic/DebugWireJtagParameters.hpp"
#include "Parameters/AVR8Generic/PdiParameters.hpp"
#include "Parameters/AVR8Generic/UpdiParameters.hpp"

// AVR events
#include "Events/AVR8Generic/BreakEvent.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr
{
    using namespace Targets::Microchip::Avr;
    using namespace Avr8Bit;
    using namespace Exceptions;

    using CommandFrames::Avr8Generic::Stop;
    using CommandFrames::Avr8Generic::Run;
    using CommandFrames::Avr8Generic::RunTo;
    using CommandFrames::Avr8Generic::Step;
    using CommandFrames::Avr8Generic::Reset;
    using CommandFrames::Avr8Generic::GetProgramCounter;
    using CommandFrames::Avr8Generic::SetProgramCounter;
    using CommandFrames::Avr8Generic::GetDeviceId;
    using CommandFrames::Avr8Generic::SetSoftwareBreakpoints;
    using CommandFrames::Avr8Generic::ClearSoftwareBreakpoints;
    using CommandFrames::Avr8Generic::ClearAllSoftwareBreakpoints;
    using CommandFrames::Avr8Generic::SetHardwareBreakpoint;
    using CommandFrames::Avr8Generic::ClearHardwareBreakpoint;
    using CommandFrames::Avr8Generic::ReadMemory;
    using CommandFrames::Avr8Generic::EnterProgrammingMode;
    using CommandFrames::Avr8Generic::LeaveProgrammingMode;
    using CommandFrames::Avr8Generic::SetParameter;
    using CommandFrames::Avr8Generic::GetParameter;
    using CommandFrames::Avr8Generic::ActivatePhysical;
    using CommandFrames::Avr8Generic::DeactivatePhysical;
    using CommandFrames::Avr8Generic::Attach;
    using CommandFrames::Avr8Generic::Detach;
    using CommandFrames::Avr8Generic::ReadMemory;
    using CommandFrames::Avr8Generic::WriteMemory;
    using CommandFrames::Avr8Generic::EraseMemory;
    using CommandFrames::Avr8Generic::DisableDebugWire;

    using Targets::TargetAddressSpaceDescriptor;
    using Targets::TargetMemorySegmentType;
    using Targets::TargetState;
    using Targets::TargetPhysicalInterface;
    using Targets::TargetMemoryType;
    using Targets::TargetMemoryBuffer;
    using Targets::TargetMemoryAddress;
    using Targets::TargetMemorySize;
    using Targets::TargetRegister;
    using Targets::TargetRegisterDescriptor;
    using Targets::TargetRegisterDescriptors;
    using Targets::TargetRegisterDescriptorId;
    using Targets::TargetRegisterDescriptorIds;
    using Targets::TargetRegisters;

    EdbgAvr8Interface::EdbgAvr8Interface(
        EdbgInterface* edbgInterface,
        const Targets::Microchip::Avr::Avr8Bit::TargetDescriptionFile& targetDescriptionFile,
        const Targets::Microchip::Avr::Avr8Bit::Avr8TargetConfig& targetConfig
    )
        : edbgInterface(edbgInterface)
        , session(EdbgAvr8Session(targetDescriptionFile, targetConfig))
    {}

    void EdbgAvr8Interface::init() {
        if (this->session.configVariant == Avr8ConfigVariant::XMEGA) {
            // Default PDI clock to 4MHz
            // TODO: Make this adjustable via a target config parameter
            this->setParameter(Avr8EdbgParameters::PDI_CLOCK_SPEED, static_cast<std::uint16_t>(4000));
        }

        if (this->session.configVariant == Avr8ConfigVariant::UPDI) {
            // Default UPDI clock to 1.8MHz
            this->setParameter(Avr8EdbgParameters::PDI_CLOCK_SPEED, static_cast<std::uint16_t>(1800));
            this->setParameter(Avr8EdbgParameters::ENABLE_HIGH_VOLTAGE_UPDI, static_cast<std::uint8_t>(0));
        }

        if (this->session.configVariant == Avr8ConfigVariant::MEGAJTAG) {
            // Default clock value for mega debugging is 200KHz
            // TODO: Make this adjustable via a target config parameter
            this->setParameter(Avr8EdbgParameters::MEGA_DEBUG_CLOCK, static_cast<std::uint16_t>(200));
            this->setParameter(Avr8EdbgParameters::JTAG_DAISY_CHAIN_SETTINGS, static_cast<std::uint32_t>(0));
        }

        this->setParameter(
            Avr8EdbgParameters::CONFIG_VARIANT,
            static_cast<std::uint8_t>(this->session.configVariant)
        );

        this->setParameter(
            Avr8EdbgParameters::CONFIG_FUNCTION,
            static_cast<std::uint8_t>(Avr8ConfigFunction::DEBUGGING)
        );

        this->setParameter(
            Avr8EdbgParameters::PHYSICAL_INTERFACE,
            getPhysicalInterfaceToAvr8IdMapping().at(this->session.targetConfig.physicalInterface)
        );

        this->setTargetParameters();
    }

    void EdbgAvr8Interface::stop() {
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            Stop()
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Stop target command failed", responseFrame);
        }

        if (this->getTargetState() == TargetState::RUNNING) {
            this->waitForStoppedEvent();
        }
    }

    void EdbgAvr8Interface::run() {
        this->clearEvents();
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            Run()
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Run command failed", responseFrame);
        }

        this->targetState = TargetState::RUNNING;
    }

    void EdbgAvr8Interface::runTo(TargetMemoryAddress address) {
        this->clearEvents();
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            RunTo(address)
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Run-to command failed", responseFrame);
        }

        this->targetState = TargetState::RUNNING;
    }

    void EdbgAvr8Interface::step() {
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            Step()
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Step target command failed", responseFrame);
        }

        this->targetState = TargetState::RUNNING;
    }

    void EdbgAvr8Interface::reset() {
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            Reset()
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Reset target command failed", responseFrame);
        }

        try {
            // Wait for stopped event
            this->waitForStoppedEvent();

        } catch (const Exception& exception) {
            throw Exception("Failed to reset AVR8 target - missing stopped event.");
        }

        /*
         * Issuing another command immediately after reset sometimes results in an 'illegal target state' error from
         * the EDBG debug tool. Even though we waited for the break event.
         *
         * All we can really do here is introduce a small delay, to ensure that we're not issuing commands too quickly
         * after reset.
         */
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    void EdbgAvr8Interface::activate() {
        if (!this->physicalInterfaceActivated) {
            try {
                this->activatePhysical();

            } catch (const Avr8CommandFailure& activationException) {
                if (
                    this->session.targetConfig.physicalInterface == TargetPhysicalInterface::DEBUG_WIRE
                    && (
                        activationException.code == Avr8CommandFailureCode::DEBUGWIRE_PHYSICAL_ERROR
                        || activationException.code == Avr8CommandFailureCode::FAILED_TO_ENABLE_OCD
                    )
                ) {
                    throw DebugWirePhysicalInterfaceError(
                        "Failed to activate the debugWire physical interface - check target connection. "
                        "If the target was recently programmed via ISP, try cycling the target power. See "
                        + Services::PathService::homeDomainName() + "/docs/debugging-avr-debugwire for more information."
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
                this->session.targetConfig.physicalInterface == TargetPhysicalInterface::DEBUG_WIRE
                && this->session.targetConfig.disableDebugWireOnDeactivate
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

            this->detach();
        }

        if (this->physicalInterfaceActivated) {
            this->deactivatePhysical();
        }
    }

    TargetMemoryAddress EdbgAvr8Interface::getProgramCounter() {
        if (this->targetState != TargetState::STOPPED) {
            this->stop();
        }

        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            GetProgramCounter()
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Get program counter command failed", responseFrame);
        }

        return responseFrame.extractProgramCounter();
    }

    void EdbgAvr8Interface::setProgramCounter(TargetMemoryAddress programCounter) {
        if (this->targetState != TargetState::STOPPED) {
            this->stop();
        }

        /*
         * The program counter will be given in byte address form, but the EDBG tool will be expecting it in word
         * address (16-bit) form. This is why we divide it by 2.
         */
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            SetProgramCounter(programCounter / 2)
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Set program counter command failed", responseFrame);
        }
    }

    TargetSignature EdbgAvr8Interface::getDeviceId() {
        if (this->session.configVariant == Avr8ConfigVariant::UPDI) {
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
            const auto signatureMemory = this->readMemory(
                Avr8MemoryType::SRAM,
                this->session.signatureMemorySegment.startAddress,
                3
            );

            if (signatureMemory.size() != 3) {
                throw Exception("Failed to read AVR8 signature from target - unexpected response size");
            }

            return {signatureMemory[0], signatureMemory[1], signatureMemory[2]};
        }

        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            GetDeviceId()
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Get device ID command failed", responseFrame);
        }

        return responseFrame.extractSignature(this->session.targetConfig.physicalInterface);
    }

    void EdbgAvr8Interface::setSoftwareBreakpoint(TargetMemoryAddress address) {
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            SetSoftwareBreakpoints({address})
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Set software breakpoint command failed", responseFrame);
        }
    }

    void EdbgAvr8Interface::clearSoftwareBreakpoint(TargetMemoryAddress address) {
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            ClearSoftwareBreakpoints({address})
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Clear software breakpoint command failed", responseFrame);
        }
    }

    void EdbgAvr8Interface::setHardwareBreakpoint(TargetMemoryAddress address) {
        static const auto getAvailableBreakpointNumbers = [this] () {
            auto breakpointNumbers = std::set<std::uint8_t>({1, 2, 3});

            for (const auto& [address, allocatedNumber] : this->hardwareBreakpointNumbersByAddress) {
                breakpointNumbers.erase(allocatedNumber);
            }

            return breakpointNumbers;
        };

        const auto availableBreakpointNumbers = getAvailableBreakpointNumbers();

        if (availableBreakpointNumbers.empty()) {
            throw Exception("Maximum hardware breakpoints have been allocated");
        }

        const auto breakpointNumber = *(availableBreakpointNumbers.begin());

        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            SetHardwareBreakpoint(address, breakpointNumber)
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Set hardware breakpoint command failed", responseFrame);
        }

        this->hardwareBreakpointNumbersByAddress.insert(std::pair(address, breakpointNumber));
    }

    void EdbgAvr8Interface::clearHardwareBreakpoint(TargetMemoryAddress address) {
        const auto breakpointNumberIt = this->hardwareBreakpointNumbersByAddress.find(address);

        if (breakpointNumberIt == this->hardwareBreakpointNumbersByAddress.end()) {
            Logger::error("No hardware breakpoint at byte address 0x" + Services::StringService::toHex(address));
            return;
        }

        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            ClearHardwareBreakpoint(breakpointNumberIt->second)
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Clear hardware breakpoint command failed", responseFrame);
        }

        this->hardwareBreakpointNumbersByAddress.erase(address);
    }

    void EdbgAvr8Interface::clearAllBreakpoints() {
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            ClearAllSoftwareBreakpoints()
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Clear all software breakpoints command failed", responseFrame);
        }

        // Clear all hardware breakpoints
        while (!this->hardwareBreakpointNumbersByAddress.empty()) {
            this->clearHardwareBreakpoint(this->hardwareBreakpointNumbersByAddress.begin()->first);
        }
    }

    TargetRegisters EdbgAvr8Interface::readRegisters(const TargetRegisterDescriptorIds& descriptorIds) {
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
        auto descriptorIdsByType = std::map<TargetRegisterType, std::set<TargetRegisterDescriptorId>>();

        /*
         * An address range is just an std::pair of addresses - the first being the start address, the second being
         * the end address.
         *
         * TODO: Can't we just use the TargetMemoryAddressRange struct here? Review
         */
        using AddressRange = std::pair<TargetMemoryAddress, TargetMemoryAddress>;
        auto addressRangeByType = std::map<TargetRegisterType, AddressRange>();

        for (const auto& descriptorId : descriptorIds) {
            const auto descriptorIt = this->targetRegisterDescriptorsById.find(descriptorId);
            assert(descriptorIt != this->targetRegisterDescriptorsById.end());

            const auto& descriptor = descriptorIt->second;

            if (!descriptor.startAddress.has_value()) {
                Logger::debug(
                    "Attempted to read register in the absence of a start address - register name: "
                        + descriptor.name.value_or("unknown")
                );
                continue;
            }

            descriptorIdsByType[descriptor.type].insert(descriptor.id);

            const auto startAddress = descriptor.startAddress.value();
            const auto endAddress = startAddress + (descriptor.size - 1);

            const auto addressRangeIt = addressRangeByType.find(descriptor.type);

            if (addressRangeIt == addressRangeByType.end()) {
                addressRangeByType[descriptor.type] = AddressRange(startAddress, endAddress);

            } else {
                auto& addressRange = addressRangeIt->second;

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
        for (const auto&[registerType, descriptorIds] : descriptorIdsByType) {
            const auto& addressRange = addressRangeByType[registerType];
            const auto startAddress = addressRange.first;
            const auto endAddress = addressRange.second;
            const auto readSize = (endAddress - startAddress) + 1;

            const auto memoryType = (registerType != TargetRegisterType::GENERAL_PURPOSE_REGISTER)
                ? Avr8MemoryType::SRAM
                : (this->session.configVariant == Avr8ConfigVariant::XMEGA || this->session.configVariant == Avr8ConfigVariant::UPDI
                    ? Avr8MemoryType::REGISTER_FILE
                    : Avr8MemoryType::SRAM);

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
            auto excludedAddresses = std::set<TargetMemoryAddress>();
            if (memoryType == Avr8MemoryType::SRAM && this->targetParameters.ocdDataRegister.has_value()) {
                excludedAddresses.insert(
                    this->targetParameters.ocdDataRegister.value()
                        + this->targetParameters.mappedIoSegmentStartAddress.value_or(0)
                );
            }

            const auto flatMemoryData = this->readMemory(
                memoryType,
                startAddress,
                readSize,
                excludedAddresses
            );

            if (flatMemoryData.size() != readSize) {
                throw Exception(
                    "Failed to read memory within register type address range (" + std::to_string(startAddress)
                        + " - " + std::to_string(endAddress) + "). Expected " + std::to_string(readSize)
                        + " bytes, got " + std::to_string(flatMemoryData.size())
                );
            }

            // Construct our TargetRegister objects directly from the flat memory buffer
            for (const auto descriptorId : descriptorIds) {
                const auto descriptorIt = this->targetRegisterDescriptorsById.find(descriptorId);
                const auto& descriptor = descriptorIt->second;

                /*
                 * Multibyte AVR8 registers are stored in LSB form.
                 *
                 * This is why we use reverse iterators when extracting our data from flatMemoryData. Doing so allows
                 * us to extract the data in MSB form (as is expected for all register values held in TargetRegister
                 * objects).
                 */
                const auto bufferStartIt = flatMemoryData.rend() - (descriptor.startAddress.value() - startAddress)
                    - descriptor.size;

                output.emplace_back(
                    TargetRegister(
                        descriptor.id,
                        TargetMemoryBuffer(bufferStartIt, bufferStartIt + descriptor.size)
                    )
                );
            }
        }

        return output;
    }

    void EdbgAvr8Interface::writeRegisters(const Targets::TargetRegisters& registers) {
        for (const auto& reg : registers) {
            const auto& registerDescriptorIt = this->targetRegisterDescriptorsById.find(reg.descriptorId);
            assert(registerDescriptorIt != this->targetRegisterDescriptorsById.end());

            const auto& registerDescriptor = registerDescriptorIt->second;
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
                && (this->session.configVariant == Avr8ConfigVariant::XMEGA || this->session.configVariant == Avr8ConfigVariant::UPDI)
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
        TargetMemoryAddress startAddress,
        TargetMemorySize bytes,
        const std::set<Targets::TargetMemoryAddressRange>& excludedAddressRanges
    ) {
        if (this->programmingModeEnabled && memoryType == TargetMemoryType::RAM) {
            throw Exception("Cannot access RAM when programming mode is enabled");
        }

        auto avr8MemoryType = Avr8MemoryType::SRAM;

        switch (memoryType) {
            case TargetMemoryType::RAM: {
                avr8MemoryType = Avr8MemoryType::SRAM;
                break;
            }
            case TargetMemoryType::FLASH: {
                if (
                    this->configVariant == Avr8ConfigVariant::DEBUG_WIRE
                    || this->configVariant == Avr8ConfigVariant::UPDI
                ) {
                    avr8MemoryType = Avr8MemoryType::FLASH_PAGE;

                } else if (this->configVariant == Avr8ConfigVariant::MEGAJTAG) {
                    avr8MemoryType = this->programmingModeEnabled ? Avr8MemoryType::FLASH_PAGE : Avr8MemoryType::SPM;

                } else if (this->configVariant == Avr8ConfigVariant::XMEGA) {
                    const auto bootSectionStartAddress = this->targetParameters.bootSectionStartAddress.value();
                    if (startAddress >= bootSectionStartAddress) {
                        avr8MemoryType = Avr8MemoryType::BOOT_FLASH;

                        /*
                         * When using the BOOT_FLASH memory type, the address should be relative to the start of the
                         * boot section.
                         */
                        startAddress -= bootSectionStartAddress;

                    } else {
                        /*
                         * When using the APPL_FLASH memory type, the address should be relative to the start of the
                         * application section.
                         */
                        startAddress -= this->targetParameters.appSectionStartAddress.value();
                        avr8MemoryType = Avr8MemoryType::APPL_FLASH;
                    }
                }
                break;
            }
            case TargetMemoryType::EEPROM: {
                // For JTAG targets, we must use the EEPROM_PAGE memory type when in programming mode.
                avr8MemoryType = (this->configVariant == Avr8ConfigVariant::MEGAJTAG && this->programmingModeEnabled)
                    ? Avr8MemoryType::EEPROM_PAGE
                    : Avr8MemoryType::EEPROM;

                if (this->configVariant == Avr8ConfigVariant::XMEGA) {
                    // EEPROM addresses should be in relative form, for XMEGA (PDI) targets
                    startAddress -= this->targetParameters.eepromStartAddress.value();
                }
                break;
            }
            case TargetMemoryType::FUSES: {
                avr8MemoryType = Avr8MemoryType::FUSES;
                break;
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
        auto excludedAddresses = std::set<TargetMemoryAddress>();
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
        TargetMemoryAddress startAddress,
        const TargetMemoryBuffer& buffer
    ) {
        auto avr8MemoryType = Avr8MemoryType::SRAM;

        switch (memoryType) {
            case TargetMemoryType::RAM: {
                avr8MemoryType = Avr8MemoryType::SRAM;
                break;
            }
            case TargetMemoryType::FLASH: {
                if (
                    this->session.configVariant == Avr8ConfigVariant::DEBUG_WIRE
                    || this->session.configVariant == Avr8ConfigVariant::UPDI
                    || this->session.configVariant == Avr8ConfigVariant::MEGAJTAG
                ) {
                    avr8MemoryType = Avr8MemoryType::FLASH_PAGE;

                } else if (this->session.configVariant == Avr8ConfigVariant::XMEGA) {
                    const auto bootSectionStartAddress = this->session.programBootSection.value().get().startAddress;
                    if (startAddress >= bootSectionStartAddress) {
                        avr8MemoryType = Avr8MemoryType::BOOT_FLASH;

                        /*
                         * When using the BOOT_FLASH memory type, the address should be relative to the start of the
                         * boot section.
                         */
                        startAddress -= bootSectionStartAddress;

                    } else {
                        /*
                         * When using the APPL_FLASH memory type, the address should be relative to the start of the
                         * application section.
                         */
                        startAddress -= this->session.programAppSection.value().get().startAddress;
                        avr8MemoryType = Avr8MemoryType::APPL_FLASH;
                    }
                }
                break;
            }
            case TargetMemoryType::EEPROM: {
                switch (this->session.configVariant) {
                    case Avr8ConfigVariant::UPDI:
                    case Avr8ConfigVariant::XMEGA: {
                        avr8MemoryType = Avr8MemoryType::EEPROM_ATOMIC;

                        if (this->session.configVariant == Avr8ConfigVariant::XMEGA) {
                            // EEPROM addresses should be in relative form, for XMEGA (PDI) targets
                            startAddress -= this->session.eepromMemorySegment.startAddress;
                        }

                        break;
                    }
                    case Avr8ConfigVariant::MEGAJTAG: {
                        avr8MemoryType = this->programmingModeEnabled
                            ? Avr8MemoryType::EEPROM_PAGE
                            : Avr8MemoryType::EEPROM;
                        break;
                    }
                    default: {
                        avr8MemoryType = Avr8MemoryType::EEPROM;
                        break;
                    }
                }
                break;
            }
            case TargetMemoryType::FUSES: {
                avr8MemoryType = Avr8MemoryType::FUSES;
                break;
            }
            default: {
                break;
            }
        }

        return this->writeMemory(avr8MemoryType, startAddress, buffer);
    }

    void EdbgAvr8Interface::eraseProgramMemory(std::optional<Avr8Bit::ProgramMemorySection> section) {
        if (this->session.configVariant == Avr8ConfigVariant::DEBUG_WIRE) {
            // The EDBG erase command does not work on debugWire targets - we'll just write to the memory instead
            return this->writeMemory(
                TargetMemoryType::FLASH,
                this->session.programMemorySegment.startAddress,
                TargetMemoryBuffer(this->session.programMemorySegment.size, 0xFF)
            );
        }

        if (this->session.configVariant == Avr8ConfigVariant::XMEGA) {
            // For PDI (XMEGA) targets, we can erase flash memory without erasing EEPROM

            if (!section.has_value() || *section == Avr8Bit::ProgramMemorySection::BOOT) {
                const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
                    EraseMemory(Avr8EraseMemoryMode::BOOT_SECTION)
                );

                if (responseFrame.id == Avr8ResponseId::FAILED) {
                    throw Avr8CommandFailure("AVR8 erase memory command (for BOOT section) failed", responseFrame);
                }
            }

            if (!section.has_value() || *section == Avr8Bit::ProgramMemorySection::APPLICATION) {
                const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
                    EraseMemory(Avr8EraseMemoryMode::APPLICATION_SECTION)
                );

                if (responseFrame.id == Avr8ResponseId::FAILED) {
                    throw Avr8CommandFailure(
                        "AVR8 erase memory command (for APPLICATION section) failed",
                        responseFrame
                    );
                }
            }

            return;
        }

        throw Exception("JTAG and UPDI targets do not support program memory erase.");
    }

    void EdbgAvr8Interface::eraseChip() {
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            EraseMemory(Avr8EraseMemoryMode::CHIP)
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 erase memory command failed", responseFrame);
        }
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

    void EdbgAvr8Interface::enableProgrammingMode() {
        if (this->programmingModeEnabled) {
            return;
        }

        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            EnterProgrammingMode()
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("Failed to enter programming mode on EDBG debug tool", responseFrame);
        }

        this->programmingModeEnabled = true;
        this->hardwareBreakpointNumbersByAddress.clear();
    }

    void EdbgAvr8Interface::disableProgrammingMode() {
        if (!this->programmingModeEnabled) {
            return;
        }

        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            LeaveProgrammingMode()
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("Failed to leave programming mode on EDBG debug tool", responseFrame);
        }

        this->programmingModeEnabled = false;

        if (this->session.configVariant == Avr8ConfigVariant::MEGAJTAG && this->reactivateJtagTargetPostProgrammingMode) {
            this->deactivatePhysical();
            this->targetAttached = false;
            this->activate();
        }
    }

    void EdbgAvr8Interface::setTargetParameters() {
        switch (this->session.configVariant) {
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

    void EdbgAvr8Interface::setParameter(const Avr8EdbgParameter& parameter, const std::vector<unsigned char>& value) {
        using Services::StringService;

        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            SetParameter(parameter, value)
        );

        Logger::debug(
            "Setting AVR8 EDBG parameter (context: 0x" + StringService::toHex(parameter.context) + ", id: 0x"
                + StringService::toHex(parameter.id) + ", value: 0x" + StringService::toHex(value) + ")"
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("Failed to set parameter on device!", responseFrame);
        }
    }

    std::vector<unsigned char> EdbgAvr8Interface::getParameter(const Avr8EdbgParameter& parameter, std::uint8_t size) {
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            GetParameter(parameter, size)
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("Failed to get parameter from device!", responseFrame);
        }

        return responseFrame.getPayloadData();
    }

    void EdbgAvr8Interface::setDebugWireAndJtagParameters() {
        const auto parameters = Parameters::Avr8Generic::DebugWireJtagParameters(this->session.targetDescriptionFile);

        Logger::debug("Setting FLASH_PAGE_SIZE AVR8 device parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_FLASH_PAGE_SIZE, parameters.flashPageSize);

        Logger::debug("Setting FLASH_SIZE AVR8 device parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_FLASH_SIZE, parameters.flashSize);

        Logger::debug("Setting FLASH_BASE AVR8 device parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_FLASH_BASE, parameters.flashStartWordAddress);

        if (parameters.bootSectionStartWordAddress.has_value()) {
            Logger::debug("Setting BOOT_START_ADDR AVR8 device parameter");
            this->setParameter(Avr8EdbgParameters::DEVICE_BOOT_START_ADDR, *(parameters.bootSectionStartWordAddress));
        }

        Logger::debug("Setting SRAM_START AVR8 device parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_SRAM_START, parameters.ramStartAddress);

        Logger::debug("Setting EEPROM_SIZE AVR8 device parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_EEPROM_SIZE, parameters.eepromSize);

        Logger::debug("Setting EEPROM_PAGE_SIZE AVR8 device parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_EEPROM_PAGE_SIZE, parameters.eepromPageSize);

        Logger::debug("Setting OCD_REVISION AVR8 device parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_OCD_REVISION, parameters.ocdRevision);

        Logger::debug("Setting OCD_DATA_REGISTER AVR8 device parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_OCD_DATA_REGISTER, parameters.ocdDataRegisterAddress);

        Logger::debug("Setting EEARL_ADDR AVR8 device parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_EEARL_ADDR, parameters.eearAddressLow);

        Logger::debug("Setting EEARH_ADDR AVR8 device parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_EEARH_ADDR, parameters.eearAddressHigh);

        Logger::debug("Setting EECR_ADDR AVR8 device parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_EECR_ADDR, parameters.eecrAddress);

        Logger::debug("Setting EEDR_ADDR AVR8 device parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_EEDR_ADDR, parameters.eedrAddress);

        Logger::debug("Setting SPMCR_REGISTER AVR8 device parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_SPMCR_REGISTER, parameters.spmcrAddress);

        Logger::debug("Setting OSCCAL_ADDR AVR8 device parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_OSCCAL_ADDR, parameters.osccalAddress);
    }

    void EdbgAvr8Interface::setPdiParameters() {
        const auto parameters = Parameters::Avr8Generic::PdiParameters(this->session.targetDescriptionFile);

        Logger::debug("Setting APPL_BASE_ADDR AVR8 parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_XMEGA_APPL_BASE_ADDR, parameters.appSectionPdiOffset);

        Logger::debug("Setting BOOT_BASE_ADDR AVR8 parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_XMEGA_BOOT_BASE_ADDR, parameters.bootSectionPdiOffset);

        Logger::debug("Setting EEPROM_BASE_ADDR AVR8 parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_XMEGA_EEPROM_BASE_ADDR, parameters.eepromPdiOffset);

        Logger::debug("Setting FUSE_BASE_ADDR AVR8 parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_XMEGA_FUSE_BASE_ADDR, parameters.fuseRegistersPdiOffset);

        Logger::debug("Setting LOCKBIT_BASE_ADDR AVR8 parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_XMEGA_LOCKBIT_BASE_ADDR, parameters.lockRegistersPdiOffset);

        Logger::debug("Setting USER_SIGN_BASE_ADDR AVR8 parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_XMEGA_USER_SIGN_BASE_ADDR, parameters.userSignaturesPdiOffset);

        Logger::debug("Setting PROD_SIGN_BASE_ADDR AVR8 parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_XMEGA_PROD_SIGN_BASE_ADDR, parameters.prodSignaturesPdiOffset);

        Logger::debug("Setting DATA_BASE_ADDR AVR8 parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_XMEGA_DATA_BASE_ADDR, parameters.ramPdiOffset);

        Logger::debug("Setting APPLICATION_BYTES AVR8 parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_XMEGA_APPLICATION_BYTES, parameters.appSectionSize);

        Logger::debug("Setting BOOT_BYTES AVR8 parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_XMEGA_BOOT_BYTES, parameters.bootSectionSize);

        Logger::debug("Setting FLASH_PAGE_BYTES AVR8 parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_XMEGA_FLASH_PAGE_BYTES, parameters.flashPageSize);

        Logger::debug("Setting EEPROM_SIZE AVR8 parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_XMEGA_EEPROM_SIZE, parameters.eepromSize);

        Logger::debug("Setting EEPROM_PAGE_SIZE AVR8 parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_XMEGA_EEPROM_PAGE_SIZE, parameters.eepromPageSize);

        Logger::debug("Setting NVM_BASE AVR8 parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_XMEGA_NVM_BASE, parameters.nvmModuleBaseAddress);

        Logger::debug("Setting SIGNATURE_OFFSET AVR8 parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_XMEGA_SIGNATURE_OFFSET, parameters.signaturesPdiOffset);
    }

    void EdbgAvr8Interface::setUpdiParameters() {
        const auto parameters = Parameters::Avr8Generic::UpdiParameters(this->session.targetDescriptionFile);

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
        Logger::debug("Setting UPDI_PROGMEM_BASE_ADDR AVR8 device parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_UPDI_PROGMEM_BASE_ADDR,
            static_cast<std::uint16_t>(parameters.programMemoryStartAddress)
        );

        Logger::debug("Setting UPDI_PROGMEM_BASE_ADDR_MSB AVR8 device parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_UPDI_PROGMEM_BASE_ADDR_MSB,
            static_cast<std::uint8_t>(parameters.programMemoryStartAddress >> 16)
        );

        this->setParameter(
            Avr8EdbgParameters::DEVICE_UPDI_24_BIT_ADDRESSING_ENABLE,
            parameters.programMemoryStartAddress > 0xFFFF
                ? static_cast<std::uint8_t>(1)
                : static_cast<std::uint8_t>(0)
        );

        /*
         * See the comment above regarding capacity limitations of the DEVICE_UPDI_PROGMEM_BASE_ADDR field.
         *
         * The same applies here, for the flash page size field (DEVICE_UPDI_FLASH_PAGE_SIZE).
         */
        Logger::debug("Setting UPDI_FLASH_PAGE_SIZE AVR8 device parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_UPDI_FLASH_PAGE_SIZE,
            static_cast<std::uint8_t>(parameters.flashPageSize)
        );

        Logger::debug("Setting UPDI_FLASH_PAGE_SIZE_MSB AVR8 device parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_UPDI_FLASH_PAGE_SIZE_MSB,
            static_cast<std::uint8_t>(parameters.flashPageSize >> 8)
        );

        Logger::debug("Setting UPDI_EEPROM_PAGE_SIZE AVR8 device parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_UPDI_EEPROM_PAGE_SIZE, parameters.eepromPageSize);

        Logger::debug("Setting UPDI_NVMCTRL_ADDR AVR8 device parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_UPDI_NVMCTRL_ADDR, parameters.nvmModuleBaseAddress);

        Logger::debug("Setting UPDI_OCD_ADDR AVR8 device parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_UPDI_OCD_ADDR, parameters.ocdModuleAddress);

        Logger::debug("Setting UPDI_FLASH_SIZE AVR8 device parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_UPDI_FLASH_SIZE, parameters.flashSize);

        Logger::debug("Setting UPDI_EEPROM_SIZE AVR8 device parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_UPDI_EEPROM_SIZE, parameters.eepromSize);

        Logger::debug("Setting UPDI_EEPROM_BASE_ADDR AVR8 device parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_UPDI_EEPROM_BASE_ADDR, parameters.eepromStartAddress);

        Logger::debug("Setting UPDI_SIG_BASE_ADDR AVR8 device parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_UPDI_SIG_BASE_ADDR, parameters.signatureSegmentStartAddress);

        Logger::debug("Setting UPDI_FUSE_BASE_ADDR AVR8 device parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_UPDI_FUSE_BASE_ADDR, parameters.fuseSegmentStartAddress);

        Logger::debug("Setting UPDI_FUSE_SIZE AVR8 device parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_UPDI_FUSE_SIZE, parameters.fuseSegmentSize);

        Logger::debug("Setting UPDI_LOCK_BASE_ADDR AVR8 device parameter");
        this->setParameter(Avr8EdbgParameters::DEVICE_UPDI_LOCK_BASE_ADDR, parameters.lockbitSegmentStartAddress);
    }

    void EdbgAvr8Interface::activatePhysical(bool applyExternalReset) {
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            ActivatePhysical(applyExternalReset)
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            if (!applyExternalReset) {
                // Try again with external reset applied
                Logger::debug("Failed to activate physical interface on AVR8 target "
                    "- retrying with external reset applied.");
                return this->activatePhysical(true);
            }

            throw Avr8CommandFailure("AVR8 Activate physical interface command failed", responseFrame);
        }

        this->physicalInterfaceActivated = true;
    }

    void EdbgAvr8Interface::deactivatePhysical() {
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            DeactivatePhysical()
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Deactivate physical interface command failed", responseFrame);
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
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            Attach(
                this->session.configVariant != Avr8ConfigVariant::MEGAJTAG
            )
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Attach command failed", responseFrame);
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
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            Detach()
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Detach command failed", responseFrame);
        }

        this->targetAttached = false;
    }

    std::unique_ptr<AvrEvent> EdbgAvr8Interface::getAvrEvent() {
        auto event = this->edbgInterface->requestAvrEvent();

        if (!event.has_value()) {
            return nullptr;
        }

        switch (event->eventId.value()) {
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

    bool EdbgAvr8Interface::alignmentRequired(Avr8MemoryType memoryType) {
        return
            memoryType == Avr8MemoryType::FLASH_PAGE
            || memoryType == Avr8MemoryType::SPM
            || memoryType == Avr8MemoryType::APPL_FLASH
            || memoryType == Avr8MemoryType::BOOT_FLASH
            || memoryType == Avr8MemoryType::EEPROM_ATOMIC
            || memoryType == Avr8MemoryType::EEPROM_PAGE
        ;
    }

    TargetMemoryAddress EdbgAvr8Interface::alignMemoryAddress(
        Avr8MemoryType memoryType,
        TargetMemoryAddress address
    ) {
        std::uint16_t alignTo = 1;

        switch (memoryType) {
            case Avr8MemoryType::FLASH_PAGE:
            case Avr8MemoryType::SPM:
            case Avr8MemoryType::APPL_FLASH:
            case Avr8MemoryType::BOOT_FLASH: {
                /*
                 * Although the EDBG documentation claims any number of bytes can be accessed via the FLASH_PAGE mem
                 * type, when using the UPDI config variant, this isn't strictly true.
                 *
                 * When writing to flash on UPDI targets, we MUST page align the write operations. And we cannot word
                 * align them - we've tried only word aligning them - the debug tool reports a "Too many or too few
                 * bytes" error.
                 */
                alignTo = static_cast<std::uint16_t>(this->session.programMemorySegment.pageSize.value());
                break;
            }
            case Avr8MemoryType::EEPROM_ATOMIC:
            case Avr8MemoryType::EEPROM_PAGE: {
                alignTo = static_cast<std::uint16_t>(this->session.eepromMemorySegment.pageSize.value());
                break;
            }
            default: {
                break;
            }
        }

        if ((address % alignTo) != 0) {
            return static_cast<TargetMemoryAddress>(std::floor(
                static_cast<float>(address) / static_cast<float>(alignTo)
            ) * alignTo);
        }

        return address;
    }

    TargetMemorySize EdbgAvr8Interface::alignMemoryBytes(
        Avr8MemoryType memoryType,
        TargetMemorySize bytes
    ) {
        std::uint16_t alignTo = 1;

        switch (memoryType) {
            case Avr8MemoryType::FLASH_PAGE:
            case Avr8MemoryType::SPM:
            case Avr8MemoryType::APPL_FLASH:
            case Avr8MemoryType::BOOT_FLASH: {
                // See comment in EdbgAvr8Interface::alignMemoryAddress()
                alignTo = static_cast<std::uint16_t>(this->session.programMemorySegment.pageSize.value());
                break;
            }
            case Avr8MemoryType::EEPROM_ATOMIC:
            case Avr8MemoryType::EEPROM_PAGE: {
                alignTo = static_cast<std::uint16_t>(this->session.eepromMemorySegment.pageSize.value());
                break;
            }
            default: {
                break;
            }
        }

        if ((bytes % alignTo) != 0) {
            return static_cast<TargetMemorySize>(std::ceil(
                static_cast<float>(bytes) / static_cast<float>(alignTo)
            ) * alignTo);
        }

        return bytes;
    }

    std::optional<Targets::TargetMemorySize> EdbgAvr8Interface::maximumMemoryAccessSize(Avr8MemoryType memoryType) {
        if (
            memoryType == Avr8MemoryType::FLASH_PAGE
            || memoryType == Avr8MemoryType::APPL_FLASH
            || memoryType == Avr8MemoryType::BOOT_FLASH
            || (memoryType == Avr8MemoryType::SPM && this->session.configVariant == Avr8ConfigVariant::MEGAJTAG)
        ) {
            // These flash memory types require single page access.
            return this->session.programMemorySegment.pageSize.value();
        }

        if (
            memoryType == Avr8MemoryType::EEPROM_ATOMIC
            || memoryType == Avr8MemoryType::EEPROM_PAGE
        ) {
            // These EEPROM memory types requires single page access.
            return this->session.eepromMemorySegment.pageSize.value();
        }

        if (this->maximumMemoryAccessSizePerRequest.has_value()) {
            // There is a memory access size limit for this entire EdbgAvr8Interface instance
            return this->maximumMemoryAccessSizePerRequest;
        }

        /*
         * EDBG AVR8 debug tools behave in a really weird way when receiving or responding with more than two packets
         * for a single memory access command. The data they read/write in this case appears to be wrong.
         *
         * To address this, we make sure we only issue memory access commands that will result in no more than two
         * packets being sent to and from the debug tool.
         *
         * The -30 is to accommodate for the bytes in the command that are not part of the main payload of the command.
         */
        return static_cast<Targets::TargetMemorySize>(
            (this->edbgInterface->getUsbHidInputReportSize() - 30) * 2
        );
    }

    TargetMemoryBuffer EdbgAvr8Interface::readMemory(
        Avr8MemoryType type,
        TargetMemoryAddress startAddress,
        TargetMemorySize bytes,
        const std::set<TargetMemoryAddress>& excludedAddresses
    ) {
        if (type == Avr8MemoryType::FUSES) {
            if (this->session.configVariant == Avr8ConfigVariant::DEBUG_WIRE) {
                throw Exception("Cannot access AVR fuses via the debugWire interface");
            }
        }

        const auto managingProgrammingMode = type == Avr8MemoryType::FUSES && !this->programmingModeEnabled;
        if (managingProgrammingMode) {
            this->enableProgrammingMode();
        }

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

                    std::move(segmentBuffer.begin(), segmentBuffer.end(), std::back_inserter(output));
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

                std::move(segmentBuffer.begin(), segmentBuffer.end(), std::back_inserter(output));
            }

            return output;
        }

        if (this->alignmentRequired(type)) {
            const auto alignedStartAddress = this->alignMemoryAddress(type, startAddress);
            const auto alignedBytes = this->alignMemoryBytes(type, bytes + (startAddress - alignedStartAddress));

            if (alignedStartAddress != startAddress || alignedBytes != bytes) {
                auto memoryBuffer = this->readMemory(type, alignedStartAddress, alignedBytes, excludedAddresses);

                const auto offset = memoryBuffer.begin() + (startAddress - alignedStartAddress);
                auto output = TargetMemoryBuffer();
                output.reserve(bytes);
                std::move(offset, offset + bytes, std::back_inserter(output));

                return output;
            }
        }

        const auto maximumReadSize = this->maximumMemoryAccessSize(type);
        if (maximumReadSize.has_value() && bytes > *maximumReadSize) {
            auto output = Targets::TargetMemoryBuffer();
            output.reserve(bytes);

            while (output.size() < bytes) {
                const auto bytesToRead = std::min(
                    static_cast<TargetMemorySize>(bytes - output.size()),
                    *maximumReadSize
                );

                auto data = this->readMemory(
                    type,
                    static_cast<TargetMemoryAddress>(startAddress + output.size()),
                    bytesToRead,
                    excludedAddresses
                );
                std::move(data.begin(), data.end(), std::back_inserter(output));
            }

            return output;
        }

        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            ReadMemory(
                type,
                startAddress,
                bytes,
                excludedAddresses
            )
        );

        if (managingProgrammingMode) {
            this->disableProgrammingMode();
        }

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Read memory command failed", responseFrame);
        }

        const auto data = responseFrame.getMemoryData();

        if (data.size() != bytes) {
            throw Avr8CommandFailure("Unexpected number of bytes returned from EDBG debug tool");
        }

        return data;
    }

    void EdbgAvr8Interface::writeMemory(
        Avr8MemoryType type,
        TargetMemoryAddress startAddress,
        const TargetMemoryBuffer& buffer
    ) {
        if (type == Avr8MemoryType::FUSES) {
            if (this->session.configVariant == Avr8ConfigVariant::DEBUG_WIRE) {
                throw Exception("Cannot access AVR fuses via the debugWire interface");
            }
        }

        const auto managingProgrammingMode = type == Avr8MemoryType::FUSES && !this->programmingModeEnabled;
        if (managingProgrammingMode) {
            this->enableProgrammingMode();
        }

        const auto bytes = static_cast<TargetMemorySize>(buffer.size());

        if (this->alignmentRequired(type)) {
            const auto alignedStartAddress = this->alignMemoryAddress(type, startAddress);
            const auto alignedBytes = this->alignMemoryBytes(type, bytes + (startAddress - alignedStartAddress));

            if (alignedStartAddress != startAddress || alignedBytes != bytes) {
                /*
                 * We can't just forward the memory type to readMemory(), because some memory types (such as
                 * EEPROM_ATOMIC) can only be used for writing.
                 *
                 * This nasty hack will have to do for now.
                 */
                const auto readMemoryType = type == Avr8MemoryType::EEPROM_ATOMIC ? Avr8MemoryType::EEPROM : type;

                auto alignedBuffer = this->readMemory(readMemoryType, alignedStartAddress, alignedBytes);
                assert(alignedBuffer.size() >= buffer.size());

                const auto offset = alignedBuffer.begin() + (startAddress - alignedStartAddress);
                std::copy(buffer.begin(), buffer.end(), offset);

                return this->writeMemory(type, alignedStartAddress, alignedBuffer);
            }
        }

        const auto maximumWriteSize = this->maximumMemoryAccessSize(type);
        if (maximumWriteSize.has_value() && buffer.size() > *maximumWriteSize) {
            auto bytesWritten = TargetMemorySize(0);

            while (bytesWritten < buffer.size()) {
                const auto chunkSize = std::min(
                    static_cast<TargetMemorySize>(buffer.size() - bytesWritten),
                    *maximumWriteSize
                );

                this->writeMemory(
                    type,
                    startAddress + bytesWritten,
                    TargetMemoryBuffer(
                        buffer.begin() + bytesWritten,
                        buffer.begin() + bytesWritten + chunkSize
                    )
                );

                bytesWritten += chunkSize;
            }

            return;
        }

        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            WriteMemory(
                type,
                startAddress,
                buffer
            )
        );


        // We must disable and re-enable programming mode, in order for the changes to the fuse bit to take effect.
        if (type == Avr8MemoryType::FUSES) {
            this->disableProgrammingMode();

            if (!managingProgrammingMode) {
                this->enableProgrammingMode();
            }
        }

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Write memory command failed", responseFrame);
        }
    }

    void EdbgAvr8Interface::refreshTargetState() {
        const auto avrEvent = this->getAvrEvent();

        if (avrEvent != nullptr && avrEvent->eventId == AvrEventId::AVR8_BREAK_EVENT) {
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
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            DisableDebugWire()
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure("AVR8 Disable debugWire command failed", responseFrame);
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
