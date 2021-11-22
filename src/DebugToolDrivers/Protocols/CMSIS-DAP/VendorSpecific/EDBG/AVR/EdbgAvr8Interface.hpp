#pragma once

#include <cstdint>
#include <chrono>
#include <thread>
#include <cassert>

#include "src/DebugToolDrivers/TargetInterfaces/Microchip/AVR/AVR8/Avr8Interface.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/Avr8Generic.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/EdbgInterface.hpp"
#include "src/Targets/Microchip/AVR/Target.hpp"
#include "src/Targets/Microchip/AVR/AVR8/Family.hpp"
#include "src/Targets/Microchip/AVR/AVR8/PhysicalInterface.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr
{
    /**
     * The EdbgAvr8Interface implements the AVR8 Generic EDBG/CMSIS-DAP protocol, as an Avr8Interface.
     *
     * See the "AVR8 Generic Protocol" section in the DS50002630A document by Microchip, for more information on the
     * protocol.
     *
     * This implementation should work with any Microchip EDBG based CMSIS-DAP debug tool (such as the Atmel-ICE,
     * Power Debugger and the MPLAB SNAP debugger (in "AVR mode")).
     */
    class EdbgAvr8Interface: public TargetInterfaces::Microchip::Avr::Avr8::Avr8Interface
    {
    public:
        explicit EdbgAvr8Interface(EdbgInterface& edbgInterface)
        : edbgInterface(edbgInterface) {};

        /**
         * Some EDBG devices don't seem to operate correctly when actioning the masked memory read EDBG command. The
         * data returned in response to the command appears to be completely incorrect. This appears to only occur
         * with the MPLAB Snap device.
         *
         * Setting this flag to true will disable the EdbgAvr8Interface driver's use of the masked memory read command.
         * The driver will perform the masking itself, and then issue standard read memory commands. See the
         * implementation of EdbgAvr8Interface::readMemory() for more.
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
         * To address this, debug tool drivers can set a hard limit on the number of bytes this EdbgAvr8Interface instance
         * will attempt to access in a single request, using the EdbgAvr8Interface::setMaximumMemoryAccessSizePerRequest()
         * function.
         *
         * This limit will be enforced in all forms of memory access on the AVR8 target, including register access.
         *
         * @TODO: Enforce the limit on memory writes.
         *
         * @param maximumSize
         */
        void setMaximumMemoryAccessSizePerRequest(std::uint32_t maximumSize) {
            this->maximumMemoryAccessSizePerRequest = maximumSize;
        }

        /*
         * The public methods below implement the interface defined by the Avr8Interface class.
         * See the comments in that class for more info on the expected behaviour of each method.
         */

        /**
         * As already mentioned in numerous comments above, the EdbgAvr8Interface requires some configuration from
         * the user. This is supplied via the user's Bloom configuration.
         *
         * @param targetConfig
         */
        void configure(const TargetConfig& targetConfig) override;

        /**
         * Configures the target family. For some physical interfaces, the target family is required in order
         * properly configure the EDBG tool. See EdbgAvr8Interface::resolveConfigVariant() for more.
         *
         * @param family
         */
        void setFamily(Targets::Microchip::Avr::Avr8Bit::Family family) override {
            this->family = family;
        }

        /**
         * Accepts target parameters from the AVR8 target instance and sends the necessary target parameters to the
         * debug tool.
         *
         * @param config
         */
        void setTargetParameters(const Targets::Microchip::Avr::Avr8Bit::TargetParameters& config) override;

        /**
         * Initialises the AVR8 Generic protocol interface by setting the appropriate parameters on the debug tool.
         */
        void init() override;

        /**
         * Issues the "stop" command to the debug tool, halting target execution.
         */
        void stop() override;

        /**
         * Issues the "run" command to the debug tool, resuming execution on the target.
         */
        void run() override;

        /**
         * Issues the "run to" command to the debug tool, resuming execution on the target, up to a specific byte
         * address. The target will dispatch an AVR BREAK event once it reaches the specified address.
         *
         * @param address
         *  The (byte) address to run to.
         */
        void runTo(std::uint32_t address) override;

        /**
         * Issues the "step" command to the debug tool, stepping the execution on the target. The stepping can be
         * configured to step in, out or over. But currently we only support stepping in. The target will dispatch
         * an AVR BREAK event once it reaches the next instruction.
         */
        void step() override;

        /**
         * Issues the "reset" command to the debug tool, resetting target execution.
         */
        void reset() override;

        /**
         * Activates the physical interface and starts a debug session on the target (via attach()).
         */
        void activate() override;

        /**
         * Terminates any active debug session on the target and severs the connection between the debug tool and
         * the target (by deactivating the physical interface).
         */
        void deactivate() override;

        /**
         * Issues the "PC Read" command to the debug tool, to extract the current program counter.
         *
         * @return
         */
        std::uint32_t getProgramCounter() override;

        /**
         * Issues the "PC Write" command to the debug tool, setting the program counter on the target.
         *
         * @param programCounter
         *  The byte address to set as the program counter.
         */
        void setProgramCounter(std::uint32_t programCounter) override;

        /**
         * Issues the "Get ID" command to the debug tool, to extract the signature from the target.
         *
         * @return
         */
        Targets::Microchip::Avr::TargetSignature getDeviceId() override;

        /**
         * Issues the "Software Breakpoint Set" command to the debug tool, setting a software breakpoint at the given
         * byte address.
         *
         * @param address
         *  The byte address to position the breakpoint.
         */
        void setBreakpoint(std::uint32_t address) override;

        /**
         * Issues the "Software Breakpoint Clear" command to the debug tool, clearing any breakpoint at the given
         * byte address.
         *
         * @param address
         *  The byte address of the breakpoint to clear.
         */
        void clearBreakpoint(std::uint32_t address) override;

        /**
         * Issues the "Software Breakpoint Clear All" command to the debug tool, clearing all software breakpoints
         * that were set *in the current debug session*.
         *
         * If the debug session ended before any of the set breakpoints were cleared, this will *not* clear them.
         */
        void clearAllBreakpoints() override;

        /**
         * Reads registers from the target.
         *
         * @param descriptors
         * @return
         */
        Targets::TargetRegisters readRegisters(const Targets::TargetRegisterDescriptors& descriptors) override;

        /**
         * Writes registers to target.
         *
         * @param registers
         */
        void writeRegisters(const Targets::TargetRegisters& registers) override;

        /**
         * This is an overloaded method.
         *
         * Resolves the correct Avr8MemoryType from the given TargetMemoryType and calls readMemory().
         *
         * @param memoryType
         * @param startAddress
         * @param bytes
         * @return
         */
        Targets::TargetMemoryBuffer readMemory(
            Targets::TargetMemoryType memoryType,
            std::uint32_t startAddress,
            std::uint32_t bytes
        ) override;

        /**
         * This is an overloaded method.
         *
         * Resolves the correct Avr8MemoryType from the given TargetMemoryType and calls writeMemory().
         *
         * @param memoryType
         * @param startAddress
         * @param buffer
         */
        void writeMemory(
            Targets::TargetMemoryType memoryType,
            std::uint32_t startAddress,
            const Targets::TargetMemoryBuffer& buffer
        ) override;

        /**
         * Returns the current state of the target.
         *
         * @return
         */
        Targets::TargetState getTargetState() override;

    private:
        /**
         * The AVR8 Generic protocol is a sub-protocol of the EDBG AVR protocol, which is served via CMSIS-DAP vendor
         * commands.
         *
         * Every EDBG based debug tool that utilises this implementation must provide access to its EDBG interface.
         */
        EdbgInterface& edbgInterface;

        /**
         * The target family is taken into account when configuring the AVR8 Generic protocol on the EDBG device.
         *
         * We use this to determine which config variant to select.
         * See EdbgAvr8Interface::resolveConfigVariant() for more.
         */
        std::optional<Targets::Microchip::Avr::Avr8Bit::Family> family;

        /**
         * The AVR8 Generic protocol provides two functions: Debugging and programming. The desired function must be
         * configured via the setting of the "AVR8_CONFIG_FUNCTION" parameter.
         */
        Avr8ConfigFunction configFunction = Avr8ConfigFunction::DEBUGGING;

        /**
         * Configuring of the AVR8 Generic protocol depends on some characteristics of the target.
         * The "AVR8_CONFIG_VARIANT" parameter allows us to determine which target parameters are required by the
         * debug tool.
         */
        Avr8ConfigVariant configVariant = Avr8ConfigVariant::NONE;

        /**
         * Currently, the AVR8 Generic protocol supports 4 physical interfaces: debugWire, JTAG, PDI and UPDI.
         * The desired physical interface must be selected by setting the "AVR8_PHY_PHYSICAL" parameter.
         */
        Targets::Microchip::Avr::Avr8Bit::PhysicalInterface physicalInterface =
            Targets::Microchip::Avr::Avr8Bit::PhysicalInterface::DEBUG_WIRE;

        /**
         * EDBG-based debug tools require target specific parameters such as memory locations, page sizes and
         * register addresses. It is the AVR8 target's responsibility to obtain the required information and pass it
         * to the Avr8Interface. See Avr8::getTargetParameters() and Avr8::postPromotionConfigure().
         *
         * For the EdbgAvr8Interface, we send the required parameters to the debug tool immediately upon receiving
         * them. See EdbgAvr8Interface::setTargetParameters().
         */
        Targets::Microchip::Avr::Avr8Bit::TargetParameters targetParameters;

        /**
         * See the comment for EdbgAvr8Interface::setAvoidMaskedMemoryRead().
         */
        bool avoidMaskedMemoryRead = false;

        /**
         * See the comment for EdbgAvr8Interface::setMaximumMemoryAccessSizePerRequest().
         */
        std::optional<std::uint32_t> maximumMemoryAccessSizePerRequest;

        /**
         * We keep record of the current target state for caching purposes. We'll only refresh the target state if the
         * target is running. If it has already stopped, then we assume it cannot transition to a running state without
         * an instruction from us.
         *
         * @TODO: Review this. Is the above assumption correct? Always? Explore the option of polling the target state.
         */
        Targets::TargetState targetState = Targets::TargetState::UNKNOWN;

        /**
         * Upon configuration, the physical interface must be activated on the debug tool. We keep record of this to
         * assist in our decision to deactivate the physical interface, when deactivate() is called.
         */
        bool physicalInterfaceActivated = false;

        /**
         * As with activating the physical interface, we are required to issue the "attach" command to the debug
         * tool, in order to start a debug session in the target.
         */
        bool targetAttached = false;

        /**
         * Because the debugWire module requires control of the reset pin on the target, enabling this module will
         * effectively mean losing control of the reset pin. This means users won't be able to use other
         * interfaces that require access to the reset pin, such as ISP, until the debugWire module is disabled.
         *
         * The AVR8 Generic protocol provides a function for temporarily disabling the debugWire module on the target.
         * This doesn't change the DWEN fuse and its affect is only temporary - the debugWire module will be
         * reactivated upon the user cycling the power to the target.
         *
         * Bloom is able to temporarily disable the debugWire module, automatically, upon deactivating of the
         * target (which usually occurs after a debug session has ended). This allows users to program the target via
         * ISP, after they've finished a debug session. After programming the target, the user will need to cycle the
         * target power before Bloom can gain access for another debug session.
         *
         * NOTE: ATM, this isn't actually used anywhere. It will always be false. We may include this as a target
         * config parameter, but we first need to consider the actual demand. AVRDUDE used to handle the disabling of
         * the debugWire module, but a recent update resulted in it failing to do so. And that was some time ago, so
         * we need to check if AVRDUDE and other programming software (MPLAB IPE) can handle the disabling of the
         * debugWire module. If not, then we can come back to this.
         *
         * See disableDebugWire() method below.
         */
        bool disableDebugWireOnDeactivate = false;

        /**
         * Users are required to set their desired physical interface in their Bloom configuration. This would take
         * the form of a string, so we map the available options to the appropriate enums.
         */
        static inline auto getPhysicalInterfacesByName() {
            using Targets::Microchip::Avr::Avr8Bit::PhysicalInterface;

            return std::map<std::string, PhysicalInterface>({
                {"debugwire", PhysicalInterface::DEBUG_WIRE},
                {"debug-wire", PhysicalInterface::DEBUG_WIRE},
                {"pdi", PhysicalInterface::PDI},
                {"jtag", PhysicalInterface::JTAG},
                {"updi", PhysicalInterface::UPDI},
            });
        };

        /**
         * This mapping allows us to determine which config variant to select, based on the target family and the
         * selected physical interface.
         */
        static inline auto getConfigVariantsByFamilyAndPhysicalInterface() {
            using Targets::Microchip::Avr::Avr8Bit::Family;
            using Targets::Microchip::Avr::Avr8Bit::PhysicalInterface;
            return std::map<Family, std::map<PhysicalInterface, Avr8ConfigVariant>>({
                {
                    Family::MEGA,
                    {
                        {PhysicalInterface::JTAG, Avr8ConfigVariant::MEGAJTAG},
                        {PhysicalInterface::DEBUG_WIRE, Avr8ConfigVariant::DEBUG_WIRE},
                        {PhysicalInterface::UPDI, Avr8ConfigVariant::UPDI},
                    }
                },
                {
                    Family::TINY,
                    {
                        {PhysicalInterface::JTAG, Avr8ConfigVariant::MEGAJTAG},
                        {PhysicalInterface::DEBUG_WIRE, Avr8ConfigVariant::DEBUG_WIRE},
                        {PhysicalInterface::UPDI, Avr8ConfigVariant::UPDI},
                    }
                },
                {
                    Family::XMEGA,
                    {
                        {PhysicalInterface::JTAG, Avr8ConfigVariant::XMEGA},
                        {PhysicalInterface::PDI, Avr8ConfigVariant::XMEGA},
                    }
                },
                {
                    Family::DA,
                    {
                        {PhysicalInterface::UPDI, Avr8ConfigVariant::UPDI},
                    }
                },
                {
                    Family::DB,
                    {
                        {PhysicalInterface::UPDI, Avr8ConfigVariant::UPDI},
                    }
                },
                {
                    Family::DD,
                    {
                        {PhysicalInterface::UPDI, Avr8ConfigVariant::UPDI},
                    }
                },
            });
        };

        /**
         * Will attempt to resolve the config variant with the information currently held.
         *
         * @return
         */
        std::optional<Avr8ConfigVariant> resolveConfigVariant() {
            if (this->family.has_value()) {
                auto configVariantsByFamily = EdbgAvr8Interface::getConfigVariantsByFamilyAndPhysicalInterface();

                if (configVariantsByFamily.contains(this->family.value())) {
                    auto configVariantsByPhysicalInterface = configVariantsByFamily
                        .at(this->family.value());

                    if (configVariantsByPhysicalInterface.contains(this->physicalInterface)) {
                        return configVariantsByPhysicalInterface.at(this->physicalInterface);
                    }
                }

            } else {
                /*
                 * If there is no family set, we may be able to resort to a simpler mapping of physical interfaces
                 * to config variants. But this will only work if the selected physical interface is *NOT* JTAG.
                 *
                 * This is because JTAG is the only physical interface that could map to two different config
                 * variants (MEGAJTAG and XMEGA). The only way we can figure out which config variant to use is if we
                 * know the target family.
                 *
                 * This is why we don't allow users to use ambiguous target names (such as the generic "avr8" target
                 * name), when using the JTAG physical interface. We won't be able to resolve the correct target
                 * variant. Users are required to specify the exact target name in their config, when using the JTAG
                 * physical interface. That way, this->family will be set by the time resolveConfigVariant() is called.
                 */
                using Targets::Microchip::Avr::Avr8Bit::PhysicalInterface;
                static std::map<PhysicalInterface, Avr8ConfigVariant> physicalInterfacesToConfigVariants = {
                    {PhysicalInterface::DEBUG_WIRE, Avr8ConfigVariant::DEBUG_WIRE},
                    {PhysicalInterface::PDI, Avr8ConfigVariant::XMEGA},
                    {PhysicalInterface::UPDI, Avr8ConfigVariant::UPDI},
                };

                if (physicalInterfacesToConfigVariants.contains(this->physicalInterface)) {
                    return physicalInterfacesToConfigVariants.at(this->physicalInterface);
                }
            }

            return std::nullopt;
        }

        /**
         * Sets an AVR8 parameter on the debug tool. See the Avr8EdbgParameters class and protocol documentation
         * for more on available parameters.
         *
         * @param parameter
         * @param value
         */
        void setParameter(const Avr8EdbgParameter& parameter, const std::vector<unsigned char>& value);

        /**
         * Overload for setting parameters with single byte values.
         *
         * @param parameter
         * @param value
         */
        void setParameter(const Avr8EdbgParameter& parameter, std::uint8_t value) {
            this->setParameter(parameter, std::vector<unsigned char>(1, value));
        }

        /**
         * Overload for setting parameters with four byte integer values.
         *
         * @param parameter
         * @param value
         */
        void setParameter(const Avr8EdbgParameter& parameter, std::uint32_t value) {
            auto paramValue = std::vector<unsigned char>(4);
            paramValue[0] = static_cast<unsigned char>(value);
            paramValue[1] = static_cast<unsigned char>(value >> 8);
            paramValue[2] = static_cast<unsigned char>(value >> 16);
            paramValue[3] = static_cast<unsigned char>(value >> 24);

            this->setParameter(parameter, paramValue);
        }

        /**
         * Overload for setting parameters with two byte integer values.
         *
         * @param parameter
         * @param value
         */
        void setParameter(const Avr8EdbgParameter& parameter, std::uint16_t value) {
            auto paramValue = std::vector<unsigned char>(2);
            paramValue[0] = static_cast<unsigned char>(value);
            paramValue[1] = static_cast<unsigned char>(value >> 8);

            this->setParameter(parameter, paramValue);
        }

        /**
         * Fetches an AV8 parameter from the debug tool.
         *
         * @param parameter
         * @param size
         * @return
         */
        std::vector<unsigned char> getParameter(const Avr8EdbgParameter& parameter, std::uint8_t size);

        /**
         * EDBG-based debug tools require target specific parameters such as memory locations, page sizes and
         * register addresses. These parameters can be sent to the tool before and during a session.
         *
         * What parameters we need to send depend on the physical interface (and config variant) selected by the user.
         * For target parameters, the address (ID) of the parameter also varies across config variants. This is why
         * we sometimes have separate parameters for sending the same data, where they differ only in parameter IDs
         * (and sometimes size constraints). For example, the Avr8EdbgParameters::FLASH_PAGE_BYTES parameter is used
         * to specify the size of a single page in flash memory. The parameter is assigned an address (ID) of 0x00. But
         * the Avr8EdbgParameters::DEVICE_XMEGA_FLASH_PAGE_BYTES parameter is used to send the same data (flash page
         * size), but only for sessions with the PDI physical interface. The address is 0x26.
         *
         * - The setDebugWireAndJtagParameters() function sends the required target parameters for debugWire and JTAG
         *   sessions. Both sessions are covered in a single function because they require the same parameters.
         * - The setPdiParameters() function sends the required target parameters for PDI sessions.
         * - The setUpdiParameters() function sends the required target parameters for UPDI sessions.
         */
        void setDebugWireAndJtagParameters();
        void setPdiParameters();
        void setUpdiParameters();

        /**
         * Sends the "Activate Physical" command to the debug tool, activating the physical interface and thus enabling
         * communication between the debug tool and the target.
         *
         * @param applyExternalReset
         */
        void activatePhysical(bool applyExternalReset = false);

        /**
         * Sends the "Deactivate Physical" command to the debug tool, which will result in the connection between the
         * debug tool and the target being severed.
         */
        void deactivatePhysical();

        /**
         * Sends the "Attach" command to the debug tool, which starts a debug session on the target.
         */
        void attach();

        /**
         * Sends the "Detach" command to the debug tool, which terminates any active debug session on the target.
         */
        void detach();

        /**
         * Fetches any queued events belonging to the AVR8 Generic protocol (such as target break events).
         *
         * @return
         */
        std::unique_ptr<AvrEvent> getAvrEvent();

        /**
         * Clears (discards) any queued AVR8 Generic protocol events on the debug tool.
         */
        void clearEvents();

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
            std::uint32_t startAddress,
            std::uint32_t bytes,
            const std::set<std::uint32_t>& excludedAddresses = {}
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
        void writeMemory(Avr8MemoryType type, std::uint32_t address, const Targets::TargetMemoryBuffer& buffer);

        /**
         * Fetches the current target state.
         *
         * This currently uses AVR BREAK events to determine if a target has stopped. The lack of any
         * queued BREAK events leads to the assumption that the target is still running.
         */
        void refreshTargetState();

        /**
         * Temporarily disables the debugWire module on the target. This does not affect the DWEN fuse. The module
         * will be reactivated upon the cycling of the target power.
         */
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
        std::unique_ptr<AvrEventType> waitForAvrEvent(int maximumAttempts = 20) {
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

                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                attemptCount++;
            }

            return nullptr;
        }

        /**
         * Waits for an AVR BREAK event.
         *
         * This simply wraps a call to waitForAvrEvent<BreakEvent>(). An exception will be thrown if the call doesn't
         * return a BreakEvent.
         *
         * This should only be used when a BreakEvent is always expected.
         */
        void waitForStoppedEvent();
    };
}
