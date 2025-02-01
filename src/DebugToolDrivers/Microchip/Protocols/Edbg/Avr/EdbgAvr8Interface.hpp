#pragma once

#include <cstdint>
#include <chrono>
#include <thread>
#include <optional>
#include <cassert>

#include "src/DebugToolDrivers/TargetInterfaces/Microchip/Avr8/Avr8DebugInterface.hpp"
#include "src/DebugToolDrivers/Microchip/Protocols/Edbg/EdbgInterface.hpp"

#include "Avr8Generic.hpp"
#include "EdbgAvr8Session.hpp"

#include "src/Targets/TargetPhysicalInterface.hpp"
#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/ProgramBreakpointRegistry.hpp"
#include "src/Targets/TargetRegisterDescriptor.hpp"
#include "src/Targets/Microchip/Avr8/TargetDescriptionFile.hpp"
#include "src/Targets/Microchip/Avr8/Family.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr
{
    /**
     * The EdbgAvr8Interface implements the AVR8 Generic EDBG/CMSIS-DAP protocol, as an Avr8DebugInterface.
     *
     * See the "AVR8 Generic Protocol" section in the DS50002630A document by Microchip, for more information on the
     * protocol.
     *
     * This implementation should work with any Microchip EDBG-based CMSIS-DAP debug tool (such as the Atmel-ICE,
     * Power Debugger, the MPLAB SNAP debugger (in "AVR mode"), etc).
     */
    class EdbgAvr8Interface: public ::DebugToolDrivers::TargetInterfaces::Microchip::Avr8::Avr8DebugInterface
    {
    public:
        explicit EdbgAvr8Interface(
            EdbgInterface* edbgInterface,
            const Targets::Microchip::Avr8::TargetDescriptionFile& targetDescriptionFile,
            const Targets::Microchip::Avr8::Avr8TargetConfig& targetConfig
        );

        /**
         * Some EDBG devices don't seem to operate correctly when actioning the masked memory read EDBG command. The
         * data returned in response to the command appears to be completely incorrect.
         *
         * Setting this flag to true will disable the EdbgAvr8Interface driver's use of the masked memory read command.
         * The driver will perform the masking itself, and then issue standard read memory commands. See the
         * implementation of EdbgAvr8Interface::readMemory() for more.
         *
         * NOTE: Masked memory read commands are only implemented for SRAM reads. EDBG debug tools report EEPROM and
         * FLASH as invalid memory types, when using the masked memory read command. So any masked reads to non-SRAM
         * will result in driver-side masking, regardless of the value of this flag.
         *
         * NOTE: We now avoid masked memory read commands by default, unless this flag is explicitly set to false.
         * This means the default value of this->avoidMaskedMemoryRead is true.
         *
         * @param avoidMaskedMemoryRead
         */
        void setAvoidMaskedMemoryRead(bool avoidMaskedMemoryRead) {
            this->avoidMaskedMemoryRead = avoidMaskedMemoryRead;
        }

        /**
         * Some EDBG AVR8 debug tools behave in a really weird way when servicing read memory requests that exceed a
         * certain size. For example, the ATMEGA4809-XPRO Xplained Pro debug tool returns incorrect data for any read
         * memory command that exceeds 256 bytes in the size of the read request, despite the fact that the HID report
         * size is 512 bytes. The debug tool doesn't report any error, it just returns incorrect data.
         *
         * To address this, debug tool drivers can set a soft limit on the number of bytes this EdbgAvr8Interface
         * instance will attempt to access in a single request, using the EdbgAvr8Interface::setMaximumMemoryAccessSizePerRequest()
         * member function.
         *
         * This limit will be enforced in all forms of memory access on the AVR8 target, including register access.
         * However, it will *NOT* be enforced for memory types that require page alignment, where the page size is
         * greater than the set limit. In these cases, the limit will effectively be ignored. I've tested this on
         * the ATXMEGAA1U-XPRO Xplained Pro debug tool, where the flash page size is 512 bytes (but the limit is set to
         * 256 bytes), and flash memory access works fine. This makes me suspect that this issue may be specific to the
         * ATMEGA4809-XPRO Xplained Pro debug tool.
         *
         * @param maximumSize
         */
        void setMaximumMemoryAccessSizePerRequest(Targets::TargetMemorySize maximumSize) {
            this->maximumMemoryAccessSizePerRequest = maximumSize;
        }

        void setReactivateJtagTargetPostProgrammingMode(bool reactivateJtagTargetPostProgrammingMode) {
            this->reactivateJtagTargetPostProgrammingMode = reactivateJtagTargetPostProgrammingMode;
        }

        /*
         * The public methods below implement the interface defined by the Avr8Interface class.
         * See the comments in that class for more info on the expected behaviour of each method.
         */

        void init() override;

        void stop() override;
        void run() override;
        void runTo(Targets::TargetMemoryAddress address) override;
        void step() override;
        void reset() override;

        void activate() override;
        void deactivate() override;

        void applyAccessRestrictions(
            Targets::TargetRegisterDescriptor& registerDescriptor,
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor
        ) override;

        Targets::TargetMemoryAddress getProgramCounter() override;
        void setProgramCounter(Targets::TargetMemoryAddress programCounter) override;

        Targets::Microchip::Avr8::TargetSignature getDeviceId() override;

        void setProgramBreakpoint(const Targets::TargetProgramBreakpoint& breakpoint) override;
        void removeProgramBreakpoint(const Targets::TargetProgramBreakpoint& breakpoint) override;
        void clearAllBreakpoints() override;

        Targets::TargetRegisterDescriptorAndValuePairs readRegisters(
            const Targets::TargetRegisterDescriptors& descriptors
        ) override;
        void writeRegisters(const Targets::TargetRegisterDescriptorAndValuePairs& registers) override;

        Targets::TargetMemoryBuffer readMemory(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemorySize bytes,
            const std::set<Targets::TargetMemoryAddressRange>& excludedAddressRanges = {}
        ) override;
        void writeMemory(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemoryBufferSpan buffer
        ) override;

        void eraseProgramMemory(
            std::optional<Targets::Microchip::Avr8::ProgramMemorySection> section = std::nullopt
        ) override;
        void eraseChip() override;

        Targets::TargetExecutionState getExecutionState() override;

        void enableProgrammingMode() override;
        void disableProgrammingMode() override;

    private:
        EdbgInterface* edbgInterface = nullptr;
        EdbgAvr8Session session;

        bool avoidMaskedMemoryRead = true;
        std::optional<Targets::TargetMemorySize> maximumMemoryAccessSizePerRequest;
        bool reactivateJtagTargetPostProgrammingMode = false;
        Targets::TargetExecutionState cachedExecutionState = Targets::TargetExecutionState::UNKNOWN;
        bool physicalInterfaceActivated = false;
        bool targetAttached = false;
        bool programmingModeEnabled = false;

        /**
         * The TargetController refreshes the relevant program memory cache after inserting/removing software
         * breakpoints. So it expects the insertion/removal operation to take place immediately. But, EDBG tools do
         * not appear to support this. They queue the insertion/removal of software breakpoints until the next
         * program flow control command.
         *
         * To get around this, we keep track of the pending operations and intercept program memory access operations
         * to inject the relevant instruction opcodes, so that it appears as if the breakpoints have been
         * inserted/removed. In other words: we fake it - we make it seem as if the breakpoints were inserted/removed
         * immediately, when in actuality, the operation is still pending.
         *
         * It's horrible, but I can't think of a better way. Yes, I have considered loosening the restriction on the
         * target driver, but I dislike that approach even more, because it would mean making the rest of Bloom
         * accommodate the "I'll do it when I feel like it" nature of the EDBG tool, which is something I really
         * don't want to do.
         *
         * This also addresses another potential issue, where EDBG tools will filter out software breakpoints when
         * accessing program memory via some memory types. This is undesired behaviour. We negate it by injecting
         * BREAK opcodes at the addresses of all active software breakpoints, when accessing program memory via any
         * memory type.
         *
         * For more, see the following member functions:
         *   - EdbgAvr8Interface::injectActiveBreakpoints()
         *   - EdbgAvr8Interface::concealPendingBreakpointOperations()
         *   - EdbgAvr8Interface::commitPendingBreakpointOperations()
         */
        Targets::ProgramBreakpointRegistry pendingSoftwareBreakpointInsertions;
        Targets::ProgramBreakpointRegistry pendingSoftwareBreakpointDeletions;
        Targets::ProgramBreakpointRegistry activeSoftwareBreakpoints;

        /**
         * Every hardware breakpoint is assigned a "breakpoint number", which we need to keep track of in order to
         * clear a hardware breakpoint.
         */
        std::map<Targets::TargetMemoryAddress, std::uint8_t> hardwareBreakpointNumbersByAddress;

        void setTargetParameters();
        void setParameter(const Avr8EdbgParameter& parameter, const std::vector<unsigned char>& value);
        void setParameter(const Avr8EdbgParameter& parameter, std::uint8_t value) {
            this->setParameter(parameter, std::vector<unsigned char>(1, value));
        }
        void setParameter(const Avr8EdbgParameter& parameter, std::uint32_t value) {
            auto paramValue = std::vector<unsigned char>(4);
            paramValue[0] = static_cast<unsigned char>(value);
            paramValue[1] = static_cast<unsigned char>(value >> 8);
            paramValue[2] = static_cast<unsigned char>(value >> 16);
            paramValue[3] = static_cast<unsigned char>(value >> 24);

            this->setParameter(parameter, paramValue);
        }
        void setParameter(const Avr8EdbgParameter& parameter, std::uint16_t value) {
            auto paramValue = std::vector<unsigned char>(2);
            paramValue[0] = static_cast<unsigned char>(value);
            paramValue[1] = static_cast<unsigned char>(value >> 8);

            this->setParameter(parameter, paramValue);
        }

        std::vector<unsigned char> getParameter(const Avr8EdbgParameter& parameter, std::uint8_t size);

        /**
         * EDBG-based debug tools require target specific parameters such as memory locations, page sizes and
         * register addresses. These parameters can be sent to the tool before and during a session.
         *
         * What parameters we need to send depend on the physical interface (and config variant) selected by the user.
         * For target parameters, the address (ID) of the parameter also varies across config variants.
         *
         * - The setDebugWireAndJtagParameters() function sends the required target parameters for debugWIRE and JTAG
         *   sessions. Both sessions are covered in a single function because they require the same parameters.
         * - The setPdiParameters() function sends the required target parameters for PDI sessions.
         * - The setUpdiParameters() function sends the required target parameters for UPDI sessions.
         *
         * We extract the required parameters from the TDF. See the constructors for the `DebugWireJtagParameters`,
         * `PdiParameters` and `UpdiParameters` structs for more.
         */
        void setDebugWireAndJtagParameters();
        void setPdiParameters();
        void setUpdiParameters();

        void activatePhysical(bool applyExternalReset = false);
        void deactivatePhysical();

        void attach();
        void detach();

        void setSoftwareBreakpoint(Targets::TargetMemoryAddress address);
        void clearSoftwareBreakpoint(Targets::TargetMemoryAddress address);
        void setHardwareBreakpoint(Targets::TargetMemoryAddress address);
        void clearHardwareBreakpoint(Targets::TargetMemoryAddress address);
        void clearAllSoftwareBreakpoints();
        void injectActiveBreakpoints(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            Targets::TargetMemoryBuffer& data,
            Targets::TargetMemoryAddress startAddress
        );
        void concealPendingBreakpointOperations(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            Targets::TargetMemoryBuffer& data,
            Targets::TargetMemoryAddress startAddress
        );
        void commitPendingBreakpointOperations();
        void injectBreakOpcodeAtBreakpoint(
            const Targets::TargetProgramBreakpoint& breakpoint,
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            Targets::TargetMemoryBuffer& data,
            Targets::TargetMemoryAddress startAddress
        );
        void injectOriginalOpcodeAtBreakpoint(
            const Targets::TargetProgramBreakpoint& breakpoint,
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            Targets::TargetMemoryBuffer& data,
            Targets::TargetMemoryAddress startAddress
        );

        std::unique_ptr<AvrEvent> getAvrEvent();
        void clearEvents();

        Avr8MemoryType getRegisterMemoryType(const Targets::TargetRegisterDescriptor& descriptor);

        bool alignmentRequired(Avr8MemoryType memoryType);
        Targets::TargetMemoryAddress alignMemoryAddress(Avr8MemoryType memoryType, Targets::TargetMemoryAddress address);
        Targets::TargetMemorySize alignMemoryBytes(Avr8MemoryType memoryType, Targets::TargetMemorySize bytes);

        Targets::TargetMemorySize maximumMemoryAccessSize(Avr8MemoryType memoryType);

        /**
         * Reads memory on the target.
         *
         * This method will handle any alignment requirements for the selected memory type.
         *
         * See the Avr8MemoryType enum for list of supported AVR8 memory types.
         *
         * @param type
         *  The type of memory to access (Flash, EEPROM, SRAM, etc). See protocol documentation for more on this.
         *
         * @param startAddress
         *  The start address (byte address)
         *
         * @param bytes
         *  Number of bytes to access.
         *
         * @param excludedAddresses
         *  A set of addresses to exclude from the read operation. This is used to read memory ranges that could
         *  involve accessing an illegal address, like the OCDDR address.
         *
         * @return
         */
        Targets::TargetMemoryBuffer readMemory(
            Avr8MemoryType type,
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemorySize bytes,
            const std::set<Targets::TargetMemoryAddress>& excludedAddresses = {}
        );

        /**
         * Writes memory to the target.
         *
         * This method will handle any alignment requirements for the selected memory type.
         *
         * See the Avr8MemoryType enum for list of supported AVR8 memory types.
         *
         * @param type
         * @param address
         * @param buffer
         */
        void writeMemory(
            Avr8MemoryType type,
            Targets::TargetMemoryAddress address,
            Targets::TargetMemoryBufferSpan buffer
        );

        void refreshTargetState();

        void disableDebugWire();

        /**
         * Waits for an AVR event of a specific type.
         *
         * @tparam AvrEventType
         *  Type of AVR event to wait for. See AvrEvent class for more.
         *
         * @param maximumAttempts
         *  Maximum number of attempts to poll the debug tool for the expected event.
         *
         * @return
         *  If an event is found before maximumAttempts is reached, the event will be returned. Otherwise a nullptr
         *  will be returned.
         */
        template <class AvrEventType>
        std::unique_ptr<AvrEventType> waitForAvrEvent(int maximumAttempts = 60) {
            int attemptCount = 0;

            while (attemptCount <= maximumAttempts) {
                auto genericEvent = this->getAvrEvent();

                if (genericEvent != nullptr) {
                    // Attempt to downcast event
                    auto event = std::unique_ptr<AvrEventType>(dynamic_cast<AvrEventType*>(genericEvent.release()));

                    if (event != nullptr) {
                        return event;
                    }
                }

                std::this_thread::sleep_for(std::chrono::milliseconds{5});
                attemptCount++;
            }

            return nullptr;
        }
        void waitForStoppedEvent();
    };
}
