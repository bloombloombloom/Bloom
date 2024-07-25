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
#include "src/TargetController/Commands/SetBreakpoint.hpp"
#include "src/TargetController/Commands/RemoveBreakpoint.hpp"
#include "src/TargetController/Commands/SetTargetProgramCounter.hpp"
#include "src/TargetController/Commands/SetTargetStackPointer.hpp"
#include "src/TargetController/Commands/GetTargetGpioPinStates.hpp"
#include "src/TargetController/Commands/SetTargetGpioPinState.hpp"
#include "src/TargetController/Commands/GetTargetStackPointer.hpp"
#include "src/TargetController/Commands/GetTargetProgramCounter.hpp"
#include "src/TargetController/Commands/EnableProgrammingMode.hpp"
#include "src/TargetController/Commands/DisableProgrammingMode.hpp"
#include "src/TargetController/Commands/Shutdown.hpp"

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
    using TargetController::Commands::SetBreakpoint;
    using TargetController::Commands::RemoveBreakpoint;
    using TargetController::Commands::SetTargetProgramCounter;
    using TargetController::Commands::SetTargetStackPointer;
    using TargetController::Commands::GetTargetGpioPinStates;
    using TargetController::Commands::SetTargetGpioPinState;
    using TargetController::Commands::GetTargetStackPointer;
    using TargetController::Commands::GetTargetProgramCounter;
    using TargetController::Commands::EnableProgrammingMode;
    using TargetController::Commands::DisableProgrammingMode;
    using TargetController::Commands::Shutdown;

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
    using Targets::TargetStackPointer;

    using Targets::TargetBreakpoint;

    using Targets::TargetPinoutDescriptor;
    using Targets::TargetPinDescriptor;
    using Targets::TargetGpioPinState;
    using Targets::TargetGpioPinDescriptorAndStatePairs;

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
        Targets::TargetMemoryBuffer&& buffer
    ) const {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<WriteTargetMemory>(
                addressSpaceDescriptor,
                memorySegmentDescriptor,
                startAddress,
                std::move(buffer)
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

    Targets::TargetBreakpoint TargetControllerService::setBreakpoint(
        Targets::TargetMemoryAddress address,
        Targets::TargetBreakpoint::Type preferredType
    ) const {
        return this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<SetBreakpoint>(address, preferredType),
            this->defaultTimeout,
            this->activeAtomicSessionId
        )->breakpoint;
    }

    void TargetControllerService::removeBreakpoint(TargetBreakpoint breakpoint) const {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<RemoveBreakpoint>(breakpoint),
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

    TargetGpioPinDescriptorAndStatePairs TargetControllerService::getGpioPinStates(
        const TargetPinoutDescriptor& pinoutDescriptor
    ) const {
        return this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<GetTargetGpioPinStates>(pinoutDescriptor),
            this->defaultTimeout,
            this->activeAtomicSessionId
        )->gpioPinStates;
    }

    void TargetControllerService::setGpioPinState(
        const TargetPinDescriptor& pinDescriptor,
        const TargetGpioPinState& state
    ) const {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<SetTargetGpioPinState>(pinDescriptor, state),
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
