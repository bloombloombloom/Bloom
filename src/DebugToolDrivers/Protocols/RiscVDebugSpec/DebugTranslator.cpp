#include "DebugTranslator.hpp"

#include <string>
#include <thread>
#include <chrono>
#include <limits>
#include <cassert>
#include <algorithm>
#include <thread>

#include "Registers/CpuRegisterNumbers.hpp"
#include "DebugModule/Registers/RegisterAddresses.hpp"
#include "DebugModule/Registers/RegisterAccessControlField.hpp"
#include "DebugModule/Registers/MemoryAccessControlField.hpp"
#include "DebugModule/Registers/AbstractCommandAutoExecuteRegister.hpp"

#include "src/Targets/RiscV/Opcodes/Lb.hpp"
#include "src/Targets/RiscV/Opcodes/Lw.hpp"
#include "src/Targets/RiscV/Opcodes/Sb.hpp"
#include "src/Targets/RiscV/Opcodes/Sw.hpp"
#include "src/Targets/RiscV/Opcodes/Addi.hpp"

#include "TriggerModule/Registers/TriggerSelect.hpp"
#include "TriggerModule/Registers/TriggerInfo.hpp"
#include "TriggerModule/Registers/TriggerData1.hpp"
#include "TriggerModule/Registers/MatchControl.hpp"

#include "src/Helpers/Array.hpp"

#include "src/Exceptions/Exception.hpp"
#include "src/Exceptions/InvalidConfig.hpp"
#include "src/Exceptions/InternalFatalErrorException.hpp"
#include "src/TargetController/Exceptions/TargetFailure.hpp"
#include "src/TargetController/Exceptions/TargetOperationFailure.hpp"
#include "src/Targets/RiscV/Exceptions/IllegalMemoryAccess.hpp"

#include "src/Logger/Logger.hpp"

namespace DebugToolDrivers::Protocols::RiscVDebugSpec
{
    using Registers::DebugControlStatusRegister;

    using DebugModule::Registers::RegisterAddress;
    using DebugModule::Registers::ControlRegister;
    using DebugModule::Registers::StatusRegister;
    using DebugModule::Registers::AbstractControlStatusRegister;
    using DebugModule::Registers::AbstractCommandRegister;
    using DebugModule::Registers::AbstractCommandAutoExecuteRegister;
    using DebugModule::Registers::RegisterAccessControlField;
    using DebugModule::Registers::MemoryAccessControlField;
    using DebugModule::AbstractCommandError;
    using DebugModule::MemoryAccessStrategy;

    using Registers::CpuRegisterNumber;

    using namespace ::Targets::RiscV;

    using ::Targets::TargetExecutionState;
    using ::Targets::TargetMemoryAddress;
    using ::Targets::TargetMemoryAddressRange;
    using ::Targets::TargetMemorySize;
    using ::Targets::TargetMemoryBuffer;
    using ::Targets::TargetMemoryBufferSpan;
    using ::Targets::TargetStackPointer;
    using ::Targets::TargetAddressSpaceDescriptor;
    using ::Targets::TargetMemorySegmentDescriptor;
    using ::Targets::TargetRegisterDescriptors;
    using ::Targets::TargetRegisterDescriptorAndValuePairs;

    DebugTranslator::DebugTranslator(
        DebugTransportModuleInterface& dtmInterface,
        const DebugTranslatorConfig& config,
        const TargetDescriptionFile& targetDescriptionFile,
        const RiscVTargetConfig& targetConfig
    )
        : dtmInterface(dtmInterface)
        , config(config)
        , targetDescriptionFile(targetDescriptionFile)
        , targetConfig(targetConfig)
        , sysAddressSpaceDescriptor(targetDescriptionFile.getSystemAddressSpaceDescriptor())
    {}

    void DebugTranslator::activate() {
        this->debugModuleDescriptor.hartIndices = this->discoverHartIndices();
        if (this->debugModuleDescriptor.hartIndices.empty()) {
            throw Exceptions::TargetFailure{"Failed to discover any RISC-V harts"};
        }

        if (this->debugModuleDescriptor.hartIndices.size() > 1) {
            Logger::debug(
                "Discovered RISC-V harts: " + std::to_string(this->debugModuleDescriptor.hartIndices.size())
            );
            Logger::warning("Bloom only supports debugging a single RISC-V hart - selecting first available");
        }

        this->selectedHartIndex = this->debugModuleDescriptor.hartIndices.front();
        Logger::info("Selected RISC-V hart index: " + std::to_string(this->selectedHartIndex));

        /*
         * Disabling the debug module before enabling it will clear any state from a previous debug session that
         * wasn't terminated properly.
         */
        this->disableDebugModule();
        this->enableDebugModule();

        this->stop();

        this->debugModuleDescriptor.triggerDescriptorsByIndex = this->discoverTriggers();

        Logger::debug(
            "Discovered RISC-V triggers: "
                + std::to_string(this->debugModuleDescriptor.triggerDescriptorsByIndex.size())
        );

        if (!this->debugModuleDescriptor.triggerDescriptorsByIndex.empty()) {
            // Clear any left-over triggers from the previous debug session
            this->clearAllTriggers();
        }

        this->initDebugControlStatusRegister();

        const auto abstractControlStatusRegister = this->readDebugModuleAbstractControlStatusRegister();
        this->debugModuleDescriptor.abstractDataRegisterCount = abstractControlStatusRegister.dataRegisterCount;
        this->debugModuleDescriptor.programBufferSize = abstractControlStatusRegister.programBufferSize;

        Logger::debug("Data register count: " + std::to_string(this->debugModuleDescriptor.abstractDataRegisterCount));
        Logger::debug("Program buffer size: " + std::to_string(this->debugModuleDescriptor.programBufferSize));

        this->clearProgramBuffer();

        if (this->debugModuleDescriptor.abstractDataRegisterCount > 0) {
            if (this->debugModuleDescriptor.programBufferSize >= 3) {
                this->debugModuleDescriptor.memoryAccessStrategies.insert(MemoryAccessStrategy::PROGRAM_BUFFER);
            }

            /*
             * Attempt to read a single word from the start of the system address space, via a memory access abstract
             * command.
             */
            constexpr auto probingMemoryAccessCommand = AbstractCommandRegister{
                .control = MemoryAccessControlField{
                    .postIncrement = true,
                    .size = MemoryAccessControlField::MemorySize::SIZE_32,
                }.value(),
                .commandType = AbstractCommandRegister::CommandType::MEMORY_ACCESS
            };

            this->dtmInterface.writeDebugModuleRegister(
                RegisterAddress::ABSTRACT_DATA_1,
                this->targetDescriptionFile.getSystemAddressSpace().startAddress
            );

            if (this->tryExecuteAbstractCommand(probingMemoryAccessCommand) == AbstractCommandError::NONE) {
                this->debugModuleDescriptor.memoryAccessStrategies.insert(MemoryAccessStrategy::ABSTRACT_COMMAND);
            }
        }

        if (this->debugModuleDescriptor.memoryAccessStrategies.empty()) {
            throw Exceptions::TargetFailure{"Target doesn't support any known memory access strategies"};
        }

        this->memoryAccessStrategy = this->determineMemoryAccessStrategy();
        Logger::debug(
            "Selected memory access strategy: " + (
                this->memoryAccessStrategy == MemoryAccessStrategy::ABSTRACT_COMMAND
                    ? std::string{"ABSTRACT_COMMAND"}
                    : std::string{"PROGRAM_BUFFER"}
            )
        );
    }

    void DebugTranslator::deactivate() {
        this->disableDebugModule();
    }

    TargetExecutionState DebugTranslator::getExecutionState() {
        const auto statusRegister = this->readDebugModuleStatusRegister();

        if (statusRegister.anyHaveReset) {
            Logger::warning("Reset detected at RISC-V hart " + std::to_string(this->selectedHartIndex));

            if (statusRegister.anyRunning) {
                this->stop();
            }

            this->initDebugControlStatusRegister();
            this->writeDebugModuleControlRegister(ControlRegister{
                .debugModuleActive = true,
                .selectedHartIndex = this->selectedHartIndex,
                .acknowledgeHaveReset = true,
            });

            if (statusRegister.anyRunning) {
                this->run();
            }
        }

        return statusRegister.anyRunning
            ? TargetExecutionState::RUNNING
            : TargetExecutionState::STOPPED;
    }

    void DebugTranslator::stop() {
        auto controlRegister = ControlRegister{
            .debugModuleActive = true,
            .selectedHartIndex = this->selectedHartIndex,
            .haltRequest = true,
        };

        this->writeDebugModuleControlRegister(controlRegister);
        auto statusRegister = this->readDebugModuleStatusRegister();

        for (
            auto attempts = 0;
            !statusRegister.allHalted
                && (DebugTranslator::DEBUG_MODULE_RESPONSE_DELAY * attempts) <= this->config.targetResponseTimeout;
            ++attempts
        ) {
            std::this_thread::sleep_for(DebugTranslator::DEBUG_MODULE_RESPONSE_DELAY);
            statusRegister = this->readDebugModuleStatusRegister();
        }

        controlRegister.haltRequest = false;
        this->writeDebugModuleControlRegister(controlRegister);

        if (!statusRegister.allHalted) {
            throw Exceptions::TargetOperationFailure{"Target took too long to halt selected harts"};
        }
    }

    void DebugTranslator::run() {
        auto controlRegister = ControlRegister{
            .debugModuleActive = true,
            .setResetHaltRequest = true,
            .selectedHartIndex = this->selectedHartIndex,
            .resumeRequest = true,
        };

        this->writeDebugModuleControlRegister(controlRegister);
        auto statusRegister = this->readDebugModuleStatusRegister();

        for (
            auto attempts = 0;
            !statusRegister.allResumeAcknowledge
                && (DebugTranslator::DEBUG_MODULE_RESPONSE_DELAY * attempts) <= this->config.targetResponseTimeout;
            ++attempts
        ) {
            std::this_thread::sleep_for(DebugTranslator::DEBUG_MODULE_RESPONSE_DELAY);
            statusRegister = this->readDebugModuleStatusRegister();
        }

        controlRegister.resumeRequest = false;
        this->writeDebugModuleControlRegister(controlRegister);

        if (!statusRegister.allResumeAcknowledge) {
            Logger::debug("Failed to resume target execution - stopping target");
            this->stop();
            throw Exceptions::TargetOperationFailure{"Target took too long to acknowledge resume request"};
        }
    }

    void DebugTranslator::step() {
        auto debugControlStatusRegister = this->readDebugControlStatusRegister();
        debugControlStatusRegister.step = true;

        this->writeDebugControlStatusRegister(debugControlStatusRegister);

        auto controlRegister = ControlRegister{
            .debugModuleActive = true,
            .setResetHaltRequest = true,
            .selectedHartIndex = this->selectedHartIndex,
            .resumeRequest = true,
        };

        this->writeDebugModuleControlRegister(controlRegister);

        controlRegister.resumeRequest = false;
        this->writeDebugModuleControlRegister(controlRegister);

        debugControlStatusRegister.step = false;
        this->writeDebugControlStatusRegister(debugControlStatusRegister);
    }

    void DebugTranslator::reset() {
        this->writeDebugModuleControlRegister(ControlRegister{
            .debugModuleActive = true,
            .ndmReset = true,
            .setResetHaltRequest = true,
            .selectedHartIndex = this->selectedHartIndex,
            .haltRequest = true,
        });
        this->writeDebugModuleControlRegister(ControlRegister{
            .debugModuleActive = true,
            .selectedHartIndex = this->selectedHartIndex,
            .haltRequest = true,
        });

        auto statusRegister = this->readDebugModuleStatusRegister();
        for (
            auto attempts = 0;
            !statusRegister.allHaveReset
                && (DebugTranslator::DEBUG_MODULE_RESPONSE_DELAY * attempts) <= this->config.targetResponseTimeout;
            ++attempts
        ) {
            std::this_thread::sleep_for(DebugTranslator::DEBUG_MODULE_RESPONSE_DELAY);
            statusRegister = this->readDebugModuleStatusRegister();
        }

        this->writeDebugModuleControlRegister(ControlRegister{
            .debugModuleActive = true,
            .setResetHaltRequest = true,
            .selectedHartIndex = this->selectedHartIndex,
            .acknowledgeHaveReset = true,
            .haltRequest = true,
        });

        if (!statusRegister.allHaveReset) {
            throw Exceptions::TargetOperationFailure{"Target took too long to reset"};
        }

        this->initDebugControlStatusRegister();
    }

    std::uint16_t DebugTranslator::getTriggerCount() const {
        return static_cast<std::uint16_t>(this->debugModuleDescriptor.triggerDescriptorsByIndex.size());
    }

    void DebugTranslator::insertTriggerBreakpoint(TargetMemoryAddress address) {
        using TriggerModule::TriggerType;

        // We may already have a trigger for this address. If so, reuse it.
        const auto preexistingTriggerIndexIt = this->triggerIndicesByBreakpointAddress.find(address);
        auto triggerDescriptorOpt = preexistingTriggerIndexIt != this->triggerIndicesByBreakpointAddress.end()
            ? std::cref(this->debugModuleDescriptor.triggerDescriptorsByIndex.at(preexistingTriggerIndexIt->second))
            : this->getAvailableTrigger();

        if (!triggerDescriptorOpt.has_value()) {
            throw Exceptions::TargetOperationFailure{"Insufficient resources - no available trigger"};
        }

        const auto& triggerDescriptor = triggerDescriptorOpt->get();
        Logger::debug(
            "Installing RISC-V trigger for program address 0x" + Services::StringService::toHex(address)
                + " with trigger index " + std::to_string(triggerDescriptor.index)
        );

        if (triggerDescriptor.supportedTypes.contains(TriggerType::MATCH_CONTROL)) {
            using TriggerModule::Registers::MatchControl;

            this->writeCpuRegister(
                CpuRegisterNumber::TRIGGER_SELECT,
                TriggerModule::Registers::TriggerSelect{triggerDescriptor.index}.value()
            );

            this->writeCpuRegister(
                CpuRegisterNumber::TRIGGER_DATA_1,
                MatchControl{
                    .execute = true,
                    .enabledInUserMode = true,
                    .enabledInSupervisorMode = true,
                    .enabledInMachineMode = true,
                    .action = TriggerModule::TriggerAction::ENTER_DEBUG_MODE,
                    .accessSize = MatchControl::AccessSize::ANY,
                    .compareValueType = MatchControl::CompareValueType::ADDRESS,
                }.value()
            );
            this->writeCpuRegister(CpuRegisterNumber::TRIGGER_DATA_2, address);

            this->allocatedTriggerIndices.emplace(triggerDescriptor.index);
            this->triggerIndicesByBreakpointAddress.emplace(address, triggerDescriptor.index);
            return;
        }

        throw Exceptions::TargetOperationFailure{"Unsupported trigger"};
    }

    void DebugTranslator::clearTriggerBreakpoint(TargetMemoryAddress address) {
        const auto triggerIndexIt = this->triggerIndicesByBreakpointAddress.find(address);
        if (triggerIndexIt == this->triggerIndicesByBreakpointAddress.end()) {
            throw Exceptions::TargetOperationFailure{"Unknown hardware breakpoint"};
        }

        const auto& triggerDescriptor = this->debugModuleDescriptor.triggerDescriptorsByIndex.at(triggerIndexIt->second);

        this->clearTrigger(triggerDescriptor);
        this->triggerIndicesByBreakpointAddress.erase(address);
        this->allocatedTriggerIndices.erase(triggerDescriptor.index);
    }

    void DebugTranslator::clearAllTriggers() {
        // To ensure that any untracked breakpoints are cleared, we clear all triggers on the target.
        for (const auto& [triggerIndex, triggerDescriptor] : this->debugModuleDescriptor.triggerDescriptorsByIndex) {
            this->clearTrigger(triggerDescriptor);
        }

        this->triggerIndicesByBreakpointAddress.clear();
        this->allocatedTriggerIndices.clear();
    }

    TargetRegisterDescriptorAndValuePairs DebugTranslator::readCpuRegisters(
        const TargetRegisterDescriptors& descriptors
    ) {
        auto output = TargetRegisterDescriptorAndValuePairs{};

        for (const auto& descriptor : descriptors) {
            const auto registerValue = this->readCpuRegister(static_cast<RegisterNumber>(descriptor->startAddress));
            output.emplace_back(
                *descriptor,
                TargetMemoryBuffer{
                    static_cast<unsigned char>(registerValue >> 24),
                    static_cast<unsigned char>(registerValue >> 16),
                    static_cast<unsigned char>(registerValue >> 8),
                    static_cast<unsigned char>(registerValue),
                }
            );
        }

        return output;
    }

    void DebugTranslator::writeCpuRegisters(const TargetRegisterDescriptorAndValuePairs& registers) {
        for (const auto& [descriptor, value] : registers) {
            assert((value.size() * 8) <= std::numeric_limits<RegisterValue>::digits);

            auto registerValue = RegisterValue{0};
            for (const auto& registerByte : value) {
                registerValue = (registerValue << 8) | registerByte;
            }

            this->writeCpuRegister(static_cast<RegisterNumber>(descriptor.startAddress), registerValue);
        }
    }

    TargetMemoryBuffer DebugTranslator::readMemory(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        TargetMemoryAddress startAddress,
        TargetMemorySize bytes,
        const std::set<TargetMemoryAddressRange>& excludedAddressRanges
    ) {
        if (addressSpaceDescriptor != this->sysAddressSpaceDescriptor) {
            throw Exceptions::TargetOperationFailure{"Unsupported address space"};
        }

        // TODO: excluded addresses

        constexpr auto alignTo = DebugTranslator::WORD_BYTE_SIZE;
        if ((startAddress % alignTo) != 0 || (bytes % alignTo) != 0) {
            // Alignment required
            const auto alignedStartAddress = this->alignMemoryAddress(startAddress, alignTo);
            const auto alignedBytes = this->alignMemorySize(bytes + (startAddress - alignedStartAddress), alignTo);

            const auto memoryBuffer = this->readMemory(
                addressSpaceDescriptor,
                memorySegmentDescriptor,
                alignedStartAddress,
                alignedBytes,
                excludedAddressRanges
            );

            const auto offset = memoryBuffer.begin() + (startAddress - alignedStartAddress);
            return TargetMemoryBuffer{offset, offset + bytes};
        }

        if (this->memoryAccessStrategy == MemoryAccessStrategy::PROGRAM_BUFFER) {
            return this->readMemoryViaProgramBuffer(startAddress, bytes);
        }

        if (this->memoryAccessStrategy == MemoryAccessStrategy::ABSTRACT_COMMAND) {
            return this->readMemoryViaAbstractCommand(startAddress, bytes);
        }

        throw Exceptions::InternalFatalErrorException{"Unknown selected memory access strategy"};
    }

    void DebugTranslator::writeMemory(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        TargetMemoryAddress startAddress,
        TargetMemoryBufferSpan buffer
    ) {
        if (addressSpaceDescriptor != this->sysAddressSpaceDescriptor) {
            throw Exceptions::TargetOperationFailure{"Unsupported address space"};
        }

        constexpr auto alignTo = DebugTranslator::WORD_BYTE_SIZE;
        const auto bytes = static_cast<TargetMemorySize>(buffer.size());
        if ((startAddress % alignTo) != 0 || (bytes % alignTo) != 0) {
            /*
             * Alignment required
             *
             * To align the write operation, we read the front and back offset bytes and use them to construct an
             * aligned buffer.
             */
            const auto alignedStartAddress = this->alignMemoryAddress(startAddress, alignTo);
            const auto alignedBytes = this->alignMemorySize(bytes + (startAddress - alignedStartAddress), alignTo);

            assert(alignedBytes > bytes);

            auto alignedBuffer = (alignedStartAddress < startAddress)
                ? this->readMemory(
                    addressSpaceDescriptor,
                    memorySegmentDescriptor,
                    alignedStartAddress,
                    (startAddress - alignedStartAddress),
                    {}
                )
                : TargetMemoryBuffer{};

            alignedBuffer.resize(alignedBytes);

            std::copy(
                buffer.begin(),
                buffer.end(),
                alignedBuffer.begin() + (startAddress - alignedStartAddress)
            );

            const auto dataBack = this->readMemory(
                addressSpaceDescriptor,
                memorySegmentDescriptor,
                startAddress + bytes,
                alignedBytes - bytes - (startAddress - alignedStartAddress),
                {}
            );
            std::copy(
                dataBack.begin(),
                dataBack.end(),
                alignedBuffer.begin() + (startAddress - alignedStartAddress) + bytes
            );

            return this->writeMemory(
                addressSpaceDescriptor,
                memorySegmentDescriptor,
                alignedStartAddress,
                alignedBuffer
            );
        }

        if (this->memoryAccessStrategy == MemoryAccessStrategy::PROGRAM_BUFFER) {
            return this->writeMemoryViaProgramBuffer(startAddress, buffer);
        }

        if (this->memoryAccessStrategy == MemoryAccessStrategy::ABSTRACT_COMMAND) {
            return this->writeMemoryViaAbstractCommand(startAddress, buffer);
        }

        throw Exceptions::InternalFatalErrorException{"Unknown selected memory access strategy"};
    }

    AbstractCommandError DebugTranslator::readAndClearAbstractCommandError() {
        const auto commandError = this->readDebugModuleAbstractControlStatusRegister().commandError;
        if (commandError != AbstractCommandError::NONE) {
            this->clearAbstractCommandError();
        }

        return commandError;
    }

    void DebugTranslator::clearProgramBuffer() {
        if (this->debugModuleDescriptor.programBufferSize < 1) {
            return;
        }

        static constexpr auto clearedBuffer = Array::repeat<DebugTranslator::MAX_PROGRAM_BUFFER_SIZE>(Opcodes::Ebreak);

        assert(this->debugModuleDescriptor.programBufferSize <= DebugTranslator::MAX_PROGRAM_BUFFER_SIZE);
        this->writeProgramBuffer({clearedBuffer.begin(), this->debugModuleDescriptor.programBufferSize});
    }

    void DebugTranslator::executeFenceProgram() {
        static constexpr auto programOpcodes = std::to_array<Opcodes::Opcode>({
            Opcodes::FenceI,
            Opcodes::Fence,
            Opcodes::Ebreak,
        });

        if (programOpcodes.size() > this->debugModuleDescriptor.programBufferSize) {
            throw Exceptions::TargetOperationFailure{
                "Cannot execute fence program via RISC-V debug module program buffer - insufficient program buffer size"
            };
        }

        this->writeProgramBuffer(programOpcodes);
        this->readCpuRegister(CpuRegisterNumber::GPR_X8, {.postExecute = true});
    }

    std::vector<DebugModule::HartIndex> DebugTranslator::discoverHartIndices() {
        auto hartIndices = std::vector<DebugModule::HartIndex>{};

        /*
         * We can obtain the maximum hart index by setting all of the hartsel bits in the control register and then
         * read the value back.
         */
        this->writeDebugModuleControlRegister(
            ControlRegister{.debugModuleActive = true, .selectedHartIndex = 0xFFFFF}
        );

        const auto maxHartIndex = this->readDebugModuleControlRegister().selectedHartIndex;

        for (auto hartIndex = DebugModule::HartIndex{0}; hartIndex <= maxHartIndex; ++hartIndex) {
            /*
             * We can't just assume that everything between 0 and the maximum hart index are valid hart indices. We
             * have to test each index until we find one that is non-existent.
             */
            this->writeDebugModuleControlRegister(
                ControlRegister{.debugModuleActive = true, .selectedHartIndex = hartIndex}
            );

            /*
             * It's worth noting that some RISC-V targets **do not** set the non-existent flags. I'm not sure why.
             * Maybe hartsel has been hardwired to 0 on targets that only support a single hart, preventing the
             * selection of non-existent harts.
             *
             * Relying on the maximum hart index seems to be all we can do in this case.
             */
            if (this->readDebugModuleStatusRegister().anyNonExistent) {
                break;
            }

            hartIndices.emplace_back(hartIndex);
        }

        return hartIndices;
    }

    std::unordered_map<
        TriggerModule::TriggerIndex,
        TriggerModule::TriggerDescriptor
    > DebugTranslator::discoverTriggers() {
        auto output = std::unordered_map<TriggerModule::TriggerIndex, TriggerModule::TriggerDescriptor>{};

        constexpr auto MAX_TRIGGER_INDEX = 10;
        for (auto triggerIndex = TriggerModule::TriggerIndex{0}; triggerIndex <= MAX_TRIGGER_INDEX; ++triggerIndex) {
            const auto selectRegValue = TriggerModule::Registers::TriggerSelect{triggerIndex}.value();

            const auto writeSelectError = this->tryWriteCpuRegister(CpuRegisterNumber::TRIGGER_SELECT, selectRegValue);
            if (writeSelectError == AbstractCommandError::EXCEPTION) {
                break;
            }

            if (writeSelectError != AbstractCommandError::NONE) {
                throw Exceptions::TargetOperationFailure{
                    "Failed to write to TRIGGER_SELECT register - abstract command error: 0x"
                        + Services::StringService::toHex(static_cast<std::uint8_t>(writeSelectError))
                };
            }

            if (this->readCpuRegister(CpuRegisterNumber::TRIGGER_SELECT) != selectRegValue) {
                break;
            }

            const auto infoReg = TriggerModule::Registers::TriggerInfo::fromValue(
                this->readCpuRegister(CpuRegisterNumber::TRIGGER_INFO)
            );

            if (infoReg.info == 0x01) {
                // Trigger doesn't exist
                break;
            }

            auto supportedTypes = infoReg.getSupportedTriggerTypes();
            if (supportedTypes.empty()) {
                // The trigger info register has no trigger type info. Try the data1 register.
                const auto data1Reg = TriggerModule::Registers::TriggerData1::fromValue(
                    this->readCpuRegister(CpuRegisterNumber::TRIGGER_DATA_1)
                );

                const auto triggerType = data1Reg.getType();
                if (!triggerType.has_value()) {
                    // Trigger data1 register also lacks type info. Assume the trigger doesn't exist
                    break;
                }

                supportedTypes.insert(*triggerType);
            }

            output.emplace(triggerIndex, TriggerModule::TriggerDescriptor{triggerIndex, supportedTypes});
        }

        return output;
    }

    ControlRegister DebugTranslator::readDebugModuleControlRegister() {
        return ControlRegister::fromValue(
            this->dtmInterface.readDebugModuleRegister(RegisterAddress::CONTROL_REGISTER)
        );
    }

    StatusRegister DebugTranslator::readDebugModuleStatusRegister() {
        return StatusRegister::fromValue(this->dtmInterface.readDebugModuleRegister(RegisterAddress::STATUS_REGISTER));
    }

    AbstractControlStatusRegister DebugTranslator::readDebugModuleAbstractControlStatusRegister() {
        return AbstractControlStatusRegister::fromValue(
            this->dtmInterface.readDebugModuleRegister(RegisterAddress::ABSTRACT_CONTROL_STATUS_REGISTER)
        );
    }

    DebugControlStatusRegister DebugTranslator::readDebugControlStatusRegister() {
        return DebugControlStatusRegister::fromValue(
            this->readCpuRegister(static_cast<RegisterNumber>(CpuRegisterNumber::DEBUG_CONTROL_STATUS_REGISTER))
        );
    }

    void DebugTranslator::enableDebugModule() {
        auto controlRegister = ControlRegister{
            .debugModuleActive = true,
            .selectedHartIndex = this->selectedHartIndex
        };

        this->writeDebugModuleControlRegister(controlRegister);
        controlRegister = this->readDebugModuleControlRegister();

        for (
            auto attempts = 0;
            !controlRegister.debugModuleActive
                && (DebugTranslator::DEBUG_MODULE_RESPONSE_DELAY * attempts) <= this->config.targetResponseTimeout;
            ++attempts
        ) {
            std::this_thread::sleep_for(DebugTranslator::DEBUG_MODULE_RESPONSE_DELAY);
            controlRegister = this->readDebugModuleControlRegister();
        }

        if (!controlRegister.debugModuleActive) {
            throw Exceptions::TargetOperationFailure{"Took too long to enable debug module"};
        }
    }

    void DebugTranslator::disableDebugModule() {
        this->writeDebugModuleControlRegister(
            ControlRegister{.debugModuleActive = false, .selectedHartIndex = this->selectedHartIndex}
        );

        auto controlRegister = this->readDebugModuleControlRegister();

        for (
            auto attempts = 0;
            controlRegister.debugModuleActive
                && (DebugTranslator::DEBUG_MODULE_RESPONSE_DELAY * attempts) <= this->config.targetResponseTimeout;
            ++attempts
        ) {
            std::this_thread::sleep_for(DebugTranslator::DEBUG_MODULE_RESPONSE_DELAY);
            controlRegister = this->readDebugModuleControlRegister();
        }

        if (controlRegister.debugModuleActive) {
            throw Exceptions::TargetOperationFailure{"Took too long to disable debug module"};
        }
    }

    void DebugTranslator::initDebugControlStatusRegister() {
        this->writeDebugControlStatusRegister(DebugControlStatusRegister{
            .breakUMode = true,
            .breakSMode = true,
            .breakMMode = true,
            .breakVUMode = true,
            .breakVSMode = true,
        });
    }

    Expected<RegisterValue, AbstractCommandError> DebugTranslator::tryReadCpuRegister(
        RegisterNumber number,
        const RegisterAccessControlField::Flags& flags
    ) {
        const auto commandError = this->tryExecuteAbstractCommand(AbstractCommandRegister{
            .control = RegisterAccessControlField{
                .registerNumber = number,
                .transfer = true,
                .flags = flags,
                .size= RegisterAccessControlField::RegisterSize::SIZE_32
            }.value(),
            .commandType = AbstractCommandRegister::CommandType::REGISTER_ACCESS
        });

        if (commandError != AbstractCommandError::NONE) {
            return commandError;
        }

        return this->dtmInterface.readDebugModuleRegister(RegisterAddress::ABSTRACT_DATA_0);
    }

    Expected<RegisterValue, AbstractCommandError> DebugTranslator::tryReadCpuRegister(
        Registers::CpuRegisterNumber number,
        const RegisterAccessControlField::Flags& flags
    ) {
        return this->tryReadCpuRegister(static_cast<RegisterNumber>(number), flags);
    }

    RegisterValue DebugTranslator::readCpuRegister(
        RegisterNumber number,
        const RegisterAccessControlField::Flags& flags
    ) {
        const auto result = this->tryReadCpuRegister(number, flags);

        if (!result.hasValue()) {
            throw Exceptions::TargetOperationFailure{
                "Failed to read CPU register (number: 0x" + Services::StringService::toHex(number)
                    + ") - abstract command error: 0x"
                    + Services::StringService::toHex(static_cast<std::uint8_t>(result.error()))
            };
        }

        return result.value();
    }

    RegisterValue DebugTranslator::readCpuRegister(
        Registers::CpuRegisterNumber number,
        const RegisterAccessControlField::Flags& flags
    ) {
        return this->readCpuRegister(static_cast<RegisterNumber>(number), flags);
    }

    AbstractCommandError DebugTranslator::tryWriteCpuRegister(
        RegisterNumber number,
        RegisterValue value,
        const RegisterAccessControlField::Flags& flags
    ) {
        this->dtmInterface.writeDebugModuleRegister(RegisterAddress::ABSTRACT_DATA_0, value);
        return this->tryExecuteAbstractCommand(AbstractCommandRegister{
            .control = RegisterAccessControlField{
                .registerNumber = number,
                .write = true,
                .transfer = true,
                .flags = flags,
                .size = RegisterAccessControlField::RegisterSize::SIZE_32
            }.value(),
            .commandType = AbstractCommandRegister::CommandType::REGISTER_ACCESS
        });
    }

    AbstractCommandError DebugTranslator::tryWriteCpuRegister(
        Registers::CpuRegisterNumber number,
        RegisterValue value,
        const RegisterAccessControlField::Flags& flags
    ) {
        return this->tryWriteCpuRegister(static_cast<RegisterNumber>(number), value, flags);
    }

    void DebugTranslator::writeCpuRegister(
        RegisterNumber number,
        RegisterValue value,
        const RegisterAccessControlField::Flags& flags
    ) {
        const auto commandError = this->tryWriteCpuRegister(number, value, flags);
        if (commandError != AbstractCommandError::NONE) {
            throw Exceptions::TargetOperationFailure{
                "Failed to write to CPU register (number: 0x" + Services::StringService::toHex(number)
                    + ") - abstract command error: 0x"
                    + Services::StringService::toHex(static_cast<std::uint8_t>(commandError))
            };
        }
    }

    void DebugTranslator::writeCpuRegister(
        Registers::CpuRegisterNumber number,
        RegisterValue value,
        const RegisterAccessControlField::Flags& flags
    ) {
        this->writeCpuRegister(static_cast<RegisterNumber>(number), value, flags);
    }

    void DebugTranslator::writeDebugModuleControlRegister(const ControlRegister& controlRegister) {
        this->dtmInterface.writeDebugModuleRegister(RegisterAddress::CONTROL_REGISTER, controlRegister.value());
    }

    void DebugTranslator::writeDebugControlStatusRegister(const DebugControlStatusRegister& controlRegister) {
        this->writeCpuRegister(
            static_cast<RegisterNumber>(CpuRegisterNumber::DEBUG_CONTROL_STATUS_REGISTER),
            controlRegister.value()
        );
    }

    void DebugTranslator::clearAbstractCommandError() {
        this->dtmInterface.writeDebugModuleRegister(
            RegisterAddress::ABSTRACT_CONTROL_STATUS_REGISTER,
            AbstractControlStatusRegister{.commandError = AbstractCommandError::CLEAR}.value()
        );
    }

    AbstractCommandError DebugTranslator::tryExecuteAbstractCommand(
        const DebugModule::Registers::AbstractCommandRegister& abstractCommandRegister
    ) {
        this->dtmInterface.writeDebugModuleRegister(
            RegisterAddress::ABSTRACT_COMMAND_REGISTER,
            abstractCommandRegister.value()
        );

        auto abstractStatusRegister = this->readDebugModuleAbstractControlStatusRegister();

        for (
            auto attempts = 0;
            abstractStatusRegister.busy
                && (DebugTranslator::DEBUG_MODULE_RESPONSE_DELAY * attempts) <= this->config.targetResponseTimeout;
            ++attempts
        ) {
            std::this_thread::sleep_for(DebugTranslator::DEBUG_MODULE_RESPONSE_DELAY);
            abstractStatusRegister = this->readDebugModuleAbstractControlStatusRegister();
        }

        if (abstractStatusRegister.busy) {
            throw Exceptions::TargetOperationFailure{"Abstract command took too long to execute"};
        }

        if (abstractStatusRegister.commandError != AbstractCommandError::NONE) {
            this->clearAbstractCommandError();
        }

        return abstractStatusRegister.commandError;
    }

    void DebugTranslator::executeAbstractCommand(
        const DebugModule::Registers::AbstractCommandRegister& abstractCommandRegister
    ) {
        const auto commandError = this->tryExecuteAbstractCommand(abstractCommandRegister);
        if (commandError != AbstractCommandError::NONE) {
            throw Exceptions::TargetOperationFailure{
                "Failed to execute abstract command - error: 0x"
                    + Services::StringService::toHex(static_cast<std::uint8_t>(commandError))
            };
        }
    }

    MemoryAccessStrategy DebugTranslator::determineMemoryAccessStrategy() {
        assert(!this->debugModuleDescriptor.memoryAccessStrategies.empty());

        if (
            this->config.preferredMemoryAccessStrategy.has_value()
            && this->debugModuleDescriptor.memoryAccessStrategies.contains(
                *(this->config.preferredMemoryAccessStrategy)
            )
        ) {
            return *(this->config.preferredMemoryAccessStrategy);
        }

        // Favour the abstract command strategy, as it seems to be faster on the targets currently supported by Bloom.
        return this->debugModuleDescriptor.memoryAccessStrategies.contains(MemoryAccessStrategy::ABSTRACT_COMMAND)
            ? MemoryAccessStrategy::ABSTRACT_COMMAND
            : *(this->debugModuleDescriptor.memoryAccessStrategies.begin());
    }

    TargetMemoryAddress DebugTranslator::alignMemoryAddress(TargetMemoryAddress address, TargetMemoryAddress alignTo) {
        return (address / alignTo) * alignTo;
    }

    TargetMemorySize DebugTranslator::alignMemorySize(TargetMemorySize size, TargetMemorySize alignTo) {
        return static_cast<TargetMemorySize>(
            std::ceil(static_cast<double>(size) / static_cast<double>(alignTo))
        ) * alignTo;
    }

    Targets::TargetMemoryBuffer DebugTranslator::readMemoryViaAbstractCommand(
        Targets::TargetMemoryAddress startAddress,
        Targets::TargetMemorySize bytes
    ) {
        assert(startAddress % DebugTranslator::WORD_BYTE_SIZE == 0);
        assert(bytes % DebugTranslator::WORD_BYTE_SIZE == 0);

        /*
         * We only need to set the address once. No need to update it as we use the post-increment function to
         * increment the address. See MemoryAccessControlField::postIncrement
         */
        this->dtmInterface.writeDebugModuleRegister(RegisterAddress::ABSTRACT_DATA_1, startAddress);

        constexpr auto command = AbstractCommandRegister{
            .control = MemoryAccessControlField{
                .postIncrement = true,
                .size = MemoryAccessControlField::MemorySize::SIZE_32,
            }.value(),
            .commandType = AbstractCommandRegister::CommandType::MEMORY_ACCESS
        };

        auto output = TargetMemoryBuffer{};
        output.reserve(bytes);

        for (auto address = startAddress; address <= (startAddress + bytes - 1); address += 4) {
            const auto commandError = this->tryExecuteAbstractCommand(command);
            if (commandError != AbstractCommandError::NONE) {
                if (commandError == AbstractCommandError::EXCEPTION) {
                    throw Exceptions::IllegalMemoryAccess{};
                }

                throw Exceptions::TargetOperationFailure{
                    "Failed to read memory via abstract command - error: 0x"
                        + Services::StringService::toHex(static_cast<std::uint8_t>(commandError))
                };
            }

            const auto data = this->dtmInterface.readDebugModuleRegister(RegisterAddress::ABSTRACT_DATA_0);
            output.emplace_back(static_cast<unsigned char>(data));
            output.emplace_back(static_cast<unsigned char>(data >> 8));
            output.emplace_back(static_cast<unsigned char>(data >> 16));
            output.emplace_back(static_cast<unsigned char>(data >> 24));
        }

        return output;
    }

    void DebugTranslator::writeMemoryViaAbstractCommand(
        Targets::TargetMemoryAddress startAddress,
        Targets::TargetMemoryBufferSpan buffer
    ) {
        using DebugModule::Registers::MemoryAccessControlField;
        assert(startAddress % DebugTranslator::WORD_BYTE_SIZE == 0);
        assert(buffer.size() % DebugTranslator::WORD_BYTE_SIZE == 0);

        this->dtmInterface.writeDebugModuleRegister(RegisterAddress::ABSTRACT_DATA_1, startAddress);

        static constexpr auto command = AbstractCommandRegister{
            .control = MemoryAccessControlField{
                .write = true,
                .postIncrement = true,
                .size = MemoryAccessControlField::MemorySize::SIZE_32,
            }.value(),
            .commandType = AbstractCommandRegister::CommandType::MEMORY_ACCESS
        };

        for (auto offset = std::size_t{0}; offset < buffer.size(); offset += 4) {
            this->dtmInterface.writeDebugModuleRegister(
                RegisterAddress::ABSTRACT_DATA_0,
                static_cast<RegisterValue>(
                    (buffer[offset + 3] << 24)
                    | (buffer[offset + 2] << 16)
                    | (buffer[offset + 1] << 8)
                    | (buffer[offset])
                )
            );

            const auto commandError = this->tryExecuteAbstractCommand(command);
            if (commandError != AbstractCommandError::NONE) {
                if (commandError == AbstractCommandError::EXCEPTION) {
                    throw Exceptions::IllegalMemoryAccess{};
                }

                throw Exceptions::TargetOperationFailure{
                    "Failed to write memory via abstract command - error: 0x"
                        + Services::StringService::toHex(static_cast<std::uint8_t>(commandError))
                };
            }
        }
    }

    Targets::TargetMemoryBuffer DebugTranslator::readMemoryViaProgramBuffer(
        Targets::TargetMemoryAddress startAddress,
        Targets::TargetMemorySize bytes
    ) {
        assert(startAddress % DebugTranslator::WORD_BYTE_SIZE == 0);
        assert(bytes % DebugTranslator::WORD_BYTE_SIZE == 0);

        static constexpr auto programOpcodes = std::to_array<Opcodes::Opcode>({
            Opcodes::Lw{
                .destinationRegister = Opcodes::GprNumber::X9,
                .baseAddressRegister = Opcodes::GprNumber::X8,
                .addressOffset = 0
            }.opcode(),
            Opcodes::Addi{
                .destinationRegister = Opcodes::GprNumber::X8,
                .sourceRegister = Opcodes::GprNumber::X8,
                .value = DebugTranslator::WORD_BYTE_SIZE
            }.opcode(),
            Opcodes::Ebreak,
        });

        if (programOpcodes.size() > this->debugModuleDescriptor.programBufferSize) {
            throw Exceptions::TargetOperationFailure{
                "Cannot read memory via RISC-V debug module program buffer - insufficient program buffer size"
            };
        }

        auto preservedX8Register = PreservedCpuRegister{CpuRegisterNumber::GPR_X8, *this};
        auto preservedX9Register = PreservedCpuRegister{CpuRegisterNumber::GPR_X9, *this};

        try {
            this->writeProgramBuffer(programOpcodes);

            auto commandError = this->tryWriteCpuRegister(
                CpuRegisterNumber::GPR_X8,
                startAddress,
                {.postExecute = true}
            );
            if (commandError != AbstractCommandError::NONE) {
                if (commandError == AbstractCommandError::EXCEPTION) {
                    throw Exceptions::IllegalMemoryAccess{};
                }

                throw Exceptions::TargetOperationFailure{
                    "Program buffer execution failed - abstract command error: 0x"
                        + Services::StringService::toHex(commandError)
                };
            }

            auto output = Targets::TargetMemoryBuffer{};
            output.reserve(bytes);

            if (bytes == DebugTranslator::WORD_BYTE_SIZE) {
                const auto word = this->readCpuRegister(CpuRegisterNumber::GPR_X9);
                output.emplace_back(static_cast<unsigned char>(word));
                output.emplace_back(static_cast<unsigned char>(word >> 8));
                output.emplace_back(static_cast<unsigned char>(word >> 16));
                output.emplace_back(static_cast<unsigned char>(word >> 24));

                preservedX8Register.restore();
                preservedX9Register.restore();

                return output;
            }

            // Populate the abstract command register with a register access command, to read X9 into data0.
            this->readCpuRegister(CpuRegisterNumber::GPR_X9, {.postExecute = true});

            /*
             * At this point, the program buffer will have already been executed twice, with the first word currently
             * residing in data0, and the second in X9.
             *
             * The abstract command register will be populated with a register access command, to read X9 into data0,
             * with 'postexec' enabled. Enabling auto execution at this point will mean that the abstract command will
             * be executed every time we access the data0 register, resulting in the next word being copied into
             * data0 (from X9) and the program buffer being executed again (filling X9 with another word).
             *
             * To avoid reading an excess of words (which could result in an out-of-bounds exception), we only enable
             * auto execution if we require more data that what has already been read.
             */
            const auto autoExecutionEnabled = bytes > (DebugTranslator::WORD_BYTE_SIZE * 2);
            this->dtmInterface.writeDebugModuleRegister(
                RegisterAddress::ABSTRACT_COMMAND_AUTO_EXECUTE_REGISTER,
                AbstractCommandAutoExecuteRegister{.onData0Access = autoExecutionEnabled}.value()
            );

            while (output.size() < (bytes - DebugTranslator::WORD_BYTE_SIZE)) {
                if (autoExecutionEnabled && output.size() >= (bytes - DebugTranslator::WORD_BYTE_SIZE * 2)) {
                    /*
                     * We're on the second to last word, which has already been read and currently resides in data0.
                     * The last word has also been read and currently resides in X9.
                     *
                     * Disable auto execution here to prevent any further reads.
                     */
                    this->dtmInterface.writeDebugModuleRegister(
                        RegisterAddress::ABSTRACT_COMMAND_AUTO_EXECUTE_REGISTER,
                        AbstractCommandAutoExecuteRegister{}.value()
                    );
                }

                const auto word = this->dtmInterface.readDebugModuleRegister(RegisterAddress::ABSTRACT_DATA_0);
                output.emplace_back(static_cast<unsigned char>(word));
                output.emplace_back(static_cast<unsigned char>(word >> 8));
                output.emplace_back(static_cast<unsigned char>(word >> 16));
                output.emplace_back(static_cast<unsigned char>(word >> 24));
            }

            commandError = this->readAndClearAbstractCommandError();
            if (commandError != AbstractCommandError::NONE) {
                if (commandError == AbstractCommandError::EXCEPTION) {
                    throw Exceptions::IllegalMemoryAccess{};
                }

                throw Exceptions::TargetOperationFailure{
                    "Program buffer execution failed - abstract command error: 0x"
                        + Services::StringService::toHex(commandError)
                };
            }

            const auto lastWord = this->readCpuRegister(CpuRegisterNumber::GPR_X9);
            output.emplace_back(static_cast<unsigned char>(lastWord));
            output.emplace_back(static_cast<unsigned char>(lastWord >> 8));
            output.emplace_back(static_cast<unsigned char>(lastWord >> 16));
            output.emplace_back(static_cast<unsigned char>(lastWord >> 24));

            preservedX8Register.restore();
            preservedX9Register.restore();

            return output;

        } catch (const Exceptions::Exception&) {
            preservedX8Register.restoreOnce();
            preservedX9Register.restoreOnce();
            throw;
        }
    }

    void DebugTranslator::writeMemoryViaProgramBuffer(
        Targets::TargetMemoryAddress startAddress,
        Targets::TargetMemoryBufferSpan buffer
    ) {
        assert(startAddress % DebugTranslator::WORD_BYTE_SIZE == 0);
        assert(buffer.size() % DebugTranslator::WORD_BYTE_SIZE == 0);

        static constexpr auto programOpcodes = std::to_array<Opcodes::Opcode>({
            Opcodes::Sw{
                .baseAddressRegister = Opcodes::GprNumber::X8,
                .valueRegister = Opcodes::GprNumber::X9,
                .addressOffset = 0
            }.opcode(),
            Opcodes::Addi{
                .destinationRegister = Opcodes::GprNumber::X8,
                .sourceRegister = Opcodes::GprNumber::X8,
                .value = DebugTranslator::WORD_BYTE_SIZE
            }.opcode(),
            Opcodes::Ebreak,
        });

        if (programOpcodes.size() > this->debugModuleDescriptor.programBufferSize) {
            throw Exceptions::TargetOperationFailure{
                "Cannot write to memory via RISC-V debug module program buffer - insufficient program buffer size"
            };
        }

        auto preservedX8Register = PreservedCpuRegister{CpuRegisterNumber::GPR_X8, *this};
        auto preservedX9Register = PreservedCpuRegister{CpuRegisterNumber::GPR_X9, *this};

        try {
            this->writeProgramBuffer(programOpcodes);
            this->writeCpuRegister(CpuRegisterNumber::GPR_X8, startAddress, {.postExecute = false});

            this->writeCpuRegister(
                CpuRegisterNumber::GPR_X9,
                static_cast<RegisterValue>(
                    (buffer[3] << 24)
                    | (buffer[2] << 16)
                    | (buffer[1] << 8)
                    | (buffer[0])
                ),
                {.postExecute = true}
            );

            this->dtmInterface.writeDebugModuleRegister(
                RegisterAddress::ABSTRACT_COMMAND_AUTO_EXECUTE_REGISTER,
                AbstractCommandAutoExecuteRegister{.onData0Access = true}.value()
            );

            for (
                auto offset = std::size_t{DebugTranslator::WORD_BYTE_SIZE};
                offset < buffer.size();
                offset += DebugTranslator::WORD_BYTE_SIZE
            ) {
                this->dtmInterface.writeDebugModuleRegister(
                    RegisterAddress::ABSTRACT_DATA_0,
                    static_cast<RegisterValue>(
                        (buffer[offset + 3] << 24)
                        | (buffer[offset + 2] << 16)
                        | (buffer[offset + 1] << 8)
                        | (buffer[offset])
                    )
                );
            }

            this->dtmInterface.writeDebugModuleRegister(
                RegisterAddress::ABSTRACT_COMMAND_AUTO_EXECUTE_REGISTER,
                AbstractCommandAutoExecuteRegister{}.value()
            );

            const auto commandError = this->readAndClearAbstractCommandError();
            if (commandError != AbstractCommandError::NONE) {
                if (commandError == AbstractCommandError::EXCEPTION) {
                    throw Exceptions::IllegalMemoryAccess{};
                }

                throw Exceptions::TargetOperationFailure{
                    "Program buffer execution failed - abstract command error: 0x"
                        + Services::StringService::toHex(commandError)
                };
            }

            preservedX8Register.restore();
            preservedX9Register.restore();

        } catch (const Exceptions::Exception&) {
            preservedX8Register.restoreOnce();
            preservedX9Register.restoreOnce();
            throw;
        }
    }

    void DebugTranslator::writeProgramBuffer(std::span<const Targets::RiscV::Opcodes::Opcode> opcodes) {
        assert(opcodes.size() <= 16);
        assert(opcodes.size() <= this->debugModuleDescriptor.programBufferSize);

        auto programBufferAddress = static_cast<DebugModule::RegisterAddress>(RegisterAddress::PROGRAM_BUFFER_0);
        for (const auto& opcode : opcodes) {
            this->dtmInterface.writeDebugModuleRegister(programBufferAddress, opcode);
            ++programBufferAddress;
        }
    }

    std::optional<
        std::reference_wrapper<const TriggerModule::TriggerDescriptor>
    > DebugTranslator::getAvailableTrigger() {
        for (const auto& [index, descriptor] : this->debugModuleDescriptor.triggerDescriptorsByIndex) {
            if (this->allocatedTriggerIndices.contains(index)) {
                continue;
            }

            return descriptor;
        }

        return std::nullopt;
    }

    void DebugTranslator::clearTrigger(const TriggerModule::TriggerDescriptor& triggerDescriptor) {
        using TriggerModule::TriggerType;

        Logger::debug("Clearing RISC-V trigger (index: " + std::to_string(triggerDescriptor.index) + ")");

        if (triggerDescriptor.supportedTypes.contains(TriggerType::MATCH_CONTROL)) {
            using TriggerModule::Registers::MatchControl;

            this->writeCpuRegister(
                CpuRegisterNumber::TRIGGER_SELECT,
                TriggerModule::Registers::TriggerSelect{triggerDescriptor.index}.value()
            );

            this->writeCpuRegister(CpuRegisterNumber::TRIGGER_DATA_1, MatchControl{}.value());
            return;
        }

        throw Exceptions::TargetOperationFailure{"Unsupported trigger"};
    }

    DebugTranslator::PreservedCpuRegister::PreservedCpuRegister(
        Registers::CpuRegisterNumber registerNumber,
        RegisterValue value,
        DebugTranslator& debugTranslator
    )
        : registerNumber(registerNumber)
        , value(value)
        , debugTranslator(debugTranslator)
    {}

    DebugTranslator::PreservedCpuRegister::PreservedCpuRegister(
        Registers::CpuRegisterNumber registerNumber,
        DebugTranslator& debugTranslator
    )
        : PreservedCpuRegister(
            registerNumber,
            debugTranslator.readCpuRegister(registerNumber),
            debugTranslator
        )
    {}

    void DebugTranslator::PreservedCpuRegister::restore() {
        try {
            this->debugTranslator.writeCpuRegister(this->registerNumber, this->value);
            this->restored = true;

        } catch (const Exceptions::Exception& exception) {
            /*
             * If we fail to restore the value of a CPU register, we must escalate this to a fatal error, as the target
             * will be left in an undefined state. More specifically, the state of the program running on the target
             * may be corrupted. We cannot recover from this.
             *
             * TargetFailure exceptions are considered to be fatal. A clean shutdown will follow.
             */
            throw Exceptions::TargetFailure{
                "Failed to restore CPU register (number: 0x"
                    + Services::StringService::toHex(this->registerNumber) + ") - error: " + exception.getMessage()
                    + " - the target is now in an undefined state and may require a reset"
            };
        }
    }

    void DebugTranslator::PreservedCpuRegister::restoreOnce() {
        if (this->restored) {
            return;
        }

        this->restore();
    }
}
