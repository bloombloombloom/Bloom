#include "EdbgAvr8Interface.hpp"

#include <thread>
#include <cassert>
#include <cmath>
#include <algorithm>

#include "src/Services/PathService.hpp"
#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"

#include "Exceptions/Avr8CommandFailure.hpp"
#include "src/TargetController/Exceptions/DeviceInitializationFailure.hpp"
#include "src/Targets/Microchip/AVR8/Exceptions/DebugWirePhysicalInterfaceError.hpp"

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
    using namespace Targets::Microchip::Avr8;
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
    using Targets::TargetExecutionState;
    using Targets::TargetPhysicalInterface;
    using Targets::TargetMemoryBuffer;
    using Targets::TargetMemoryAddress;
    using Targets::TargetMemorySize;
    using Targets::TargetRegisterDescriptor;
    using Targets::TargetRegisterDescriptors;
    using Targets::TargetRegisterDescriptorAndValuePairs;

    EdbgAvr8Interface::EdbgAvr8Interface(
        EdbgInterface* edbgInterface,
        const Targets::Microchip::Avr8::TargetDescriptionFile& targetDescriptionFile,
        const Targets::Microchip::Avr8::Avr8TargetConfig& targetConfig
    )
        : edbgInterface(edbgInterface)
        , session(EdbgAvr8Session(targetDescriptionFile, targetConfig))
    {}

    void EdbgAvr8Interface::init() {
        if (this->session.configVariant == Avr8ConfigVariant::XMEGA) {
            // Default PDI clock to 4MHz
            // TODO: Make this adjustable via a target config parameter
            this->setParameter(Avr8EdbgParameters::PDI_CLOCK_SPEED, std::uint16_t{4000});
        }

        if (this->session.configVariant == Avr8ConfigVariant::UPDI) {
            // Default UPDI clock to 1.8MHz
            this->setParameter(Avr8EdbgParameters::PDI_CLOCK_SPEED, std::uint16_t{1800});
            this->setParameter(Avr8EdbgParameters::ENABLE_HIGH_VOLTAGE_UPDI, std::uint8_t{0});
        }

        if (this->session.configVariant == Avr8ConfigVariant::MEGAJTAG) {
            // Default clock value for mega debugging is 200KHz
            // TODO: Make this adjustable via a target config parameter
            this->setParameter(Avr8EdbgParameters::MEGA_DEBUG_CLOCK, std::uint16_t{200});
            this->setParameter(Avr8EdbgParameters::JTAG_DAISY_CHAIN_SETTINGS, std::uint32_t{0});
        }

        this->setParameter(
            Avr8EdbgParameters::CONFIG_VARIANT,
            static_cast<std::uint8_t>(this->session.configVariant)
        );

        this->setParameter(
            Avr8EdbgParameters::CONFIG_FUNCTION,
            static_cast<std::uint8_t>(Avr8ConfigFunction::DEBUGGING)
        );

        static const auto avr8PhysicalInterfaceMapping = std::map<TargetPhysicalInterface, Avr8PhysicalInterface>{
            {TargetPhysicalInterface::DEBUG_WIRE, Avr8PhysicalInterface::DEBUG_WIRE},
            {TargetPhysicalInterface::PDI, Avr8PhysicalInterface::PDI},
            {TargetPhysicalInterface::JTAG, Avr8PhysicalInterface::JTAG},
            {TargetPhysicalInterface::UPDI, Avr8PhysicalInterface::PDI_1W},
        };

        this->setParameter(
            Avr8EdbgParameters::PHYSICAL_INTERFACE,
            static_cast<std::uint8_t>(avr8PhysicalInterfaceMapping.at(this->session.targetConfig.physicalInterface))
        );

        this->setTargetParameters();
    }

    void EdbgAvr8Interface::stop() {
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(Stop{});
        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure{"AVR8 Stop target command failed", responseFrame};
        }

        if (this->cachedExecutionState != TargetExecutionState::STOPPED) {
            this->waitForStoppedEvent();
        }
    }

    void EdbgAvr8Interface::run() {
        this->clearEvents();

        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(Run{});
        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure{"AVR8 Run command failed", responseFrame};
        }

        this->cachedExecutionState = TargetExecutionState::RUNNING;
    }

    void EdbgAvr8Interface::runTo(TargetMemoryAddress address) {
        this->clearEvents();

        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(RunTo{address});
        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure{"AVR8 Run-to command failed", responseFrame};
        }

        this->cachedExecutionState = TargetExecutionState::RUNNING;
    }

    void EdbgAvr8Interface::step() {
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(Step{});
        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure{"AVR8 Step target command failed", responseFrame};
        }

        this->cachedExecutionState = TargetExecutionState::STEPPING;
    }

    void EdbgAvr8Interface::reset() {
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(Reset{});
        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure{"AVR8 Reset target command failed", responseFrame};
        }

        try {
            // Wait for stopped event
            this->waitForStoppedEvent();

        } catch (const Exception& exception) {
            throw Exception{"Failed to reset AVR8 target - missing stopped event."};
        }

        /*
         * Issuing another command immediately after reset sometimes results in an 'illegal target state' error from
         * the EDBG debug tool. Even though we waited for the break event.
         *
         * All we can really do here is introduce a small delay, to ensure that we're not issuing commands too quickly
         * after reset.
         */
        std::this_thread::sleep_for(std::chrono::milliseconds{250});
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
                    throw DebugWirePhysicalInterfaceError{
                        "Failed to activate the debugWIRE physical interface - check target connection. "
                            "If the target was recently programmed via ISP, try cycling the target power. See "
                            + Services::PathService::homeDomainName() + "/docs/debugging-avr-debugwire for more "
                            "information."
                    };
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
                        "Successfully disabled debugWIRE on the AVR8 target - this is only temporary - "
                            "the debugWIRE module has lost control of the RESET pin. Bloom may no longer be able to "
                            "interface with the target until the next power cycle."
                    );

                } catch (const Exception& exception) {
                    // Failing to disable debugWIRE should never prevent us from proceeding with target deactivation.
                    Logger::error(exception.getMessage());
                }
            }

            this->detach();
        }

        if (this->physicalInterfaceActivated) {
            this->deactivatePhysical();
        }
    }

    Targets::TargetRegisterAccess EdbgAvr8Interface::getRegisterAccess(
        const TargetRegisterDescriptor& registerDescriptor,
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor
    ) {
        /*
         * Currently, this implementation of the EDBG AVR8 debug interface can only access registers in the data and
         * register file address space (during a debug session).
         *
         * There are some other memory types we can use to access some other registers during a debug session, but
         * these are yet to be implemented.
         */
        const auto access = addressSpaceDescriptor.key == this->session.dataAddressSpace.key
            || addressSpaceDescriptor.key == this->session.registerFileAddressSpace.key;

        return {access, access};
    }

    TargetMemoryAddress EdbgAvr8Interface::getProgramCounter() {
        if (this->cachedExecutionState != TargetExecutionState::STOPPED) {
            this->stop();
        }

        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(GetProgramCounter{});
        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure{"AVR8 Get program counter command failed", responseFrame};
        }

        return responseFrame.extractProgramCounter();
    }

    void EdbgAvr8Interface::setProgramCounter(TargetMemoryAddress programCounter) {
        if (this->cachedExecutionState != TargetExecutionState::STOPPED) {
            /*
             * TODO: Review - do we need to do this? Why? The TC shouldn't attempt to set the program counter if the
             *       target is running. Add a comment
             */
            this->stop();
        }

        /*
         * The program counter will be given in byte address form, but the EDBG tool will be expecting it in word
         * address (16-bit) form. This is why we divide it by 2.
         */
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            SetProgramCounter{programCounter / 2}
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure{"AVR8 Set program counter command failed", responseFrame};
        }
    }

    TargetSignature EdbgAvr8Interface::getDeviceId() {
        if (this->session.configVariant == Avr8ConfigVariant::UPDI) {
            /*
             * In UPDI sessions, the 'Get ID' command behaves in an odd manner, where it doesn't actually return the
             * target signature, but a fixed four byte string reading: 'A', 'V', 'R' and ' ' (white space).
             *
             * So it appears we cannot use that command for UPDI sessions. For this reason, we will just read the
             * signature from memory via the signature memory segment.
             *
             * TODO: We're assuming the signature memory segment will always reside in the `data` address space, for
             *       UPDI targets. Review.
             */
            const auto signatureMemory = this->readMemory(
                Avr8MemoryType::SRAM,
                this->session.signatureMemorySegment.startAddress,
                3
            );

            if (signatureMemory.size() != 3) {
                throw Exception{"Failed to read AVR8 signature from target - unexpected response size"};
            }

            return {signatureMemory[0], signatureMemory[1], signatureMemory[2]};
        }

        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(GetDeviceId{});
        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure{"AVR8 Get device ID command failed", responseFrame};
        }

        return responseFrame.extractSignature(this->session.targetConfig.physicalInterface);
    }

    void EdbgAvr8Interface::setSoftwareBreakpoint(TargetMemoryAddress address) {
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            SetSoftwareBreakpoints{{address}}
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure{"AVR8 Set software breakpoint command failed", responseFrame};
        }
    }

    void EdbgAvr8Interface::clearSoftwareBreakpoint(TargetMemoryAddress address) {
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            ClearSoftwareBreakpoints{{address}}
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure{"AVR8 Clear software breakpoint command failed", responseFrame};
        }
    }

    void EdbgAvr8Interface::setHardwareBreakpoint(TargetMemoryAddress address) {
        static const auto getAvailableBreakpointNumbers = [this] () {
            auto breakpointNumbers = std::set<std::uint8_t>{1, 2, 3};

            for (const auto& [address, allocatedNumber] : this->hardwareBreakpointNumbersByAddress) {
                breakpointNumbers.erase(allocatedNumber);
            }

            return breakpointNumbers;
        };

        if (this->hardwareBreakpointNumbersByAddress.contains(address)) {
            Logger::debug(
                "Hardware breakpoint already installed for byte address 0x" + Services::StringService::toHex(address)
                    + " - ignoring request"
            );
            return;
        }

        const auto availableBreakpointNumbers = getAvailableBreakpointNumbers();

        if (availableBreakpointNumbers.empty()) {
            throw Exception{"Maximum hardware breakpoints have been allocated"};
        }

        const auto breakpointNumber = *(availableBreakpointNumbers.begin());

        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            SetHardwareBreakpoint{address, breakpointNumber}
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure{"AVR8 Set hardware breakpoint command failed", responseFrame};
        }

        this->hardwareBreakpointNumbersByAddress.emplace(address, breakpointNumber);
    }

    void EdbgAvr8Interface::clearHardwareBreakpoint(TargetMemoryAddress address) {
        const auto breakpointNumberIt = this->hardwareBreakpointNumbersByAddress.find(address);
        if (breakpointNumberIt == this->hardwareBreakpointNumbersByAddress.end()) {
            Logger::error("No hardware breakpoint at byte address 0x" + Services::StringService::toHex(address));
            return;
        }

        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            ClearHardwareBreakpoint{breakpointNumberIt->second}
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure{"AVR8 Clear hardware breakpoint command failed", responseFrame};
        }

        this->hardwareBreakpointNumbersByAddress.erase(address);
    }

    void EdbgAvr8Interface::clearAllBreakpoints() {
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            ClearAllSoftwareBreakpoints{}
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure{"AVR8 Clear all software breakpoints command failed", responseFrame};
        }

        // Clear all hardware breakpoints
        while (!this->hardwareBreakpointNumbersByAddress.empty()) {
            this->clearHardwareBreakpoint(this->hardwareBreakpointNumbersByAddress.begin()->first);
        }
    }

    TargetRegisterDescriptorAndValuePairs EdbgAvr8Interface::readRegisters(
        const Targets::TargetRegisterDescriptors& descriptors
    ) {
        using Targets::TargetRegisterType;
        using Targets::TargetMemoryAddressRange;

        /*
         * This function needs to be fast. Insight eagerly requests the values of all known registers that it can
         * present to the user. It does this on numerous occasions (target stopped, user clicked refresh, etc.). This
         * means we will be frequently loading over 100 register values in a single instance.
         *
         * For the above reason, we do not read each register value individually. That would take far too long if we
         * have over 100 registers to read. Instead, we calculate the address ranges for each memory type we'll be
         * reading from, and then perform a single read operation for each memory type.
         */
        auto output = TargetRegisterDescriptorAndValuePairs{};

        /*
         * AVR registers can be accessed via one of two memory types, depending on the register type and config
         * variant.
         *
         * For debugWIRE and JTAG config variants, all registers (in the data address space) can be accessed via the
         * SRAM memory type. This includes general purpose registers.
         *
         * For PDI and UPDI config variants, all registers *EXCEPT* general purpose registers can be accessed via the
         * SRAM memory type. General purpose registers can only be accessed via the REGISTER_FILE memory type.
         */
        auto sramAddressRange = std::optional<TargetMemoryAddressRange>{};
        auto registerFileAddressRange = std::optional<TargetMemoryAddressRange>{};

        auto sramRegisterDescriptors = TargetRegisterDescriptors{};
        auto registerFileRegisterDescriptors = TargetRegisterDescriptors{};

        for (const auto& descriptor : descriptors) {
            const auto regMemoryType = this->getRegisterMemoryType(*descriptor);
            const auto endAddress = descriptor->startAddress + (descriptor->size - 1);

            if (regMemoryType == Avr8MemoryType::REGISTER_FILE) {
                // This register can only be accessed via the REGISTER_FILE memory type
                if (registerFileAddressRange.has_value()) {
                    if (descriptor->startAddress < registerFileAddressRange->startAddress) {
                        registerFileAddressRange->startAddress = descriptor->startAddress;
                    }

                    if (endAddress > registerFileAddressRange->endAddress) {
                        registerFileAddressRange->endAddress = endAddress;
                    }

                } else {
                    registerFileAddressRange = TargetMemoryAddressRange(
                        descriptor->startAddress,
                        descriptor->startAddress + (descriptor->size - 1)
                    );
                }

                registerFileRegisterDescriptors.push_back(descriptor);
            }

            if (regMemoryType == Avr8MemoryType::SRAM) {
                // This register can be accessed via the SRAM memory type
                if (sramAddressRange.has_value()) {
                    if (descriptor->startAddress < sramAddressRange->startAddress) {
                        sramAddressRange->startAddress = descriptor->startAddress;
                    }

                    if (endAddress > sramAddressRange->endAddress) {
                        sramAddressRange->endAddress = endAddress;
                    }

                } else {
                    sramAddressRange = TargetMemoryAddressRange(
                        descriptor->startAddress,
                        descriptor->startAddress + (descriptor->size - 1)
                    );
                }

                sramRegisterDescriptors.push_back(descriptor);
            }
        }

        if (sramAddressRange.has_value()) {
            const auto readSize = (sramAddressRange->endAddress - sramAddressRange->startAddress) + 1;

            /*
             * When reading from SRAM, we must avoid any attempts to access the OCD data register (OCDDR), as the
             * debug tool will reject the command and respond with a 0x36 error code (invalid address error).
             *
             * For this reason, we specify the OCDDR address as an excluded address. This will mean
             * the EdbgAvr8Interface::readMemory() function will avoid reading from that particular address.
             */
            auto excludedAddresses = std::set<TargetMemoryAddress>{};
            if (this->session.ocdDataRegister.has_value()) {
                excludedAddresses.insert(*(this->session.ocdDataRegister) + this->session.ioMemorySegment.startAddress);
            }

            const auto flatMemoryData = this->readMemory(
                Avr8MemoryType::SRAM,
                sramAddressRange->startAddress,
                readSize,
                excludedAddresses
            );

            if (flatMemoryData.size() != readSize) {
                throw Exception{
                    "Failed to read memory via SRAM memory type - address range: "
                        + std::to_string(sramAddressRange->startAddress) + " - "
                        + std::to_string(sramAddressRange->endAddress) + ". Expected " + std::to_string(readSize)
                        + " bytes, got " + std::to_string(flatMemoryData.size())
                };
            }

            for (const auto& descriptor : sramRegisterDescriptors) {
                /*
                 * Multibyte AVR8 registers are stored in LSB form.
                 *
                 * We use reverse iterators here to extract the data in MSB form.
                 */
                const auto bufferStartIt = flatMemoryData.rend()
                    - (descriptor->startAddress - sramAddressRange->startAddress) - descriptor->size;

                output.emplace_back(
                    *descriptor,
                    TargetMemoryBuffer{bufferStartIt, bufferStartIt + descriptor->size}
                );
            }
        }

        if (registerFileAddressRange.has_value()) {
            const auto readSize = (registerFileAddressRange->endAddress - registerFileAddressRange->startAddress) + 1;

            const auto flatMemoryData = this->readMemory(
                Avr8MemoryType::REGISTER_FILE,
                registerFileAddressRange->startAddress,
                readSize,
                {}
            );

            if (flatMemoryData.size() != readSize) {
                throw Exception{
                    "Failed to read memory via REGISTER_FILE memory type - address range: "
                        + std::to_string(registerFileAddressRange->startAddress) + " - "
                        + std::to_string(registerFileAddressRange->endAddress) + ". Expected "
                        + std::to_string(readSize) + " bytes, got " + std::to_string(flatMemoryData.size())
                };
            }

            for (const auto& descriptor : registerFileRegisterDescriptors) {
                const auto bufferStartIt = flatMemoryData.rend()
                    - (descriptor->startAddress - registerFileAddressRange->startAddress) - descriptor->size;

                output.emplace_back(
                    *descriptor,
                    TargetMemoryBuffer{bufferStartIt, bufferStartIt + descriptor->size}
                );
            }
        }

        return output;
    }

    void EdbgAvr8Interface::writeRegisters(
        const TargetRegisterDescriptorAndValuePairs& registers
    ) {
        using Targets::TargetRegisterType;

        for (const auto& [descriptor, value] : registers) {
            auto valueCopy = value;

            if (valueCopy.empty()) {
                throw Exception{"Cannot write empty register value"};
            }

            if (valueCopy.size() > descriptor.size) {
                throw Exception{"Register value exceeds size specified by register descriptor."};
            }

            if (valueCopy.size() < descriptor.size) {
                // Fill the missing most-significant bytes with 0x00
                valueCopy.insert(valueCopy.begin(), descriptor.size - valueCopy.size(), 0x00);
            }

            if (valueCopy.size() > 1) {
                // AVR8 registers are stored in LSB
                std::reverse(valueCopy.begin(), valueCopy.end());
            }

            // TODO: This can be inefficient when updating many registers, maybe do something a little smarter here.
            this->writeMemory(this->getRegisterMemoryType(descriptor), descriptor.startAddress, valueCopy);
        }
    }

    TargetMemoryBuffer EdbgAvr8Interface::readMemory(
        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        TargetMemoryAddress startAddress,
        TargetMemorySize bytes,
        const std::set<Targets::TargetMemoryAddressRange>& excludedAddressRanges
    ) {
        if (this->programmingModeEnabled && memorySegmentDescriptor.type == TargetMemorySegmentType::RAM) {
            throw Exception{"Cannot access RAM when programming mode is enabled"};
        }

        /*
         * The internal readMemory() function accepts excluded addresses in the form of a set of addresses, as
         * opposed to a set of address ranges.
         *
         * We will perform the conversion here.
         */
        auto excludedAddresses = std::set<TargetMemoryAddress>{};
        const auto endAddress = startAddress + bytes - 1;

        for (const auto& addressRange : excludedAddressRanges) {
            if (addressRange.startAddress > endAddress) {
                // This address range is outside the range from which we will be reading
                continue;
            }

            for (auto i = addressRange.startAddress; i <= addressRange.endAddress; ++i) {
                excludedAddresses.insert(i);
            }
        }

        if (memorySegmentDescriptor.type == TargetMemorySegmentType::FLASH) {
            if (this->session.configVariant == Avr8ConfigVariant::MEGAJTAG) {
                return this->readMemory(
                    this->programmingModeEnabled ? Avr8MemoryType::FLASH_PAGE : Avr8MemoryType::SPM,
                    startAddress,
                    bytes,
                    excludedAddresses
                );
            }

            if (this->session.configVariant == Avr8ConfigVariant::XMEGA) {
                const auto bootSectionStartAddress = this->session.programBootSection.value().get().startAddress;
                if (startAddress >= bootSectionStartAddress) {
                    /*
                     * When using the BOOT_FLASH memory type, the address should be relative to the start of the
                     * boot section.
                     */
                    return this->readMemory(
                        Avr8MemoryType::BOOT_FLASH,
                        startAddress - bootSectionStartAddress,
                        bytes,
                        excludedAddresses
                    );
                }

                /*
                 * When using the APPL_FLASH memory type, the address should be relative to the start of the
                 * application section.
                 */
                return this->readMemory(
                    Avr8MemoryType::APPL_FLASH,
                    startAddress - this->session.programAppSection.value().get().startAddress,
                    bytes,
                    excludedAddresses
                );
            }

            return this->readMemory(Avr8MemoryType::FLASH_PAGE, startAddress, bytes, excludedAddresses);
        }

        if (memorySegmentDescriptor.type == TargetMemorySegmentType::EEPROM) {
            if (this->session.configVariant == Avr8ConfigVariant::MEGAJTAG) {
                return this->readMemory(
                    this->programmingModeEnabled ? Avr8MemoryType::EEPROM_PAGE : Avr8MemoryType::EEPROM,
                    startAddress,
                    bytes,
                    excludedAddresses
                );
            }

            if (this->session.configVariant == Avr8ConfigVariant::XMEGA) {
                // EEPROM addresses should be in relative form, for XMEGA (PDI) targets
                return this->readMemory(
                    Avr8MemoryType::EEPROM,
                    startAddress - this->session.eepromMemorySegment.startAddress,
                    bytes,
                    excludedAddresses
                );
            }

            return this->readMemory(Avr8MemoryType::EEPROM, startAddress, bytes, excludedAddresses);
        }

        if (
            memorySegmentDescriptor.type == TargetMemorySegmentType::FUSES
            && this->programmingModeEnabled
            && this->session.configVariant != Avr8ConfigVariant::DEBUG_WIRE
        ) {
            if (this->session.configVariant == Avr8ConfigVariant::XMEGA) {
                return this->readMemory(
                    Avr8MemoryType::FUSES,
                    startAddress - this->session.fuseMemorySegment.startAddress,
                    bytes,
                    excludedAddresses
                );
            }

            return this->readMemory(Avr8MemoryType::FUSES, startAddress, bytes, excludedAddresses);
        }

        return this->readMemory(Avr8MemoryType::SRAM, startAddress, bytes, excludedAddresses);
    }

    void EdbgAvr8Interface::writeMemory(
        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        TargetMemoryAddress startAddress,
        Targets::TargetMemoryBufferSpan buffer
    ) {
        if (memorySegmentDescriptor.type == TargetMemorySegmentType::FLASH) {
            if (this->session.configVariant == Avr8ConfigVariant::XMEGA) {
                const auto bootSectionStartAddress = this->session.programBootSection.value().get().startAddress;
                if (startAddress >= bootSectionStartAddress) {
                    /*
                     * When using the BOOT_FLASH memory type, the address should be relative to the start of the
                     * boot section.
                     */
                    return this->writeMemory(
                        Avr8MemoryType::BOOT_FLASH,
                        startAddress - bootSectionStartAddress,
                        buffer
                    );

                } else {
                    /*
                     * When using the APPL_FLASH memory type, the address should be relative to the start of the
                     * application section.
                     */
                    return this->writeMemory(
                        Avr8MemoryType::APPL_FLASH,
                        startAddress - this->session.programAppSection.value().get().startAddress,
                        buffer
                    );
                }

                return this->writeMemory(Avr8MemoryType::FLASH_PAGE, startAddress, buffer);
            }

            return this->writeMemory(Avr8MemoryType::FLASH_PAGE, startAddress, buffer);
        }

        if (memorySegmentDescriptor.type == TargetMemorySegmentType::EEPROM) {
            if (this->session.configVariant == Avr8ConfigVariant::MEGAJTAG) {
                return this->writeMemory(
                    this->programmingModeEnabled ? Avr8MemoryType::EEPROM_PAGE : Avr8MemoryType::EEPROM,
                    startAddress,
                    buffer
                );
            }

            if (this->session.configVariant == Avr8ConfigVariant::XMEGA) {
                // EEPROM addresses should be in relative form, for XMEGA (PDI) targets
                return this->writeMemory(
                    Avr8MemoryType::EEPROM_ATOMIC,
                    startAddress - this->session.eepromMemorySegment.startAddress,
                    buffer
                );
            }

            if (this->session.configVariant == Avr8ConfigVariant::UPDI) {
                return this->writeMemory(Avr8MemoryType::EEPROM_ATOMIC, startAddress, buffer);
            }

            return this->writeMemory(Avr8MemoryType::EEPROM, startAddress, buffer);
        }

        if (
            memorySegmentDescriptor.type == TargetMemorySegmentType::FUSES
            && this->programmingModeEnabled
            && this->session.configVariant != Avr8ConfigVariant::DEBUG_WIRE
        ) {
            if (this->session.configVariant == Avr8ConfigVariant::XMEGA) {
                // Fuse addresses should be in relative form, for XMEGA (PDI) targets
                return this->writeMemory(
                    Avr8MemoryType::FUSES,
                    startAddress - this->session.fuseMemorySegment.startAddress,
                    buffer
                );
            }

            return this->writeMemory(Avr8MemoryType::FUSES, startAddress, buffer);
        }

        return this->writeMemory(Avr8MemoryType::SRAM, startAddress, buffer);
    }

    void EdbgAvr8Interface::eraseProgramMemory(std::optional<ProgramMemorySection> section) {
        // The EDBG erase command with a specified "mode" parameter is only supported by XMEGA targets
        if (this->session.configVariant != Avr8ConfigVariant::XMEGA) {
            throw Exception{"debugWIRE, JTAG and UPDI targets do not support EDBG program memory erase command"};
        }

        if (!section.has_value() || *section == ProgramMemorySection::BOOT) {
            const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
                EraseMemory{Avr8EraseMemoryMode::BOOT_SECTION}
            );

            if (responseFrame.id == Avr8ResponseId::FAILED) {
                throw Avr8CommandFailure{"AVR8 erase memory command (for BOOT section) failed", responseFrame};
            }
        }

        if (!section.has_value() || *section == ProgramMemorySection::APPLICATION) {
            const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
                EraseMemory{Avr8EraseMemoryMode::APPLICATION_SECTION}
            );

            if (responseFrame.id == Avr8ResponseId::FAILED) {
                throw Avr8CommandFailure{"AVR8 erase memory command (for APPLICATION section) failed", responseFrame};
            }
        }
    }

    void EdbgAvr8Interface::eraseChip() {
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            EraseMemory{Avr8EraseMemoryMode::CHIP}
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure{"AVR8 erase memory command failed", responseFrame};
        }
    }

    TargetExecutionState EdbgAvr8Interface::getExecutionState() {
        /*
         * We are not informed when a target goes from a stopped state to a running state, so there is no need
         * to query the tool when we already know the target has stopped.
         *
         * This means we have to rely on the assumption that the target cannot enter a running state without
         * our instruction.
         */
        if (this->cachedExecutionState != TargetExecutionState::STOPPED) {
            this->refreshTargetState();
        }

        return this->cachedExecutionState;
    }

    void EdbgAvr8Interface::enableProgrammingMode() {
        if (this->programmingModeEnabled) {
            return;
        }

        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            EnterProgrammingMode{}
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure{"Failed to enter programming mode on EDBG debug tool", responseFrame};
        }

        this->programmingModeEnabled = true;
        this->hardwareBreakpointNumbersByAddress.clear();
    }

    void EdbgAvr8Interface::disableProgrammingMode() {
        if (!this->programmingModeEnabled) {
            return;
        }

        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            LeaveProgrammingMode{}
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure{"Failed to leave programming mode on EDBG debug tool", responseFrame};
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
            SetParameter{parameter, value}
        );

        Logger::debug(
            "Setting AVR8 EDBG parameter (context: 0x" + StringService::toHex(parameter.context) + ", id: 0x"
                + StringService::toHex(parameter.id) + ", value: 0x" + StringService::toHex(value) + ")"
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure{"Failed to set parameter on device!", responseFrame};
        }
    }

    std::vector<unsigned char> EdbgAvr8Interface::getParameter(const Avr8EdbgParameter& parameter, std::uint8_t size) {
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            GetParameter{parameter, size}
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure{"Failed to get parameter from device!", responseFrame};
        }

        return responseFrame.getPayloadData();
    }

    void EdbgAvr8Interface::setDebugWireAndJtagParameters() {
        const auto parameters = Parameters::Avr8Generic::DebugWireJtagParameters{this->session.targetDescriptionFile};

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
        const auto parameters = Parameters::Avr8Generic::PdiParameters{this->session.targetDescriptionFile};

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
        const auto parameters = Parameters::Avr8Generic::UpdiParameters{this->session.targetDescriptionFile};

        /*
         * The program memory base address field for UPDI sessions (DEVICE_UPDI_PROGMEM_BASE_ADDR) seems to be
         * limited to two bytes in size, as opposed to the four byte size for the debugWIRE, JTAG and PDI
         * equivalent fields. This is why, I suspect, another field was required for the most significant byte of
         * the program memory base address (DEVICE_UPDI_PROGMEM_BASE_ADDR_MSB).
         *
         * The additional DEVICE_UPDI_PROGMEM_BASE_ADDR_MSB field is only one byte in size, so it brings the total
         * capacity for the program memory base address to 24 bits.
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

        Logger::debug("Setting UPDI_24_BIT_ADDRESSING_ENABLE AVR8 device parameter");
        this->setParameter(
            Avr8EdbgParameters::DEVICE_UPDI_24_BIT_ADDRESSING_ENABLE,
            parameters.programMemoryStartAddress > 0xFFFF
                ? std::uint8_t{1}
                : std::uint8_t{0}
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
            ActivatePhysical{applyExternalReset}
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            if (!applyExternalReset) {
                // Try again with external reset applied
                Logger::debug(
                    "Failed to activate physical interface on AVR8 target - retrying with external reset applied."
                );
                return this->activatePhysical(true);
            }

            throw Avr8CommandFailure{"AVR8 Activate physical interface command failed", responseFrame};
        }

        this->physicalInterfaceActivated = true;
    }

    void EdbgAvr8Interface::deactivatePhysical() {
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            DeactivatePhysical{}
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure{"AVR8 Deactivate physical interface command failed", responseFrame};
        }

        this->physicalInterfaceActivated = false;
    }

    void EdbgAvr8Interface::attach() {
        /*
         * When attaching an ATmega JTAG target, we must not set the breakAfterAttach flag, as this results in a
         * timeout.
         *
         * However, in this case the 'attach' command seems to _sometimes_ halt the target anyway, regardless of the
         * value of the breakAfterAttach flag. So we still expect a stop event to be received shortly after issuing
         * the 'attach' command.
         */
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            Attach{this->session.configVariant != Avr8ConfigVariant::MEGAJTAG}
        );

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure{"AVR8 Attach command failed", responseFrame};
        }

        this->targetAttached = true;

        try {
            // Wait for stopped event
            this->waitForStoppedEvent();

        } catch (const Exception& exception) {
            Logger::warning("Execution on AVR8 target could not be halted post attach - " + exception.getMessage());
        }
    }

    void EdbgAvr8Interface::detach() {
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(Detach{});
        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure{"AVR8 Detach command failed", responseFrame};
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
                 * TODO: This isn't very nice as we're performing an unnecessary copy. Maybe requestAvrEvent should
                 *       return a unique_ptr instead?
                 */
                return std::make_unique<AvrEvent>(event.value());
            }
        }
    }

    void EdbgAvr8Interface::clearEvents() {
        while (this->getAvrEvent() != nullptr) {}
    }

    Avr8MemoryType EdbgAvr8Interface::getRegisterMemoryType(const TargetRegisterDescriptor& descriptor) {
        return (
            descriptor.type == Targets::TargetRegisterType::GENERAL_PURPOSE_REGISTER
            && (
                this->session.configVariant == Avr8ConfigVariant::XMEGA
                || this->session.configVariant == Avr8ConfigVariant::UPDI
            )
        ) ? Avr8MemoryType::REGISTER_FILE : Avr8MemoryType::SRAM;
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

    TargetMemoryAddress EdbgAvr8Interface::alignMemoryAddress(Avr8MemoryType memoryType, TargetMemoryAddress address) {
        auto alignTo = std::uint16_t{1};

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
            return (address / alignTo) * alignTo;
        }

        return address;
    }

    TargetMemorySize EdbgAvr8Interface::alignMemoryBytes(Avr8MemoryType memoryType, TargetMemorySize bytes) {
        auto alignTo = std::uint16_t{1};

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
                static_cast<double>(bytes) / static_cast<double>(alignTo)
            ) * alignTo);
        }

        return bytes;
    }

    TargetMemorySize EdbgAvr8Interface::maximumMemoryAccessSize(Avr8MemoryType memoryType) {
        if (
            memoryType == Avr8MemoryType::FLASH_PAGE
            || memoryType == Avr8MemoryType::APPL_FLASH
            || memoryType == Avr8MemoryType::BOOT_FLASH
            || (memoryType == Avr8MemoryType::SPM && this->session.configVariant == Avr8ConfigVariant::MEGAJTAG)
        ) {
            // These flash memory types require single page access.
            return this->session.programMemorySegment.pageSize.value();
        }

        if (memoryType == Avr8MemoryType::EEPROM_ATOMIC || memoryType == Avr8MemoryType::EEPROM_PAGE) {
            // These EEPROM memory types requires single page access.
            return this->session.eepromMemorySegment.pageSize.value();
        }

        if (this->maximumMemoryAccessSizePerRequest.has_value()) {
            // There is a memory access size limit for this entire EdbgAvr8Interface instance
            return *(this->maximumMemoryAccessSizePerRequest);
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
        return static_cast<Targets::TargetMemorySize>((this->edbgInterface->getUsbHidInputReportSize() - 30) * 2);
    }

    TargetMemoryBuffer EdbgAvr8Interface::readMemory(
        Avr8MemoryType type,
        TargetMemoryAddress startAddress,
        TargetMemorySize bytes,
        const std::set<TargetMemoryAddress>& excludedAddresses
    ) {
        if (type == Avr8MemoryType::FUSES && this->session.configVariant == Avr8ConfigVariant::DEBUG_WIRE) {
            throw Exception{"Cannot access AVR fuses via the debugWIRE interface"};
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
            auto output = TargetMemoryBuffer{};
            output.reserve(bytes);

            auto segmentStartAddress = startAddress;
            const auto endAddress = startAddress + bytes - 1;

            for (const auto excludedAddress : excludedAddresses) {
                if (excludedAddress < startAddress || excludedAddress > endAddress) {
                    // This excluded address is outside the range from which we are reading, so it can be ignored.
                    continue;
                }

                const auto segmentSize = excludedAddress - segmentStartAddress;
                if (segmentSize > 0) {
                    auto segmentBuffer = this->readMemory(type, segmentStartAddress, segmentSize);
                    std::move(segmentBuffer.begin(), segmentBuffer.end(), std::back_inserter(output));
                }

                output.emplace_back(0x00);
                segmentStartAddress = excludedAddress + 1;
            }

            // Read final segment
            const auto finalReadBytes = (endAddress - segmentStartAddress) + 1;
            if (finalReadBytes > 0) {
                auto segmentBuffer = this->readMemory(type, segmentStartAddress, finalReadBytes);
                std::move(segmentBuffer.begin(), segmentBuffer.end(), std::back_inserter(output));
            }

            if (managingProgrammingMode) {
                this->disableProgrammingMode();
            }

            return output;
        }

        if (this->alignmentRequired(type)) {
            const auto alignedStartAddress = this->alignMemoryAddress(type, startAddress);
            const auto alignedBytes = this->alignMemoryBytes(type, bytes + (startAddress - alignedStartAddress));

            if (alignedStartAddress != startAddress || alignedBytes != bytes) {
                auto memoryBuffer = this->readMemory(type, alignedStartAddress, alignedBytes, excludedAddresses);

                const auto offset = memoryBuffer.begin() + (startAddress - alignedStartAddress);
                auto output = TargetMemoryBuffer{};
                output.reserve(bytes);
                std::move(offset, offset + bytes, std::back_inserter(output));

                return output;
            }
        }

        const auto maximumReadSize = this->maximumMemoryAccessSize(type);
        if (bytes > maximumReadSize) {
            auto output = TargetMemoryBuffer{};
            output.reserve(bytes);

            while (output.size() < bytes) {
                const auto bytesToRead = std::min(
                    static_cast<TargetMemorySize>(bytes - output.size()),
                    maximumReadSize
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
            ReadMemory{type, startAddress, bytes, excludedAddresses}
        );

        if (managingProgrammingMode) {
            this->disableProgrammingMode();
        }

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure{"AVR8 Read memory command failed", responseFrame};
        }

        auto data = responseFrame.getMemoryData();

        if (data.size() != bytes) {
            throw Avr8CommandFailure{"Unexpected number of bytes returned from EDBG debug tool"};
        }

        return data;
    }

    void EdbgAvr8Interface::writeMemory(
        Avr8MemoryType type,
        TargetMemoryAddress startAddress,
        Targets::TargetMemoryBufferSpan buffer
    ) {
        if (type == Avr8MemoryType::FUSES && this->session.configVariant == Avr8ConfigVariant::DEBUG_WIRE) {
            throw Exception{"Cannot access AVR fuses via the debugWIRE interface"};
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
                 */
                const auto readMemType = type == Avr8MemoryType::EEPROM_ATOMIC ? Avr8MemoryType::EEPROM : type;
                auto alignedBuffer = (alignedStartAddress < startAddress)
                    ? this->readMemory(readMemType, alignedStartAddress, startAddress - alignedStartAddress)
                    : TargetMemoryBuffer{};

                alignedBuffer.resize(alignedBytes);

                std::copy(
                    buffer.begin(),
                    buffer.end(),
                    alignedBuffer.begin() + (startAddress - alignedStartAddress)
                );

                const auto dataBack = this->readMemory(
                    readMemType,
                    startAddress + bytes,
                    alignedBytes - bytes - (startAddress - alignedStartAddress),
                    {}
                );
                std::copy(
                    dataBack.begin(),
                    dataBack.end(),
                    alignedBuffer.begin() + (startAddress - alignedStartAddress) + bytes
                );

                return this->writeMemory(type, alignedStartAddress, alignedBuffer);
            }
        }

        const auto maximumWriteSize = this->maximumMemoryAccessSize(type);
        if (buffer.size() > maximumWriteSize) {
            auto bytesWritten = TargetMemorySize{0};

            while (bytesWritten < buffer.size()) {
                const auto chunkSize = std::min(
                    static_cast<TargetMemorySize>(buffer.size() - bytesWritten),
                    maximumWriteSize
                );

                this->writeMemory(
                    type,
                    startAddress + bytesWritten,
                    buffer.subspan(bytesWritten, chunkSize)
                );

                bytesWritten += chunkSize;
            }

            return;
        }

        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            WriteMemory{type, startAddress, buffer}
        );

        // We must disable and re-enable programming mode, in order for the changes to the fuse bit to take effect.
        if (type == Avr8MemoryType::FUSES) {
            this->disableProgrammingMode();

            if (!managingProgrammingMode) {
                this->enableProgrammingMode();
            }
        }

        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure{"AVR8 Write memory command failed", responseFrame};
        }
    }

    void EdbgAvr8Interface::refreshTargetState() {
        const auto avrEvent = this->getAvrEvent();

        if (avrEvent != nullptr && avrEvent->eventId == AvrEventId::AVR8_BREAK_EVENT) {
            auto* breakEvent = dynamic_cast<BreakEvent*>(avrEvent.get());

            if (breakEvent == nullptr) {
                throw Exception{"Failed to process AVR8 break event"};
            }

            this->cachedExecutionState = TargetExecutionState::STOPPED;
            return;
        }

        if (this->cachedExecutionState != TargetExecutionState::STEPPING) {
            this->cachedExecutionState = TargetExecutionState::RUNNING;
        }
    }

    void EdbgAvr8Interface::disableDebugWire() {
        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(DisableDebugWire{});
        if (responseFrame.id == Avr8ResponseId::FAILED) {
            throw Avr8CommandFailure{"AVR8 Disable debugWIRE command failed", responseFrame};
        }
    }

    void EdbgAvr8Interface::waitForStoppedEvent() {
        auto breakEvent = this->waitForAvrEvent<BreakEvent>();
        if (breakEvent == nullptr) {
            throw Exception{"Failed to receive break event for AVR8 target"};
        }

        this->cachedExecutionState = TargetExecutionState::STOPPED;
    }
}
