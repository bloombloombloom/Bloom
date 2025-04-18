#include "TargetControllerService.hpp"

// Commands
#include "src/TargetController/Commands/StartAtomicSession.hpp"
#include "src/TargetController/Commands/EndAtomicSession.hpp"
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
#include "src/TargetController/Commands/SetProgramBreakpointAnyType.hpp"
#include "src/TargetController/Commands/RemoveProgramBreakpoint.hpp"
#include "src/TargetController/Commands/SetTargetProgramCounter.hpp"
#include "src/TargetController/Commands/SetTargetStackPointer.hpp"
#include "src/TargetController/Commands/GetTargetGpioPadStates.hpp"
#include "src/TargetController/Commands/SetTargetGpioPadState.hpp"
#include "src/TargetController/Commands/GetTargetStackPointer.hpp"
#include "src/TargetController/Commands/GetTargetProgramCounter.hpp"
#include "src/TargetController/Commands/EnableProgrammingMode.hpp"
#include "src/TargetController/Commands/DisableProgrammingMode.hpp"
#include "src/TargetController/Commands/Shutdown.hpp"
#include "src/TargetController/Commands/GetTargetPassthroughHelpText.hpp"
#include "src/TargetController/Commands/InvokeTargetPassthroughCommand.hpp"

#include "src/Exceptions/Exception.hpp"

namespace Services
{
    using TargetController::Commands::StartAtomicSession;
    using TargetController::Commands::EndAtomicSession;
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
    using TargetController::Commands::SetProgramBreakpointAnyType;
    using TargetController::Commands::RemoveProgramBreakpoint;
    using TargetController::Commands::SetTargetProgramCounter;
    using TargetController::Commands::SetTargetStackPointer;
    using TargetController::Commands::GetTargetGpioPadStates;
    using TargetController::Commands::SetTargetGpioPadState;
    using TargetController::Commands::GetTargetStackPointer;
    using TargetController::Commands::GetTargetProgramCounter;
    using TargetController::Commands::EnableProgrammingMode;
    using TargetController::Commands::DisableProgrammingMode;
    using TargetController::Commands::Shutdown;
    using TargetController::Commands::GetTargetPassthroughHelpText;
    using TargetController::Commands::InvokeTargetPassthroughCommand;

    using Targets::TargetDescriptor;
    using Targets::TargetState;

    using Targets::TargetRegisterDescriptor;
    using Targets::TargetRegisterDescriptors;
    using Targets::TargetRegisterDescriptorAndValuePairs;

    using Targets::TargetAddressSpaceDescriptor;
    using Targets::TargetMemorySegmentDescriptor;
    using Targets::TargetMemoryAddress;
    using Targets::TargetMemorySize;
    using Targets::TargetMemoryAddressRange;
    using Targets::TargetMemoryBuffer;
    using Targets::TargetMemoryBufferSpan;
    using Targets::TargetStackPointer;

    using Targets::TargetPinoutDescriptor;
    using Targets::TargetPinDescriptor;
    using Targets::TargetGpioPadState;
    using Targets::TargetGpioPadDescriptorAndStatePairs;

    TargetControllerService::AtomicSession::AtomicSession(TargetControllerService& targetControllerService)
        : targetControllerService(targetControllerService)
    {
        if (this->targetControllerService.activeAtomicSessionId.has_value()) {
            throw Exceptions::Exception("Atomic session already active in TargetControllerService instance.");
        }

        this->sessionId = this->targetControllerService.startAtomicSession();
        this->targetControllerService.activeAtomicSessionId = this->sessionId;
    }

    TargetControllerService::AtomicSession::~AtomicSession() {
        try {
            this->targetControllerService.endAtomicSession(this->sessionId);

        } catch (const std::exception& exception) {
            Logger::error(
                "Failed to end atomic session (ID: " + std::to_string(this->sessionId) + ") - "
                    + std::string{exception.what()}
            );
        }

        if (
            this->targetControllerService.activeAtomicSessionId.has_value()
            && this->targetControllerService.activeAtomicSessionId == this->sessionId
        ) {
            this->targetControllerService.activeAtomicSessionId.reset();
        }
    }

    const Targets::TargetDescriptor& TargetControllerService::getTargetDescriptor() const {
        return this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<GetTargetDescriptor>(),
            this->defaultTimeout,
            this->activeAtomicSessionId
        )->targetDescriptor;
    }

    const TargetState& TargetControllerService::getTargetState() const {
        return this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<GetTargetState>(),
            this->defaultTimeout,
            this->activeAtomicSessionId
        )->targetState;
    }

    void TargetControllerService::stopTargetExecution() const {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<StopTargetExecution>(),
            this->defaultTimeout,
            this->activeAtomicSessionId
        );
    }

    void TargetControllerService::resumeTargetExecution() const {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<ResumeTargetExecution>(),
            this->defaultTimeout,
            this->activeAtomicSessionId
        );
    }

    void TargetControllerService::stepTargetExecution() const {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<StepTargetExecution>(),
            this->defaultTimeout,
            this->activeAtomicSessionId
        );
    }

    TargetRegisterDescriptorAndValuePairs TargetControllerService::readRegisters(
        const TargetRegisterDescriptors& descriptors
    ) const {
        return this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<ReadTargetRegisters>(descriptors),
            this->defaultTimeout,
            this->activeAtomicSessionId
        )->registers;
    }

    TargetMemoryBuffer TargetControllerService::readRegister(const TargetRegisterDescriptor& descriptor) const {
        return this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<ReadTargetRegisters>(TargetRegisterDescriptors{&descriptor}),
            this->defaultTimeout,
            this->activeAtomicSessionId
        )->registers.at(0).second;
    }

    void TargetControllerService::writeRegisters(const TargetRegisterDescriptorAndValuePairs& registers) const {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<WriteTargetRegisters>(registers),
            this->defaultTimeout,
            this->activeAtomicSessionId
        );
    }

    void TargetControllerService::writeRegister(
        const TargetRegisterDescriptor& descriptor,
        const TargetMemoryBuffer& value
    ) const {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<WriteTargetRegisters>(TargetRegisterDescriptorAndValuePairs{{descriptor, value}}),
            this->defaultTimeout,
            this->activeAtomicSessionId
        );
    }

    TargetMemoryBuffer TargetControllerService::readMemory(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        TargetMemoryAddress startAddress,
        TargetMemorySize bytes,
        bool bypassCache,
        const std::set<TargetMemoryAddressRange>& excludedAddressRanges
    ) const {
        return this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<ReadTargetMemory>(
                addressSpaceDescriptor,
                memorySegmentDescriptor,
                startAddress,
                bytes,
                bypassCache,
                excludedAddressRanges
            ),
            this->defaultTimeout,
            this->activeAtomicSessionId
        )->data;
    }

    void TargetControllerService::writeMemory(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        TargetMemoryAddress startAddress,
        TargetMemoryBufferSpan buffer
    ) const {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<WriteTargetMemory>(
                addressSpaceDescriptor,
                memorySegmentDescriptor,
                startAddress,
                TargetMemoryBuffer{buffer.begin(), buffer.end()}
            ),
            this->defaultTimeout,
            this->activeAtomicSessionId
        );
    }

    void TargetControllerService::eraseMemory(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor
    ) const {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<EraseTargetMemory>(addressSpaceDescriptor, memorySegmentDescriptor),
            this->defaultTimeout,
            this->activeAtomicSessionId
        );
    }

    Targets::TargetProgramBreakpoint TargetControllerService::setProgramBreakpointAnyType(
        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        Targets::TargetMemoryAddress address,
        Targets::TargetMemorySize size
    ) const {
        return this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<SetProgramBreakpointAnyType>(
                addressSpaceDescriptor,
                memorySegmentDescriptor,
                address,
                size
            ),
            this->defaultTimeout,
            this->activeAtomicSessionId
        )->breakpoint;
    }

    void TargetControllerService::removeProgramBreakpoint(const Targets::TargetProgramBreakpoint& breakpoint) const {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<RemoveProgramBreakpoint>(breakpoint),
            this->defaultTimeout,
            this->activeAtomicSessionId
        );
    }

    TargetMemoryAddress TargetControllerService::getProgramCounter() const {
        return this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<GetTargetProgramCounter>(),
            this->defaultTimeout,
            this->activeAtomicSessionId
        )->programCounter;
    }

    void TargetControllerService::setProgramCounter(TargetMemoryAddress address) const {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<SetTargetProgramCounter>(address),
            this->defaultTimeout,
            this->activeAtomicSessionId
        );
    }

    TargetGpioPadDescriptorAndStatePairs TargetControllerService::getGpioPadStates(
        const Targets::TargetPadDescriptors& padDescriptors
    ) const {
        return this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<GetTargetGpioPadStates>(padDescriptors),
            this->defaultTimeout,
            this->activeAtomicSessionId
        )->gpioPadStates;
    }

    void TargetControllerService::setGpioPadState(
        const Targets::TargetPadDescriptor& padDescriptor,
        const TargetGpioPadState& state
    ) const {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<SetTargetGpioPadState>(padDescriptor, state),
            this->defaultTimeout,
            this->activeAtomicSessionId
        );
    }

    TargetStackPointer TargetControllerService::getStackPointer() const {
        return this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<GetTargetStackPointer>(),
            this->defaultTimeout,
            this->activeAtomicSessionId
        )->stackPointer;
    }

    void TargetControllerService::setStackPointer(TargetStackPointer stackPointer) const {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<SetTargetStackPointer>(stackPointer),
            this->defaultTimeout,
            this->activeAtomicSessionId
        );
    }

    void TargetControllerService::resetTarget() const {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<ResetTarget>(),
            this->defaultTimeout,
            this->activeAtomicSessionId
        );
    }

    void TargetControllerService::enableProgrammingMode() const {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<EnableProgrammingMode>(),
            this->defaultTimeout,
            this->activeAtomicSessionId
        );
    }

    void TargetControllerService::disableProgrammingMode() const {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<DisableProgrammingMode>(),
            this->defaultTimeout,
            this->activeAtomicSessionId
        );
    }

    void TargetControllerService::shutdown() const {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<Shutdown>(),
            this->defaultTimeout,
            this->activeAtomicSessionId
        );
    }

    std::string TargetControllerService::getTargetPassthroughHelpText() const {
        return this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<GetTargetPassthroughHelpText>(),
            this->defaultTimeout,
            this->activeAtomicSessionId
        )->text;
    }

    std::optional<Targets::PassthroughResponse> TargetControllerService::invokeTargetPassthroughCommand(
        Targets::PassthroughCommand&& command
    ) const {
        return this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<InvokeTargetPassthroughCommand>(std::move(command)),
            this->defaultTimeout,
            this->activeAtomicSessionId
        )->response;
    }

    TargetControllerService::AtomicSession TargetControllerService::makeAtomicSession() {
        return AtomicSession{*this};
    }

    TargetController::AtomicSessionIdType TargetControllerService::startAtomicSession() {
        return this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<StartAtomicSession>(),
            this->defaultTimeout,
            this->activeAtomicSessionId
        )->sessionId;
    }

    void TargetControllerService::endAtomicSession(TargetController::AtomicSessionIdType sessionId) {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<EndAtomicSession>(sessionId),
            this->defaultTimeout,
            this->activeAtomicSessionId
        );
    }
}
