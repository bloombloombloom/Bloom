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

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr
{
    inline bool operator==(unsigned char rawId, Avr8ResponseId id) {
        return static_cast<unsigned char>(id) == rawId;
    }

    inline bool operator==(Avr8ResponseId id, unsigned char rawId) {
        return rawId == id;
    }

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
    private:
        /**
         * The AVR8 Generic protocol is a sub-protocol of the EDBG AVR protocol, which is served via CMSIS-DAP vendor
         * commands.
         *
         * Every EDBG based debug tool that utilises this implementation must provide access to its EDBG interface.
         */
        EdbgInterface& edbgInterface;

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
        Avr8ConfigVariant configVariant;

        /**
         * Currently, the AVR8 Generic protocol supports 4 physical interfaces: debugWire, JTAG, PDI and UPDI.
         * The desired physical interface must be selected by setting the "AVR8_PHY_PHYSICAL" parameter.
         */
        Avr8PhysicalInterface physicalInterface;

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
         * This feature can be activated via the user's Bloom configuration.
         *
         * See disableDebugWire() method below.
         */
        bool disableDebugWireOnDeactivate = false;

        /**
         * Users are required to set their desired physical interface in their Bloom configuration. This would take
         * the form of a string, so we map the available options to the appropriate enums.
         */
        static inline std::map<std::string, Avr8PhysicalInterface> physicalInterfacesByName = {
            {"debugwire", Avr8PhysicalInterface::DEBUG_WIRE},
            {"pdi", Avr8PhysicalInterface::PDI},
            {"jtag", Avr8PhysicalInterface::JTAG},
//            {"updi", Avr8PhysicalInterface::PDI_1W}, // Disabled for now - will add support later
        };

        /**
         * Although users can supply the desired config variant via their Bloom configuration, this is not required.
         * This mapping allows us to determine which config variant to select, based on the selected physical
         * interface.
         */
        static inline std::map<Avr8PhysicalInterface, Avr8ConfigVariant> configVariantsByPhysicalInterface = {
            {Avr8PhysicalInterface::DEBUG_WIRE, Avr8ConfigVariant::DEBUG_WIRE},
            {Avr8PhysicalInterface::PDI, Avr8ConfigVariant::XMEGA},
            {Avr8PhysicalInterface::JTAG, Avr8ConfigVariant::MEGAJTAG},
            {Avr8PhysicalInterface::PDI_1W, Avr8ConfigVariant::UPDI},
        };

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
        void setParameter(const Avr8EdbgParameter& parameter, unsigned char value) {
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
         * @param address
         *  The start address (byte address)
         *
         * @param bytes
         *  Number of bytes to access.
         *
         * @return
         */
        Targets::TargetMemoryBuffer readMemory(Avr8MemoryType type, std::uint32_t address, std::uint32_t bytes);

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
        void writeMemory(Avr8MemoryType type, std::uint32_t address, Targets::TargetMemoryBuffer buffer);

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

    public:
        EdbgAvr8Interface(EdbgInterface& edbgInterface)
        : edbgInterface(edbgInterface) {};

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
        virtual void configure(const TargetConfig& targetConfig) override;

        /**
         * Accepts target parameters from the AVR8 target instance and sends the necessary target parameters to the
         * debug tool.
         *
         * @param config
         */
        virtual void setTargetParameters(const Targets::Microchip::Avr::Avr8Bit::TargetParameters& config) override;

        /**
         * Initialises the AVR8 Generic protocol interface by setting the appropriate parameters on the debug tool.
         */
        virtual void init() override;

        /**
         * Issues the "stop" command to the debug tool, halting target execution.
         */
        virtual void stop() override;

        /**
         * Issues the "run" command to the debug tool, resuming execution on the target.
         */
        virtual void run() override;

        /**
         * Issues the "run to" command to the debug tool, resuming execution on the target, up to a specific byte
         * address. The target will dispatch an AVR BREAK event once it reaches the specified address.
         *
         * @param address
         *  The (byte) address to run to.
         */
        virtual void runTo(std::uint32_t address) override;

        /**
         * Issues the "step" command to the debug tool, stepping the execution on the target. The stepping can be
         * configured to step in, out or over. But currently we only support stepping in. The target will dispatch
         * an AVR BREAK event once it reaches the next instruction.
         */
        virtual void step() override;

        /**
         * Issues the "reset" command to the debug tool, resetting target execution.
         */
        virtual void reset() override;

        /**
         * Activates the physical interface and starts a debug session on the target (via attach()).
         */
        virtual void activate() override;

        /**
         * Terminates any active debug session on the target and severs the connection between the debug tool and
         * the target (by deactivating the physical interface).
         */
        virtual void deactivate() override;

        /**
         * Issues the "PC Read" command to the debug tool, to extract the current program counter.
         *
         * @return
         */
        virtual std::uint32_t getProgramCounter() override;

        /**
         * Reads the stack pointer register from the target.
         *
         * @return
         */
        virtual Targets::TargetRegister getStackPointerRegister() override;

        /**
         * Reads the status register from the target.
         *
         * @return
         */
        virtual Targets::TargetRegister getStatusRegister() override;

        /**
         * Updates the stack pointer register on ther target.
         *
         * @param stackPointerRegister
         */
        virtual void setStackPointerRegister(const Targets::TargetRegister& stackPointerRegister) override;

        /**
         * Updates the status register on the target.
         *
         * @param statusRegister
         */
        virtual void setStatusRegister(const Targets::TargetRegister& statusRegister) override;

        /**
         * Issues the "PC Write" command to the debug tool, setting the program counter on the target.
         *
         * @param programCounter
         *  The byte address to set as the program counter.
         */
        virtual void setProgramCounter(std::uint32_t programCounter) override;

        /**
         * Issues the "Get ID" command to the debug tool, to extract the signature from the target.
         *
         * @return
         */
        virtual Targets::Microchip::Avr::TargetSignature getDeviceId() override;

        /**
         * Issues the "Software Breakpoint Set" command to the debug tool, setting a software breakpoint at the given
         * byte address.
         *
         * @param address
         *  The byte address to position the breakpoint.
         */
        virtual void setBreakpoint(std::uint32_t address) override;

        /**
         * Issues the "Software Breakpoint Clear" command to the debug tool, clearing any breakpoint at the given
         * byte address.
         *
         * @param address
         *  The byte address of the breakpoint to clear.
         */
        virtual void clearBreakpoint(std::uint32_t address) override;

        /**
         * Issues the "Software Breakpoint Clear All" command to the debug tool, clearing all software breakpoints
         * that were set *in the current debug session*.
         *
         * If the debug session ended before any of the set breakpoints were cleared, this will *not* clear them.
         */
        virtual void clearAllBreakpoints() override;

        /**
         * Reads gernal purpose registers from the target.
         *
         * @param registerIds
         * @return
         */
        virtual Targets::TargetRegisters readGeneralPurposeRegisters(std::set<std::size_t> registerIds) override;

        /**
         * Writes general purpose registers to target.
         *
         * @param registers
         */
        virtual void writeGeneralPurposeRegisters(const Targets::TargetRegisters& registers) override;

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
        virtual Targets::TargetMemoryBuffer readMemory(
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
        virtual void writeMemory(
            Targets::TargetMemoryType memoryType,
            std::uint32_t startAddress,
            const Targets::TargetMemoryBuffer& buffer
        ) override;

        /**
         * Returns the current state of the target.
         *
         * @return
         */
        virtual Targets::TargetState getTargetState() override;
    };
}