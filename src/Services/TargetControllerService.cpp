#include "TargetControllerService.hpp"

// Commands
#include "src/TargetController/Commands/GetState.hpp"
#include "src/TargetController/Commands/Suspend.hpp"
#include "src/TargetController/Commands/Resume.hpp"
#include "src/TargetController/Commands/GetTargetDescriptor.hpp"
#include "src/TargetController/Commands/GetTargetState.hpp"
#include "src/TargetController/Commands/StopTargetExecution.hpp"
#include "src/TargetController/Commands/ResumeTargetExecution.hpp"
#include "src/TargetController/Commands/ResetTarget.hpp"
#include "src/TargetController/Commands/ReadTargetRegisters.hpp"
#include "src/TargetController/Commands/WriteTargetRegisters.hpp"
#include "src/TargetController/Commands/ReadTargetMemory.hpp"
#include "src/TargetController/Commands/WriteTargetMemory.hpp"
#include "src/TargetController/Commands/EraseTargetMemory.hpp"
#include "src/TargetController/Commands/StepTargetExecution.hpp"
#include "src/TargetController/Commands/SetBreakpoint.hpp"
#include "src/TargetController/Commands/RemoveBreakpoint.hpp"
#include "src/TargetController/Commands/SetTargetProgramCounter.hpp"
#include "src/TargetController/Commands/GetTargetPinStates.hpp"
#include "src/TargetController/Commands/SetTargetPinState.hpp"
#include "src/TargetController/Commands/GetTargetStackPointer.hpp"
#include "src/TargetController/Commands/GetTargetProgramCounter.hpp"
#include "src/TargetController/Commands/EnableProgrammingMode.hpp"
#include "src/TargetController/Commands/DisableProgrammingMode.hpp"

namespace Bloom::Services
{
    using TargetController::Commands::GetState;
    using TargetController::Commands::Suspend;
    using TargetController::Commands::Resume;
    using TargetController::Commands::GetTargetDescriptor;
    using TargetController::Commands::GetTargetState;
    using TargetController::Commands::StopTargetExecution;
    using TargetController::Commands::ResumeTargetExecution;
    using TargetController::Commands::ResetTarget;
    using TargetController::Commands::ReadTargetRegisters;
    using TargetController::Commands::WriteTargetRegisters;
    using TargetController::Commands::ReadTargetMemory;
    using TargetController::Commands::WriteTargetMemory;
    using TargetController::Commands::EraseTargetMemory;
    using TargetController::Commands::StepTargetExecution;
    using TargetController::Commands::SetBreakpoint;
    using TargetController::Commands::RemoveBreakpoint;
    using TargetController::Commands::SetTargetProgramCounter;
    using TargetController::Commands::GetTargetPinStates;
    using TargetController::Commands::SetTargetPinState;
    using TargetController::Commands::GetTargetStackPointer;
    using TargetController::Commands::GetTargetProgramCounter;
    using TargetController::Commands::EnableProgrammingMode;
    using TargetController::Commands::DisableProgrammingMode;

    using TargetController::TargetControllerState;

    using Targets::TargetDescriptor;
    using Targets::TargetState;

    using Targets::TargetRegisters;
    using Targets::TargetRegisterDescriptors;

    using Targets::TargetMemoryType;
    using Targets::TargetMemoryAddress;
    using Targets::TargetMemorySize;
    using Targets::TargetMemoryAddressRange;
    using Targets::TargetMemoryBuffer;
    using Targets::TargetProgramCounter;
    using Targets::TargetStackPointer;

    using Targets::TargetBreakpoint;

    using Targets::TargetPinDescriptor;
    using Targets::TargetPinState;
    using Targets::TargetPinStateMapping;

    TargetControllerState TargetControllerService::getTargetControllerState() const {
        return this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<GetState>(),
            this->defaultTimeout
        )->state;
    }

    bool TargetControllerService::isTargetControllerInService() const noexcept {
        try {
            return this->getTargetControllerState() == TargetControllerState::ACTIVE;

        } catch (...) {
            return false;
        }
    }

    void TargetControllerService::resumeTargetController() const {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<Resume>(),
            this->defaultTimeout
        );
        return;
    }

    void TargetControllerService::suspendTargetController() const {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<Suspend>(),
            this->defaultTimeout
        );
        return;
    }

    const TargetDescriptor& TargetControllerService::getTargetDescriptor() const {
        return this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<GetTargetDescriptor>(),
            this->defaultTimeout
        )->targetDescriptor;
    }

    TargetState TargetControllerService::getTargetState() const {
        return this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<GetTargetState>(),
            this->defaultTimeout
        )->targetState;
    }

    void TargetControllerService::stopTargetExecution() const {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<StopTargetExecution>(),
            this->defaultTimeout
        );
    }

    void TargetControllerService::continueTargetExecution(
        std::optional<TargetMemoryAddress> fromAddress,
        std::optional<Targets::TargetMemoryAddress> toAddress
    ) const {
        auto resumeExecutionCommand = std::make_unique<ResumeTargetExecution>();

        if (fromAddress.has_value()) {
            resumeExecutionCommand->fromAddress = fromAddress.value();
        }

        if (toAddress.has_value()) {
            resumeExecutionCommand->toAddress = toAddress.value();
        }

        this->commandManager.sendCommandAndWaitForResponse(
            std::move(resumeExecutionCommand),
            this->defaultTimeout
        );
    }

    void TargetControllerService::stepTargetExecution(std::optional<TargetMemoryAddress> fromAddress) const {
        auto stepExecutionCommand = std::make_unique<StepTargetExecution>();

        if (fromAddress.has_value()) {
            stepExecutionCommand->fromProgramCounter = fromAddress.value();
        }

        this->commandManager.sendCommandAndWaitForResponse(
            std::move(stepExecutionCommand),
            this->defaultTimeout
        );
    }

    TargetRegisters TargetControllerService::readRegisters(
        const Targets::TargetRegisterDescriptorIds& descriptorIds
    ) const {
        return this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<ReadTargetRegisters>(descriptorIds),
            this->defaultTimeout
        )->registers;
    }

    void TargetControllerService::writeRegisters(const TargetRegisters& registers) const {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<WriteTargetRegisters>(registers),
            this->defaultTimeout
        );
    }

    TargetMemoryBuffer TargetControllerService::readMemory(
        TargetMemoryType memoryType,
        TargetMemoryAddress startAddress,
        TargetMemorySize bytes,
        const std::set<TargetMemoryAddressRange>& excludedAddressRanges
    ) const {
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

    void TargetControllerService::writeMemory(
        TargetMemoryType memoryType,
        TargetMemoryAddress startAddress,
        const TargetMemoryBuffer& buffer
    ) const {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<WriteTargetMemory>(memoryType, startAddress, buffer),
            this->defaultTimeout
        );
    }

    void TargetControllerService::eraseMemory(Targets::TargetMemoryType memoryType) const {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<EraseTargetMemory>(memoryType),
            this->defaultTimeout
        );
    }

    void TargetControllerService::setBreakpoint(TargetBreakpoint breakpoint) const {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<SetBreakpoint>(breakpoint),
            this->defaultTimeout
        );
    }

    void TargetControllerService::removeBreakpoint(TargetBreakpoint breakpoint) const {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<RemoveBreakpoint>(breakpoint),
            this->defaultTimeout
        );
    }

    TargetProgramCounter TargetControllerService::getProgramCounter() const {
        return this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<GetTargetProgramCounter>(),
            this->defaultTimeout
        )->programCounter;
    }

    void TargetControllerService::setProgramCounter(TargetProgramCounter address) const {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<SetTargetProgramCounter>(address),
            this->defaultTimeout
        );
    }

    TargetPinStateMapping TargetControllerService::getPinStates(int variantId) const {
        return this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<GetTargetPinStates>(variantId),
            this->defaultTimeout
        )->pinStatesByNumber;
    }

    void TargetControllerService::setPinState(TargetPinDescriptor pinDescriptor, TargetPinState pinState) const {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<SetTargetPinState>(pinDescriptor, pinState),
            this->defaultTimeout
        );
    }

    TargetStackPointer TargetControllerService::getStackPointer() const {
        return this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<GetTargetStackPointer>(),
            this->defaultTimeout
        )->stackPointer;
    }

    void TargetControllerService::resetTarget() const {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<ResetTarget>(),
            this->defaultTimeout
        );
    }

    void TargetControllerService::enableProgrammingMode() const {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<EnableProgrammingMode>(),
            this->defaultTimeout
        );
    }

    void TargetControllerService::disableProgrammingMode() const {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<DisableProgrammingMode>(),
            this->defaultTimeout
        );
    }
}
