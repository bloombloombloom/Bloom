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
    class Target
    {
    protected:
        bool activated = false;
        TargetConfig config;

    public:
        explicit Target() {}

        bool getActivated() {
            return this->activated;
        }

        /**
         * There are three stages of configuration for targets.
         *
         * @param targetConfig
         */
        virtual void preActivationConfigure(const TargetConfig& targetConfig) {
            this->config = targetConfig;
        };

        virtual void postActivationConfigure() = 0;
        virtual void postPromotionConfigure() = 0;

        /**
         * Should put the target in a state where debugging can be performed. This would typically involve
         * activating the physical interface, among other things.
         */
        virtual void activate() = 0;

        /**
         * Should pull the target out of the debugging state and reset it.
         */
        virtual void deactivate() = 0;
        virtual void setBreakpoint(std::uint32_t address) = 0;
        virtual void removeBreakpoint(std::uint32_t address) = 0;
        virtual void clearAllBreakpoints() = 0;

        /**
         * Should check if debugTool is compatible with the Target.
         *
         * @param debugTool
         * @return
         */
        virtual bool isDebugToolSupported(DebugTool* debugTool) = 0;

        virtual void setDebugTool(DebugTool* debugTool) = 0;

        virtual bool supportsPromotion() {
            return false;
        }

        virtual std::unique_ptr<Target> promote() = 0;

        virtual std::string getName() const = 0;
        virtual std::string getHumanReadableId() = 0;
        virtual TargetDescriptor getDescriptor() = 0;

        virtual void run() = 0;
        virtual void stop() = 0;
        virtual void step() = 0;
        virtual void reset() = 0;

        virtual TargetRegisters readGeneralPurposeRegisters(std::set<std::size_t> registerIds) = 0;
        virtual void writeRegisters(const TargetRegisters& registers) = 0;
        virtual TargetRegisters readRegisters(const TargetRegisterDescriptors& descriptors) = 0;

        virtual TargetMemoryBuffer readMemory(TargetMemoryType memoryType, std::uint32_t startAddress, std::uint32_t bytes) = 0;
        virtual void writeMemory(TargetMemoryType memoryType, std::uint32_t startAddress, const TargetMemoryBuffer& buffer) = 0;

        virtual TargetState getState() = 0;

        virtual std::uint32_t getProgramCounter() = 0;
        virtual TargetRegister getProgramCounterRegister() = 0;
        virtual TargetRegister getStatusRegister() = 0;
        virtual TargetRegister getStackPointerRegister() = 0;
        virtual void setProgramCounter(std::uint32_t programCounter) = 0;
        virtual std::map<int, TargetPinState> getPinStates(int variantId) = 0;
        virtual void setPinState(
            int variantId,
            const TargetPinDescriptor& pinDescriptor,
            const TargetPinState& state
        ) = 0;

        virtual bool willMemoryWriteAffectIoPorts(
            TargetMemoryType memoryType,
            std::uint32_t startAddress,
            std::uint32_t bytes
        ) = 0;

        virtual ~Target() = default;


    };
}
