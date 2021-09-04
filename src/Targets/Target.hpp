#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <set>
#include <map>

#include "src/DebugToolDrivers/DebugTool.hpp"
#include "src/ApplicationConfig.hpp"
#include "TargetDescriptor.hpp"
#include "TargetState.hpp"
#include "TargetRegister.hpp"
#include "TargetMemory.hpp"
#include "TargetBreakpoint.hpp"

namespace Bloom::Targets
{
    /**
     * Abstract class for Targets.
     *
     * All targets supported by Bloom must implement this interface.
     *
     * A single implementation of this interface can represent a single target, or an entire family of targets.
     * For an example, see the Avr8 implementation. The Avr8 target class was written in a way that would allow it to
     * work, to *at least* the point of target promotion, for all AVR8 targets. For more on target promotion, see the
     * Target::promote() method.
     */
    class Target
    {
    protected:
        /**
         * Target related configuration provided by the user. This is passed in via the first stage of target
         * configuration. See Target::preActivationConfigure() for more.
         */
        TargetConfig config;

        bool activated = false;

    public:
        explicit Target() = default;

        bool isActivated() const {
            return this->activated;
        }

        /**
         * There are three stages of configuration for targets.
         *
         * preActivationConfigure() - The first stage is just before target activation (Target::activate() being called).
         * At this point, we will not have interacted with the target in any way. This method should cover any
         * configuration that can be done without the target being activated. It should also cover any configuration
         * that is required in order for us to successfully activate the target. For an example, we use this method in
         * the Avr8 target class to configure the debug tool with the correct physical interface and config variant
         * parameters (taken from the user's settings, via the TargetConfig instance). Without these being configured,
         * the debug tool would not be able to interface with the AVR8 target, and thus target activation would fail.
         *
         * postActivationConfigure() - The second stage is right after target activation (successful invocation of
         * Target::activate()). At this point, we will have established a connection with the target and so interaction
         * with the target is permitted here. We use this method in the Avr8 target class to extract the target signature
         * from the target's memory, which we then use to find & load the correct target description file.
         *
         * postPromotionConfigure() - The final stage of configuration occurs just after the target instance has been
         * promoted to a different class. See the Target::promote() method for more in this.
         *
         * If any of the three configuration methods throw an exception, the exception will be treated as a fatal error.
         * In response, the TargetController will shutdown, along with the rest of Bloom.
         *
         * @param targetConfig
         */
        virtual void preActivationConfigure(const TargetConfig& targetConfig) {
            this->config = targetConfig;
        };
        virtual void postActivationConfigure() = 0;
        virtual void postPromotionConfigure() = 0;

        /**
         * This method should attempt to establish a connection with the target, and put it in a state where debugging
         * can be performed. This method will be called after Target::preActivationConfigure().
         *
         * If an exception is thrown from this method, the TargetController will treat it as a fatal error, and thus
         * will shutdown, along with the rest of Bloom.
         */
        virtual void activate() = 0;

        /**
         * Should pull the target out of the debugging state and disconnect from it.
         *
         * This is typically called on TargetController shutdown, but keep in mind that it's called regardless of
         * whether or not Target::activate() was previously called. In other words, the TargetController will always
         * call this method on shutdown, even if the TargetController did not call Target::activate() before it began
         * shutting down. The reason behind this is to give the target a chance to deactivate in cases where the call
         * to Target::activate() failed and thus triggered a shutdown (via an exception being thrown from
         * Target::activate()).
         */
        virtual void deactivate() = 0;

        /**
         * Should check if the given debugTool is compatible with the target. Returning false in this method will
         * prevent Bloom from attempting to use the selected debug tool with the selected target. An InvalidConfig
         * exception will be raised and Bloom will shutdown.
         *
         * For AVR8 targets, we simply check if the debug tool returns a valid Avr8Interface
         * (via DebugTool::getAvr8Interface()). If it fails to do so, it would mean that the debug tool, or more so our
         * debug tool driver, does not support AVR8 targets.
         *
         * @param debugTool
         *
         * @return
         */
        virtual bool isDebugToolSupported(DebugTool* debugTool) = 0;

        /**
         * Assuming the Target::isDebugToolSupported() check passed, this method will be called shortly after, by the
         * TargetController.
         *
         * @param debugTool
         */
        virtual void setDebugTool(DebugTool* debugTool) = 0;

        /**
         * Should indicate whether this target class can be promoted to one that better represents the connected
         * target. See Target::promote() for more.
         *
         * @return
         */
        virtual bool supportsPromotion() = 0;

        /**
         * Bloom allows users to select generic targets within their configuration, but this doesn't have to mean we
         * are limited to the generic target class. In some cases, we may want to switch to a target class that is
         * more specific to the connected target. We call this "target promotion". See below for an example.
         *
         * When a user is debugging an AVR8 target, they may not specify the exact name of the target in their project
         * configuration file. Instead, they may select the generic 'avr8' target (which maps to the generic Avr8 target
         * class). In cases like this, the data we have on the target, at the point of startup, is very limited; all we
         * know about the target is that it's part of the AVR8 family. Nothing else. But this is ok, because, when we
         * begin the target configuration and activation process, we are able to learn a lot more about the target.
         * For AVR8 targets, we extract the target signature shortly after activation, and with that signature we find
         * the appropriate target description file, which has all of the information regarding the target that we could
         * possibly need. So, by the time we have activated the target, we will know a lot more about it, and it is at
         * this point, where we may want to promote it to a more specific target class (from the generic Avr8 target
         * class). The generic AVR8 target class will attempt to promote the target to one that is more specific to
         * the target's AVR8 family (ATmega, XMega, Tiny, etc). These classes can then also perform promotion of their
         * own, if required, where they could promote to a class that's not only specific to an AVR8 family, but to a
         * particular target model (for example, a target class that was written specifically for the ATmega328P target).
         *
         * This method should attempt to promote the current target class to one that is more specific to the connected
         * target, with the information it currently holds on the target.
         *
         * If this method fails to promote the target, it should return an std::unique_ptr(nullptr).
         *
         * After activating the target, assuming the first call to Target::supportsPromotion() returns true, the
         * TargetController will enter a loop, where it will repeatedly call this method and update the target
         * instance, until at least one of the following conditions are met:
         *   - The call to Target::supportsPromotion() on the current target instance returns false
         *   - The call to Target::promote() on the current target instance returns an std::unique_ptr(nullptr)
         *   - The call to Target::promote() on the current target instance returns a target class type that is equal
         *     to the type of the current target instance (promotion failed).
         *
         * Once at least one of the above conditions are met, the TargetController will break out of the loop and use
         * the last promoted target instance from there onwards.
         *
         * See TargetController::acquireHardware() for more on this.
         *
         * @return
         */
        virtual std::unique_ptr<Target> promote() = 0;

        virtual std::string getName() const = 0;
        virtual std::string getHumanReadableId() = 0;

        /**
         * Should generate and return a TargetDescriptor for the current target.
         *
         * This is called when a component within Bloom requests the TargetDescriptor from the TargetController.
         * The TargetController will cache this upon the first request. Subsequent requests will be serviced with the
         * cached value.
         *
         * @return
         */
        virtual TargetDescriptor getDescriptor() = 0;

        /**
         * Should resume execution on the target.
         */
        virtual void run() = 0;

        /**
         * Should halt execution on the target.
         */
        virtual void stop() = 0;

        /**
         * Should step execution on the target (instruction step).
         */
        virtual void step() = 0;

        /**
         * Should reset the target.
         */
        virtual void reset() = 0;

        /**
         * Should set a breakpoint on the target, at the given address.
         *
         * @param address
         */
        virtual void setBreakpoint(std::uint32_t address) = 0;

        /**
         * Should remove a breakpoint at the given address.
         *
         * @param address
         */
        virtual void removeBreakpoint(std::uint32_t address) = 0;

        /**
         * Should clear all breakpoints on the target.
         *
         * @TODO: is this still needed? Review
         */
        virtual void clearAllBreakpoints() = 0;

        /**
         * Should update the value of the given registers.
         *
         * @param registers
         */
        virtual void writeRegisters(TargetRegisters registers) = 0;

        /**
         * Should read register values of the registers described by the given descriptors.
         *
         * @param descriptors
         *
         * @return
         */
        virtual TargetRegisters readRegisters(TargetRegisterDescriptors descriptors) = 0;

        /**
         * Should read memory from the target.
         *
         * @param memoryType
         * @param startAddress
         * @param bytes
         *
         * @return
         */
        virtual TargetMemoryBuffer readMemory(TargetMemoryType memoryType, std::uint32_t startAddress, std::uint32_t bytes) = 0;

        /**
         * Should write memory to the target.
         *
         * @param memoryType
         * @param startAddress
         * @param buffer
         */
        virtual void writeMemory(TargetMemoryType memoryType, std::uint32_t startAddress, const TargetMemoryBuffer& buffer) = 0;

        /**
         * Should return the current state of the target.
         *
         * @return
         */
        virtual TargetState getState() = 0;

        /**
         * Should fetch the current program counter value.
         *
         * @return
         */
        virtual std::uint32_t getProgramCounter() = 0;

        /**
         * Should update the program counter on the target.
         *
         * @param programCounter
         */
        virtual void setProgramCounter(std::uint32_t programCounter) = 0;

        /**
         * Should get the current pin states for each pin on the target, mapped by pin number
         *
         * @param variantId
         *
         * @return
         */
        virtual std::map<int, TargetPinState> getPinStates(int variantId) = 0;

        /**
         * Should update the pin state for the given pin, with the given state.
         *
         * @param pinDescriptor
         * @param state
         */
        virtual void setPinState(
            const TargetPinDescriptor& pinDescriptor,
            const TargetPinState& state
        ) = 0;

        /**
         * Should determine whether writing to a certain memory type and address range will affect the target's pin
         * states. This is used by Insight to kick off a pin state update if some other component may have updated the
         * pin states via a memory write to IO port register addresses.
         *
         * @param memoryType
         * @param startAddress
         * @param endAddress
         *
         * @return
         */
        virtual bool memoryAddressRangeClashesWithIoPortRegisters(
            TargetMemoryType memoryType,
            std::uint32_t startAddress,
            std::uint32_t endAddress
        ) = 0;

        virtual ~Target() = default;
    };
}
