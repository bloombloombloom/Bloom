#include "TargetControllerConsole.hpp"

#include "TargetControllerComponent.hpp"

// Commands
#include "Commands/GetTargetDescriptor.hpp"
#include "Commands/GetTargetState.hpp"
#include "Commands/StopTargetExecution.hpp"
#include "Commands/ResumeTargetExecution.hpp"
#include "Commands/ResetTarget.hpp"
#include "Commands/ReadTargetRegisters.hpp"
#include "Commands/WriteTargetRegisters.hpp"
#include "Commands/ReadTargetMemory.hpp"
#include "Commands/WriteTargetMemory.hpp"
#include "Commands/StepTargetExecution.hpp"
#include "Commands/SetBreakpoint.hpp"
#include "Commands/RemoveBreakpoint.hpp"
#include "Commands/SetTargetProgramCounter.hpp"
#include "Commands/GetTargetPinStates.hpp"
#include "Commands/SetTargetPinState.hpp"
#include "Commands/GetTargetStackPointer.hpp"
#include "Commands/GetTargetProgramCounter.hpp"

namespace Bloom::TargetController
{
    using Commands::GetTargetDescriptor;
    using Commands::GetTargetState;
    using Commands::StopTargetExecution;
    using Commands::ResumeTargetExecution;
    using Commands::ResetTarget;
    using Commands::ReadTargetRegisters;
    using Commands::WriteTargetRegisters;
    using Commands::ReadTargetMemory;
    using Commands::WriteTargetMemory;
    using Commands::StepTargetExecution;
    using Commands::SetBreakpoint;
    using Commands::RemoveBreakpoint;
    using Commands::SetTargetProgramCounter;
    using Commands::GetTargetPinStates;
    using Commands::SetTargetPinState;
    using Commands::GetTargetStackPointer;
    using Commands::GetTargetProgramCounter;

    using Targets::TargetDescriptor;
    using Targets::TargetState;

    using Targets::TargetRegisters;
    using Targets::TargetRegisterDescriptors;

    using Targets::TargetMemoryType;
    using Targets::TargetMemoryAddressRange;
    using Targets::TargetMemoryBuffer;

    using Targets::TargetBreakpoint;

    using Targets::TargetPinDescriptor;
    using Targets::TargetPinState;
    using Targets::TargetPinStateMappingType;

    TargetControllerState TargetControllerConsole::getTargetControllerState() {
        return TargetControllerComponent::getState();
    }

    bool TargetControllerConsole::isTargetControllerInService() noexcept {
        try {
            return this->getTargetControllerState() == TargetControllerState::ACTIVE;

        } catch (...) {
            return false;
        }
    }

    TargetDescriptor TargetControllerConsole::getTargetDescriptor() {
        return this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<GetTargetDescriptor>(),
            this->defaultTimeout
        )->targetDescriptor;
    }

    TargetState TargetControllerConsole::getTargetState() {
        return this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<GetTargetState>(),
            this->defaultTimeout
        )->targetState;
    }

    void TargetControllerConsole::stopTargetExecution() {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<StopTargetExecution>(),
            this->defaultTimeout
        );
    }

    void TargetControllerConsole::continueTargetExecution(std::optional<std::uint32_t> fromAddress) {
        auto resumeExecutionCommand = std::make_unique<ResumeTargetExecution>();

        if (fromAddress.has_value()) {
            resumeExecutionCommand->fromProgramCounter = fromAddress.value();
        }

        this->commandManager.sendCommandAndWaitForResponse(
            std::move(resumeExecutionCommand),
            this->defaultTimeout
        );
    }

    void TargetControllerConsole::stepTargetExecution(std::optional<std::uint32_t> fromAddress) {
        auto stepExecutionCommand = std::make_unique<StepTargetExecution>();

        if (fromAddress.has_value()) {
            stepExecutionCommand->fromProgramCounter = fromAddress.value();
        }

        this->commandManager.sendCommandAndWaitForResponse(
            std::move(stepExecutionCommand),
            this->defaultTimeout
        );
    }

    TargetRegisters TargetControllerConsole::readRegisters(const TargetRegisterDescriptors& descriptors) {
        return this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<ReadTargetRegisters>(descriptors),
            this->defaultTimeout
        )->registers;
    }

    void TargetControllerConsole::writeRegisters(const TargetRegisters& registers) {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<WriteTargetRegisters>(registers),
            this->defaultTimeout
        );
    }

    TargetMemoryBuffer TargetControllerConsole::readMemory(
        TargetMemoryType memoryType,
        std::uint32_t startAddress,
        std::uint32_t bytes,
        const std::set<TargetMemoryAddressRange>& excludedAddressRanges
    ) {
        return this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<ReadTargetMemory>(
                memoryType,
                startAddress,
                bytes,
                excludedAddressRanges
            ),
            this->defaultTimeout
        )->data;
    }

    void TargetControllerConsole::writeMemory(
        TargetMemoryType memoryType,
        std::uint32_t startAddress,
        const TargetMemoryBuffer& buffer
    ) {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<WriteTargetMemory>(memoryType, startAddress, buffer),
            this->defaultTimeout
        );
    }

    void TargetControllerConsole::setBreakpoint(TargetBreakpoint breakpoint) {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<SetBreakpoint>(breakpoint),
            this->defaultTimeout
        );
    }

    void TargetControllerConsole::removeBreakpoint(TargetBreakpoint breakpoint) {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<RemoveBreakpoint>(breakpoint),
            this->defaultTimeout
        );
    }

    std::uint32_t TargetControllerConsole::getProgramCounter() {
        return this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<GetTargetProgramCounter>(),
            this->defaultTimeout
        )->programCounter;
    }

    void TargetControllerConsole::setProgramCounter(std::uint32_t address) {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<SetTargetProgramCounter>(address),
            this->defaultTimeout
        );
    }

    TargetPinStateMappingType TargetControllerConsole::getPinStates(int variantId) {
        return this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<GetTargetPinStates>(variantId),
            this->defaultTimeout
        )->pinStatesByNumber;
    }

    void TargetControllerConsole::setPinState(TargetPinDescriptor pinDescriptor, TargetPinState pinState) {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<SetTargetPinState>(pinDescriptor, pinState),
            this->defaultTimeout
        );
    }

    std::uint32_t TargetControllerConsole::getStackPointer() {
        return this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<GetTargetStackPointer>(),
            this->defaultTimeout
        )->stackPointer;
    }

    void TargetControllerConsole::resetTarget() {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<ResetTarget>(),
            this->defaultTimeout
        );
    }
}
