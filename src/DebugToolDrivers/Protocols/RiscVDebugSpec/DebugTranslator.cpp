#include "DebugTranslator.hpp"

#include <string>
#include <thread>
#include <chrono>
#include <limits>
#include <cassert>

#include "Registers/CpuRegisterNumbers.hpp"
#include "DebugModule/Registers/RegisterAddresses.hpp"
#include "DebugModule/Registers/RegisterAccessControlField.hpp"
#include "DebugModule/Registers/MemoryAccessControlField.hpp"

#include "TriggerModule/Registers/TriggerSelect.hpp"
#include "TriggerModule/Registers/TriggerInfo.hpp"
#include "TriggerModule/Registers/TriggerData1.hpp"
#include "TriggerModule/Registers/MatchControl.hpp"

#include "src/Exceptions/Exception.hpp"
#include "src/Exceptions/InvalidConfig.hpp"
#include "src/TargetController/Exceptions/DeviceInitializationFailure.hpp"
#include "src/TargetController/Exceptions/TargetOperationFailure.hpp"

#include "src/Logger/Logger.hpp"

namespace DebugToolDrivers::Protocols::RiscVDebugSpec
{
    using Registers::DebugControlStatusRegister;

    using DebugModule::Registers::RegisterAddress;
    using DebugModule::Registers::ControlRegister;
    using DebugModule::Registers::StatusRegister;
    using DebugModule::Registers::AbstractControlStatusRegister;
    using DebugModule::Registers::AbstractCommandRegister;

    using Registers::CpuRegisterNumber;

    using namespace ::Targets::RiscV;

    using ::Targets::TargetExecutionState;
    using ::Targets::TargetMemoryAddress;
    using ::Targets::TargetMemoryAddressRange;
    using ::Targets::TargetMemorySize;
    using ::Targets::TargetMemoryBuffer;
    using ::Targets::TargetStackPointer;
    using ::Targets::TargetAddressSpaceDescriptor;
    using ::Targets::TargetMemorySegmentDescriptor;
    using ::Targets::TargetRegisterDescriptors;
    using ::Targets::TargetRegisterDescriptorAndValuePairs;

    DebugTranslator::DebugTranslator(
        DebugTransportModuleInterface& dtmInterface,
        const TargetDescriptionFile& targetDescriptionFile,
        const RiscVTargetConfig& targetConfig
    )
        : dtmInterface(dtmInterface)
        , targetDescriptionFile(targetDescriptionFile)
        , targetConfig(targetConfig)
    {}

    void DebugTranslator::init() {
        // No pre-activation initialisation required.
        return;
    }

    void DebugTranslator::activate() {
        this->dtmInterface.activate();

        this->hartIndices = this->discoverHartIndices();
        if (this->hartIndices.empty()) {
            throw Exceptions::TargetOperationFailure{"Failed to discover any RISC-V harts"};
        }

        Logger::debug("Discovered RISC-V harts: " + std::to_string(this->hartIndices.size()));

        /*
         * We only support debugging a single RISC-V hart, for now.
         *
         * If we discover more than one, we select the first one and ensure that this is explicitly communicated to the
         * user.
         */
        if (this->hartIndices.size() > 1) {
            Logger::warning("Bloom only supports debugging a single RISC-V hart - selecting first available");
        }

        this->selectedHartIndex = this->hartIndices.front();
        Logger::info("Selected RISC-V hart index: " + std::to_string(this->selectedHartIndex));

        /*
         * Disabling the debug module before enabling it will clear any state from a previous debug session that
         * wasn't terminated properly.
         */
        this->disableDebugModule();
        this->enableDebugModule();

        this->stop();
        this->reset();
        this->triggerDescriptorsByIndex = this->discoverTriggers();

        Logger::debug("Discovered RISC-V triggers: " + std::to_string(this->triggerDescriptorsByIndex.size()));

        if (!this->triggerDescriptorsByIndex.empty()) {
            // Clear any left-over triggers from the previous debug session
            this->clearAllBreakpoints();
        }

        auto debugControlStatusRegister = this->readDebugControlStatusRegister();
        debugControlStatusRegister.breakUMode = true;
        debugControlStatusRegister.breakSMode = true;
        debugControlStatusRegister.breakMMode = true;

        this->writeDebugControlStatusRegister(debugControlStatusRegister);
    }

    void DebugTranslator::deactivate() {
        this->disableDebugModule();
        this->dtmInterface.deactivate();
    }

    TargetExecutionState DebugTranslator::getExecutionState() {
        return this->readDebugModuleStatusRegister().anyRunning
            ? TargetExecutionState::RUNNING
            : TargetExecutionState::STOPPED;
    }

    void DebugTranslator::stop() {
        auto controlRegister = ControlRegister{};
        controlRegister.debugModuleActive = true;
        controlRegister.selectedHartIndex = this->selectedHartIndex;
        controlRegister.haltRequest = true;

        this->writeDebugModuleControlRegister(controlRegister);

        constexpr auto maxAttempts = 10;
        auto statusRegister = this->readDebugModuleStatusRegister();

        for (auto attempts = 1; !statusRegister.allHalted && attempts <= maxAttempts; ++attempts) {
            std::this_thread::sleep_for(std::chrono::microseconds{10});
            statusRegister = this->readDebugModuleStatusRegister();
        }

        controlRegister.haltRequest = false;
        this->writeDebugModuleControlRegister(controlRegister);

        if (!statusRegister.allHalted) {
            throw Exceptions::Exception{"Target took too long to halt selected harts"};
        }
    }

    void DebugTranslator::run() {
        auto controlRegister = ControlRegister{};
        controlRegister.debugModuleActive = true;
        controlRegister.selectedHartIndex = this->selectedHartIndex;
        controlRegister.resumeRequest = true;

        this->writeDebugModuleControlRegister(controlRegister);

        constexpr auto maxAttempts = 10;
        auto statusRegister = this->readDebugModuleStatusRegister();

        for (auto attempts = 1; !statusRegister.allResumeAcknowledge && attempts <= maxAttempts; ++attempts) {
            std::this_thread::sleep_for(std::chrono::microseconds{10});
            statusRegister = this->readDebugModuleStatusRegister();
        }

        controlRegister.resumeRequest = false;
        this->writeDebugModuleControlRegister(controlRegister);

        if (!statusRegister.allResumeAcknowledge) {
            throw Exceptions::Exception{"Target took too long to acknowledge resume request"};
        }
    }

    void DebugTranslator::step() {
        auto debugControlStatusRegister = this->readDebugControlStatusRegister();
        debugControlStatusRegister.step = true;

        this->writeDebugControlStatusRegister(debugControlStatusRegister);

        auto controlRegister = ControlRegister{};
        controlRegister.debugModuleActive = true;
        controlRegister.selectedHartIndex = this->selectedHartIndex;
        controlRegister.resumeRequest = true;

        this->writeDebugModuleControlRegister(controlRegister);

        controlRegister.resumeRequest = false;
        this->writeDebugModuleControlRegister(controlRegister);

        debugControlStatusRegister.step = false;
        this->writeDebugControlStatusRegister(debugControlStatusRegister);
    }

    void DebugTranslator::reset() {
        auto controlRegister = ControlRegister{};
        controlRegister.debugModuleActive = true;
        controlRegister.selectedHartIndex = this->selectedHartIndex;
        controlRegister.setResetHaltRequest = true;
        controlRegister.haltRequest = true;
        controlRegister.ndmReset = true;

        this->writeDebugModuleControlRegister(controlRegister);

        controlRegister.ndmReset = false;
        this->writeDebugModuleControlRegister(controlRegister);

        constexpr auto maxAttempts = 10;
        auto statusRegister = this->readDebugModuleStatusRegister();

        for (auto attempts = 1; !statusRegister.allHaveReset && attempts <= maxAttempts; ++attempts) {
            std::this_thread::sleep_for(std::chrono::microseconds{10});
            statusRegister = this->readDebugModuleStatusRegister();
        }

        controlRegister = ControlRegister{};
        controlRegister.debugModuleActive = true;
        controlRegister.selectedHartIndex = this->selectedHartIndex;
        controlRegister.clearResetHaltRequest = true;
        controlRegister.acknowledgeHaveReset = true;

        this->writeDebugModuleControlRegister(controlRegister);

        if (!statusRegister.allHaveReset) {
            throw Exceptions::Exception{"Target took too long to reset"};
        }
    }

    void DebugTranslator::setSoftwareBreakpoint(TargetMemoryAddress address) {
        throw Exceptions::Exception{"SW breakpoints not supported"};
    }

    void DebugTranslator::clearSoftwareBreakpoint(TargetMemoryAddress address) {
        throw Exceptions::Exception{"SW breakpoints not supported"};
    }

    std::uint16_t DebugTranslator::getHardwareBreakpointCount() {
        return static_cast<std::uint16_t>(this->triggerDescriptorsByIndex.size());
    }

    void DebugTranslator::setHardwareBreakpoint(TargetMemoryAddress address) {
        using TriggerModule::TriggerType;

        const auto triggerDescriptorOpt = this->getAvailableTrigger();
        if (!triggerDescriptorOpt.has_value()) {
            throw Exceptions::Exception{"Insufficient resources - no available trigger"};
        }

        const auto& triggerDescriptor = triggerDescriptorOpt->get();
        Logger::debug("Installing hardware BP at address " + Services::StringService::toHex(address) + " with index " + std::to_string(triggerDescriptor.index));

        if (triggerDescriptor.supportedTypes.contains(TriggerType::MATCH_CONTROL)) {
            using TriggerModule::Registers::MatchControl;

            this->writeCpuRegister(
                CpuRegisterNumber::TRIGGER_SELECT,
                TriggerModule::Registers::TriggerSelect{triggerDescriptor.index}.value()
            );

            auto matchControlRegister = MatchControl{};
            matchControlRegister.execute = true;
            matchControlRegister.enabledInUserMode = true;
            matchControlRegister.enabledInSupervisorMode = true;
            matchControlRegister.enabledInMachineMode = true;
            matchControlRegister.action = TriggerModule::TriggerAction::ENTER_DEBUG_MODE;
            matchControlRegister.accessSize = MatchControl::AccessSize::ANY;
            matchControlRegister.compareValueType = MatchControl::CompareValueType::ADDRESS;

            this->writeCpuRegister(CpuRegisterNumber::TRIGGER_DATA_1, matchControlRegister.value());
            this->writeCpuRegister(CpuRegisterNumber::TRIGGER_DATA_2, address);

            this->allocatedTriggerIndices.emplace(triggerDescriptor.index);
            this->triggerIndicesByBreakpointAddress.emplace(address, triggerDescriptor.index);
            return;
        }

        throw Exceptions::Exception{"Unsupported trigger"};
    }

    void DebugTranslator::clearHardwareBreakpoint(TargetMemoryAddress address) {
        const auto triggerIndexIt = this->triggerIndicesByBreakpointAddress.find(address);
        if (triggerIndexIt == this->triggerIndicesByBreakpointAddress.end()) {
            throw Exceptions::Exception{"Unknown hardware breakpoint"};
        }

        const auto& triggerDescriptor = this->triggerDescriptorsByIndex.at(triggerIndexIt->second);

        this->clearTrigger(triggerDescriptor);
        this->triggerIndicesByBreakpointAddress.erase(address);
        this->allocatedTriggerIndices.erase(triggerDescriptor.index);
    }

    void DebugTranslator::clearAllBreakpoints() {
        // To ensure that any untracked breakpoints are cleared, we clear all triggers on the target.
        for (const auto [triggerIndex, triggerDescriptor] : this->triggerDescriptorsByIndex) {
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
            assert((value.size() * 8) > std::numeric_limits<RegisterValue>::digits);

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
        using DebugModule::Registers::MemoryAccessControlField;

        // TODO: excluded addresses

        const auto pageSize = 4;
        if ((startAddress % pageSize) != 0 || (bytes % pageSize) != 0) {
            // Alignment required
            const auto alignedStartAddress = this->alignMemoryAddress(startAddress, pageSize);
            const auto alignedBytes = this->alignMemorySize(bytes + (startAddress - alignedStartAddress), pageSize);

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

        auto output = TargetMemoryBuffer{};
        output.reserve(bytes);

        /*
         * We only need to set the address once. No need to update it as we use the post-increment function to
         * increment the address. See MemoryAccessControlField::postIncrement
         */
        this->dtmInterface.writeDebugModuleRegister(RegisterAddress::ABSTRACT_DATA_1, startAddress);

        constexpr auto command = AbstractCommandRegister{
            MemoryAccessControlField{
                false,
                true,
                MemoryAccessControlField::MemorySize::SIZE_32,
                false
            }.value(),
            AbstractCommandRegister::CommandType::MEMORY_ACCESS
        };

        for (auto address = startAddress; address <= (startAddress + bytes - 1); address += 4) {
            this->executeAbstractCommand(command);

            const auto data = this->dtmInterface.readDebugModuleRegister(RegisterAddress::ABSTRACT_DATA_0);
            output.emplace_back(static_cast<unsigned char>(data));
            output.emplace_back(static_cast<unsigned char>(data >> 8));
            output.emplace_back(static_cast<unsigned char>(data >> 16));
            output.emplace_back(static_cast<unsigned char>(data >> 24));
        }

        return output;
    }

    void DebugTranslator::writeMemory(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        TargetMemoryAddress startAddress,
        const TargetMemoryBuffer& buffer
    ) {
        using DebugModule::Registers::MemoryAccessControlField;

        constexpr auto alignTo = TargetMemorySize{4};
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
                    (startAddress - alignedStartAddress)
                )
                : TargetMemoryBuffer{};
            alignedBuffer.reserve(alignedBytes);

            // Read the offset bytes required to align the buffer size
            const auto dataBack = this->readMemory(
                addressSpaceDescriptor,
                memorySegmentDescriptor,
                startAddress + bytes,
                alignedBytes - bytes - (startAddress - alignedStartAddress)
            );
            alignedBuffer.insert(alignedBuffer.end(), dataBack.begin(), dataBack.end());

            return this->writeMemory(
                addressSpaceDescriptor,
                memorySegmentDescriptor,
                alignedStartAddress,
                alignedBuffer
            );
        }

        this->dtmInterface.writeDebugModuleRegister(RegisterAddress::ABSTRACT_DATA_1, startAddress);

        constexpr auto command = AbstractCommandRegister{
            MemoryAccessControlField{
                true,
                true,
                MemoryAccessControlField::MemorySize::SIZE_32,
                false
            }.value(),
            AbstractCommandRegister::CommandType::MEMORY_ACCESS
        };

        for (TargetMemoryAddress offset = 0; offset < buffer.size(); offset += 4) {
            this->dtmInterface.writeDebugModuleRegister(
                RegisterAddress::ABSTRACT_DATA_0,
                static_cast<RegisterValue>(
                    (buffer[offset + 3] << 24)
                    | (buffer[offset + 2] << 16)
                    | (buffer[offset + 1] << 8)
                    | (buffer[offset])
                )
            );

            this->executeAbstractCommand(command);
        }
    }

    std::vector<DebugModule::HartIndex> DebugTranslator::discoverHartIndices() {
        auto hartIndices = std::vector<DebugModule::HartIndex>{};

        /*
         * We can obtain the maximum hart index by setting all of the hartsel bits in the control register and then
         * read the value back.
         */
        auto controlRegister = ControlRegister{};
        controlRegister.debugModuleActive = true;
        controlRegister.selectedHartIndex = 0xFFFFF;

        this->writeDebugModuleControlRegister(controlRegister);
        const auto maxHartIndex = this->readDebugModuleControlRegister().selectedHartIndex;

        for (auto hartIndex = DebugModule::HartIndex{0}; hartIndex <= maxHartIndex; ++hartIndex) {
            /*
             * We can't just assume that everything between 0 and the maximum hart index are valid hart indices. We
             * have to test each index until we find one that is non-existent.
             */
            auto controlRegister = ControlRegister{};
            controlRegister.debugModuleActive = true;
            controlRegister.selectedHartIndex = hartIndex;

            this->writeDebugModuleControlRegister(controlRegister);

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
            if (writeSelectError == DebugModule::AbstractCommandError::EXCEPTION) {
                break;
            }

            if (writeSelectError != DebugModule::AbstractCommandError::NONE) {
                throw Exceptions::Exception{
                    "Failed to write to TRIGGER_SELECT register - abstract command error: 0x"
                        + Services::StringService::toHex(writeSelectError)
                };
            }

            if (this->readCpuRegister(CpuRegisterNumber::TRIGGER_SELECT) != selectRegValue) {
                break;
            }

            const auto infoReg = TriggerModule::Registers::TriggerInfo{
                this->readCpuRegister(CpuRegisterNumber::TRIGGER_INFO)
            };

            if (infoReg.info == 0x01) {
                // Trigger doesn't exist
                break;
            }

            auto supportedTypes = infoReg.getSupportedTriggerTypes();
            if (supportedTypes.empty()) {
                // The trigger info register has no trigger type info. Try the data1 register.
                const auto data1Reg = TriggerModule::Registers::TriggerData1{
                    this->readCpuRegister(CpuRegisterNumber::TRIGGER_DATA_1)
                };

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
        return ControlRegister{this->dtmInterface.readDebugModuleRegister(RegisterAddress::CONTROL_REGISTER)};
    }

    StatusRegister DebugTranslator::readDebugModuleStatusRegister() {
        return StatusRegister{this->dtmInterface.readDebugModuleRegister(RegisterAddress::STATUS_REGISTER)};
    }

    AbstractControlStatusRegister DebugTranslator::readDebugModuleAbstractControlStatusRegister() {
        return AbstractControlStatusRegister{
            this->dtmInterface.readDebugModuleRegister(RegisterAddress::ABSTRACT_CONTROL_STATUS_REGISTER)
        };
    }

    DebugControlStatusRegister DebugTranslator::readDebugControlStatusRegister() {
        return DebugControlStatusRegister{
            this->readCpuRegister(static_cast<RegisterNumber>(CpuRegisterNumber::DEBUG_CONTROL_STATUS_REGISTER))
        };
    }

    void DebugTranslator::enableDebugModule() {
        auto controlRegister = ControlRegister{};
        controlRegister.debugModuleActive = true;
        controlRegister.selectedHartIndex = this->selectedHartIndex;

        this->writeDebugModuleControlRegister(controlRegister);

        constexpr auto maxAttempts = 10;
        controlRegister = this->readDebugModuleControlRegister();

        for (auto attempts = 1; !controlRegister.debugModuleActive && attempts <= maxAttempts; ++attempts) {
            std::this_thread::sleep_for(std::chrono::microseconds{10});
            controlRegister = this->readDebugModuleControlRegister();
        }

        if (!controlRegister.debugModuleActive) {
            throw Exceptions::Exception{"Took too long to enable debug module"};
        }
    }

    void DebugTranslator::disableDebugModule() {
        auto controlRegister = ControlRegister{};
        controlRegister.debugModuleActive = false;
        controlRegister.selectedHartIndex = this->selectedHartIndex;

        this->writeDebugModuleControlRegister(controlRegister);

        constexpr auto maxAttempts = 10;
        controlRegister = this->readDebugModuleControlRegister();

        for (auto attempts = 1; controlRegister.debugModuleActive && attempts <= maxAttempts; ++attempts) {
            std::this_thread::sleep_for(std::chrono::microseconds{10});
            controlRegister = this->readDebugModuleControlRegister();
        }

        if (controlRegister.debugModuleActive) {
            throw Exceptions::Exception{"Took too long to disable debug module"};
        }
    }

    Expected<RegisterValue, DebugModule::AbstractCommandError> DebugTranslator::tryReadCpuRegister(
        RegisterNumber number
    ) {
        using DebugModule::Registers::RegisterAccessControlField;

        const auto commandError = this->tryExecuteAbstractCommand(AbstractCommandRegister{
            RegisterAccessControlField{
                number,
                false,
                true,
                false,
                false,
                RegisterAccessControlField::RegisterSize::SIZE_32
            }.value(),
            AbstractCommandRegister::CommandType::REGISTER_ACCESS
        });

        if (commandError != DebugModule::AbstractCommandError::NONE) {
            return commandError;
        }

        return this->dtmInterface.readDebugModuleRegister(RegisterAddress::ABSTRACT_DATA_0);
    }

    Expected<RegisterValue, DebugModule::AbstractCommandError> DebugTranslator::tryReadCpuRegister(
        Registers::CpuRegisterNumber number
    ) {
        return this->tryReadCpuRegister(static_cast<RegisterNumber>(number));
    }

    RegisterValue DebugTranslator::readCpuRegister(RegisterNumber number) {
        const auto result = this->tryReadCpuRegister(number);

        if (!result.hasValue()) {
            throw Exceptions::Exception{
                "Failed to read CPU register (number: 0x" + Services::StringService::toHex(number)
                    + ") - abstract command error: 0x" + Services::StringService::toHex(result.error())
            };
        }

        return result.value();
    }

    RegisterValue DebugTranslator::readCpuRegister(Registers::CpuRegisterNumber number) {
        return this->readCpuRegister(static_cast<RegisterNumber>(number));
    }

    DebugModule::AbstractCommandError DebugTranslator::tryWriteCpuRegister(
        RegisterNumber number,
        RegisterValue value
    ) {
        using DebugModule::Registers::RegisterAccessControlField;

        this->dtmInterface.writeDebugModuleRegister(RegisterAddress::ABSTRACT_DATA_0, value);
        return this->tryExecuteAbstractCommand(AbstractCommandRegister{
            RegisterAccessControlField{
                number,
                true,
                true,
                false,
                false,
                RegisterAccessControlField::RegisterSize::SIZE_32
            }.value(),
            AbstractCommandRegister::CommandType::REGISTER_ACCESS
        });
    }

    DebugModule::AbstractCommandError DebugTranslator::tryWriteCpuRegister(
        Registers::CpuRegisterNumber number,
        RegisterValue value
    ) {
        return this->tryWriteCpuRegister(static_cast<RegisterNumber>(number), value);
    }

    void DebugTranslator::writeCpuRegister(RegisterNumber number, RegisterValue value) {
        const auto commandError = this->tryWriteCpuRegister(number, value);
        if (commandError != DebugModule::AbstractCommandError::NONE) {
            throw Exceptions::Exception{
                "Failed to write to CPU register (number: 0x" + Services::StringService::toHex(number)
                    + ") - abstract command error: 0x" + Services::StringService::toHex(commandError)
            };
        }
    }

    void DebugTranslator::writeCpuRegister(Registers::CpuRegisterNumber number, RegisterValue value) {
        this->writeCpuRegister(static_cast<RegisterNumber>(number), value);
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

    DebugModule::AbstractCommandError DebugTranslator::tryExecuteAbstractCommand(
        const DebugModule::Registers::AbstractCommandRegister& abstractCommandRegister
    ) {
        this->dtmInterface.writeDebugModuleRegister(
            RegisterAddress::ABSTRACT_COMMAND_REGISTER,
            abstractCommandRegister.value()
        );

        auto abstractStatusRegister = this->readDebugModuleAbstractControlStatusRegister();
        if (abstractStatusRegister.commandError != DebugModule::AbstractCommandError::NONE) {
            return abstractStatusRegister.commandError;
        }

        constexpr auto maxAttempts = 10;
        for (auto attempts = 1; abstractStatusRegister.busy && attempts <= maxAttempts; ++attempts) {
            std::this_thread::sleep_for(std::chrono::microseconds{10});
            abstractStatusRegister = this->readDebugModuleAbstractControlStatusRegister();
        }

        if (abstractStatusRegister.busy) {
            throw Exceptions::Exception{"Abstract command took too long to execute"};
        }

        return abstractStatusRegister.commandError;
    }

    void DebugTranslator::executeAbstractCommand(
        const DebugModule::Registers::AbstractCommandRegister& abstractCommandRegister
    ) {
        const auto commandError = this->tryExecuteAbstractCommand(abstractCommandRegister);
        if (commandError != DebugModule::AbstractCommandError::NONE) {
            throw Exceptions::Exception{
                "Failed to execute abstract command - error: 0x" + Services::StringService::toHex(commandError)
            };
        }
    }

    TargetMemoryAddress DebugTranslator::alignMemoryAddress(TargetMemoryAddress address, TargetMemoryAddress alignTo) {
        return (address / alignTo) * alignTo;
    }

    TargetMemorySize DebugTranslator::alignMemorySize(TargetMemorySize size, TargetMemorySize alignTo) {
        return static_cast<TargetMemorySize>(
            std::ceil(static_cast<double>(size) / static_cast<double>(alignTo))
        ) * alignTo;
    }

    std::optional<
        std::reference_wrapper<const TriggerModule::TriggerDescriptor>
    > DebugTranslator::getAvailableTrigger() {
        for (const auto& [index, descriptor] : this->triggerDescriptorsByIndex) {
            if (this->allocatedTriggerIndices.contains(index)) {
                continue;
            }

            return descriptor;
        }

        return std::nullopt;
    }

    void DebugTranslator::clearTrigger(const TriggerModule::TriggerDescriptor& triggerDescriptor) {
        using TriggerModule::TriggerType;

        Logger::debug("Clearing RISC-V trigger " + std::to_string(triggerDescriptor.index));

        if (triggerDescriptor.supportedTypes.contains(TriggerType::MATCH_CONTROL)) {
            using TriggerModule::Registers::MatchControl;

            this->writeCpuRegister(
                CpuRegisterNumber::TRIGGER_SELECT,
                TriggerModule::Registers::TriggerSelect{triggerDescriptor.index}.value()
            );

            this->writeCpuRegister(CpuRegisterNumber::TRIGGER_DATA_1, MatchControl{}.value());
            return;
        }

        throw Exceptions::Exception{"Unsupported trigger"};
    }
}
