#include "RiscV.hpp"

#include <thread>
#include <chrono>
#include <limits>
#include <cmath>

#include "Registers/RegisterNumbers.hpp"
#include "DebugModule/Registers/RegisterAddresses.hpp"
#include "DebugModule/Registers/RegisterAccessControlField.hpp"
#include "DebugModule/Registers/MemoryAccessControlField.hpp"

#include "src/Exceptions/Exception.hpp"
#include "src/TargetController/Exceptions/TargetOperationFailure.hpp"

#include "src/Services/StringService.hpp"

#include "src/Logger/Logger.hpp"

namespace Targets::RiscV
{
    using Registers::DebugControlStatusRegister;

    using DebugModule::Registers::RegisterAddress;
    using DebugModule::Registers::ControlRegister;
    using DebugModule::Registers::StatusRegister;
    using DebugModule::Registers::AbstractControlStatusRegister;
    using DebugModule::Registers::AbstractCommandRegister;

    RiscV::RiscV(const TargetConfig& targetConfig)
        : name("CH32X035C8T6") // TODO: TDF
        , stackPointerRegisterDescriptor(
            RiscVRegisterDescriptor(
                TargetRegisterType::STACK_POINTER,
                static_cast<RegisterNumber>(Registers::RegisterNumber::STACK_POINTER_X2),
                4,
                TargetMemoryType::OTHER,
                "SP",
                "CPU",
                "Stack Pointer Register",
                TargetRegisterAccess(true, true)
            )
        )
    {
        this->loadRegisterDescriptors();
    }

    bool RiscV::supportsDebugTool(DebugTool* debugTool) {
        return debugTool->getRiscVDebugInterface() != nullptr;
    }

    void RiscV::setDebugTool(DebugTool* debugTool) {
        this->riscVDebugInterface = debugTool->getRiscVDebugInterface();
    }

    void RiscV::activate() {
        this->riscVDebugInterface->activate({});

        this->hartIndices = this->discoverHartIndices();

        if (this->hartIndices.empty()) {
            throw Exceptions::TargetOperationFailure("Failed to discover a single RISC-V hart");
        }

        Logger::debug("Discovered RISC-V harts: " + std::to_string(this->hartIndices.size()));

        /*
         * We only support MCUs with a single hart, for now. So select the first index and ensure that this is
         * explicitly communicated to the user.
         */
        if (this->hartIndices.size() > 1) {
            Logger::warning(
                "Bloom only supports debugging a single RISC-V hart - selecting first available hart"
            );
        }

        this->selectedHartIndex = *(this->hartIndices.begin());
        Logger::info("Selected RISC-V hart index: " + std::to_string(this->selectedHartIndex));

        /*
         * Disabling the debug module before enabling it will clear any state from a previous debug session that
         * wasn't terminated properly.
         */
        this->disableDebugModule();
        this->enableDebugModule();

        this->stop();

        auto debugControlStatusRegister = this->readDebugControlStatusRegister();
        debugControlStatusRegister.breakUMode = true;
        debugControlStatusRegister.breakSMode = true;
        debugControlStatusRegister.breakMMode = true;

        this->writeDebugControlStatusRegister(debugControlStatusRegister);
    }

    void RiscV::deactivate() {
        if (this->getState() != TargetState::RUNNING) {
            this->run();
        }

        this->disableDebugModule();
        this->riscVDebugInterface->deactivate();
    }

    TargetDescriptor RiscV::getDescriptor() {
        return TargetDescriptor(
            "TDF ID",
            TargetFamily::RISC_V,
            this->name,
            "TDF VENDOR NAME",
            {
                {
                    Targets::TargetMemoryType::FLASH,
                    TargetMemoryDescriptor(
                        Targets::TargetMemoryType::FLASH,
                        Targets::TargetMemoryAddressRange(0x00, 0x1000),
                        Targets::TargetMemoryAccess(true, true, false)
                    )
                }
            },
            {this->registerDescriptorsById.begin(), this->registerDescriptorsById.end()},
            BreakpointResources(0, 0, 0),
            {},
            TargetMemoryType::FLASH
        );
    }

    void RiscV::run(std::optional<TargetMemoryAddress> toAddress) {
        auto controlRegister = ControlRegister();
        controlRegister.debugModuleActive = true;
        controlRegister.selectedHartIndex = this->selectedHartIndex;
        controlRegister.resumeRequest = true;

        this->writeDebugModuleControlRegister(controlRegister);

        constexpr auto maxAttempts = 10;
        auto statusRegister = this->readDebugModuleStatusRegister();

        for (auto attempts = 1; !statusRegister.allResumeAcknowledge && attempts <= maxAttempts; ++attempts) {
            std::this_thread::sleep_for(std::chrono::microseconds(10));
            statusRegister = this->readDebugModuleStatusRegister();
        }

        controlRegister.resumeRequest = false;
        this->writeDebugModuleControlRegister(controlRegister);

        if (!statusRegister.allResumeAcknowledge) {
            throw Exceptions::Exception("Target took too long to acknowledge resume request");
        }
    }

    void RiscV::stop() {
        auto controlRegister = ControlRegister();
        controlRegister.debugModuleActive = true;
        controlRegister.selectedHartIndex = this->selectedHartIndex;
        controlRegister.haltRequest = true;

        this->writeDebugModuleControlRegister(controlRegister);

        constexpr auto maxAttempts = 10;
        auto statusRegister = this->readDebugModuleStatusRegister();

        for (auto attempts = 1; !statusRegister.allHalted && attempts <= maxAttempts; ++attempts) {
            std::this_thread::sleep_for(std::chrono::microseconds(10));
            statusRegister = this->readDebugModuleStatusRegister();
        }

        controlRegister.haltRequest = false;
        this->writeDebugModuleControlRegister(controlRegister);

        if (!statusRegister.allHalted) {
            throw Exceptions::Exception("Target took too long to halt selected harts");
        }
    }

    void RiscV::step() {
        auto debugControlStatusRegister = this->readDebugControlStatusRegister();
        debugControlStatusRegister.step = true;

        this->writeDebugControlStatusRegister(debugControlStatusRegister);

        auto controlRegister = ControlRegister();
        controlRegister.debugModuleActive = true;
        controlRegister.selectedHartIndex = this->selectedHartIndex;
        controlRegister.resumeRequest = true;

        this->writeDebugModuleControlRegister(controlRegister);

        controlRegister.resumeRequest = false;
        this->writeDebugModuleControlRegister(controlRegister);

        debugControlStatusRegister.step = false;
        this->writeDebugControlStatusRegister(debugControlStatusRegister);
    }

    void RiscV::reset() {

    }

    void RiscV::setSoftwareBreakpoint(TargetMemoryAddress address) {

    }

    void RiscV::removeSoftwareBreakpoint(TargetMemoryAddress address) {

    }

    void RiscV::setHardwareBreakpoint(TargetMemoryAddress address) {

    }

    void RiscV::removeHardwareBreakpoint(TargetMemoryAddress address) {

    }

    void RiscV::clearAllBreakpoints() {

    }

    TargetRegisters RiscV::readRegisters(const Targets::TargetRegisterDescriptorIds& descriptorIds) {
        auto output = TargetRegisters();

        for (const auto& descriptorId : descriptorIds) {
            const auto registerValue = this->readRegister(this->registerDescriptorsById.at(descriptorId).number);
            output.emplace_back(
                descriptorId,
                TargetMemoryBuffer({
                    static_cast<unsigned char>(registerValue >> 24),
                    static_cast<unsigned char>(registerValue >> 16),
                    static_cast<unsigned char>(registerValue >> 8),
                    static_cast<unsigned char>(registerValue),
                })
            );
        }

        return output;
    }

    void RiscV::writeRegisters(const TargetRegisters& registers) {
        for (const auto& targetRegister : registers) {
            if ((targetRegister.value.size() * 8) > std::numeric_limits<std::uintmax_t>::digits) {
                throw Exceptions::Exception("Register value bit width exceeds that of std::uintmax_t");
            }

            auto registerValue = std::uintmax_t{0};

            for (const auto& registerByte : targetRegister.value) {
                registerValue = (registerValue << 8) | registerByte;
            }

            this->writeRegister(
                this->registerDescriptorsById.at(targetRegister.descriptorId).number,
                static_cast<RegisterValue>(registerValue) // TODO: Support larger register values
            );
        }
    }

    TargetMemoryBuffer RiscV::readMemory(
        TargetMemoryType memoryType,
        TargetMemoryAddress startAddress,
        TargetMemorySize bytes,
        const std::set<Targets::TargetMemoryAddressRange>& excludedAddressRanges
    ) {
        // TODO: excluded addresses

        const auto pageSize = 4;
        if ((startAddress % pageSize) != 0 || (bytes % pageSize) != 0) {
            // Alignment required
            const auto alignedStartAddress = static_cast<TargetMemoryAddress>(
                std::floor(static_cast<float>(startAddress) / static_cast<float>(pageSize)) * pageSize
            );

            const auto alignedBytes = static_cast<TargetMemorySize>(
                std::ceil(
                    static_cast<float>(bytes + (startAddress - alignedStartAddress))
                        / static_cast<float>(pageSize)
                ) * pageSize
            );

            auto memoryBuffer = this->readMemory(
                memoryType,
                alignedStartAddress,
                alignedBytes,
                excludedAddressRanges
            );

            const auto offset = memoryBuffer.begin() + (startAddress - alignedStartAddress);

            auto output = TargetMemoryBuffer();
            output.reserve(bytes);
            std::move(offset, offset + bytes, std::back_inserter(output));

            return output;
        }

        using DebugModule::Registers::MemoryAccessControlField;

        auto output = TargetMemoryBuffer();
        output.reserve(bytes);

        /*
         * We only need to set the address once. No need to update it as we use the post-increment function to
         * increment the address. See MemoryAccessControlField::postIncrement
         */
        this->riscVDebugInterface->writeDebugModuleRegister(RegisterAddress::ABSTRACT_DATA_1, startAddress);

        auto command = AbstractCommandRegister();
        command.commandType = AbstractCommandRegister::CommandType::MEMORY_ACCESS;
        command.control = MemoryAccessControlField(
            false,
            true,
            MemoryAccessControlField::MemorySize::SIZE_32,
            false
        ).value();

        for (auto address = startAddress; address <= (startAddress + bytes - 1); address += 4) {
            this->executeAbstractCommand(command);

            const auto data = this->riscVDebugInterface->readDebugModuleRegister(RegisterAddress::ABSTRACT_DATA_0);
            output.emplace_back(static_cast<unsigned char>(data >> 24));
            output.emplace_back(static_cast<unsigned char>(data >> 16));
            output.emplace_back(static_cast<unsigned char>(data >> 8));
            output.emplace_back(static_cast<unsigned char>(data));
        }

        return output;
    }

    void RiscV::writeMemory(
        TargetMemoryType memoryType,
        TargetMemoryAddress startAddress,
        const TargetMemoryBuffer& buffer
    ) {

    }

    void RiscV::eraseMemory(TargetMemoryType memoryType) {

    }

    TargetState RiscV::getState() {
        return this->readDebugModuleStatusRegister().anyRunning ? TargetState::RUNNING : TargetState::STOPPED;
    }

    TargetMemoryAddress RiscV::getProgramCounter() {
        return this->readRegister(Registers::RegisterNumber::DEBUG_PROGRAM_COUNTER_REGISTER);
    }

    void RiscV::setProgramCounter(TargetMemoryAddress programCounter) {
        // TODO: test this
        this->writeRegister(Registers::RegisterNumber::DEBUG_PROGRAM_COUNTER_REGISTER, programCounter);
    }

    TargetStackPointer RiscV::getStackPointer() {
        return this->readRegister(Registers::RegisterNumber::STACK_POINTER_X2);
    }

    std::map<int, TargetPinState> RiscV::getPinStates(int variantId) {
        return {};
    }

    void RiscV::setPinState(
        const TargetPinDescriptor& pinDescriptor,
        const TargetPinState& state
    ) {

    }

    void RiscV::enableProgrammingMode() {

    }

    void RiscV::disableProgrammingMode() {

    }

    bool RiscV::programmingModeEnabled() {
        return false;
    }

    void RiscV::loadRegisterDescriptors() {
        for (std::uint8_t i = 0; i <= 31; i++) {
            auto generalPurposeRegisterDescriptor = RiscVRegisterDescriptor(
                TargetRegisterType::GENERAL_PURPOSE_REGISTER,
                static_cast<RegisterNumber>(Registers::RegisterNumberBase::GPR) + i,
                4,
                TargetMemoryType::OTHER,
                "x" + std::to_string(i),
                "CPU General Purpose",
                std::nullopt,
                TargetRegisterAccess(true, true)
            );

            this->registerDescriptorsById.emplace(
                generalPurposeRegisterDescriptor.id,
                std::move(generalPurposeRegisterDescriptor)
            );
        }
    }

    std::set<DebugModule::HartIndex> RiscV::discoverHartIndices() {
        auto hartIndices = std::set<DebugModule::HartIndex>();

        /*
         * We can obtain the maximum hart index by setting all of the hartsel bits in the control register and then
         * reading the value back.
         */
        auto controlRegister = ControlRegister();
        controlRegister.debugModuleActive = true;
        controlRegister.selectedHartIndex = 0xFFFFF;

        this->writeDebugModuleControlRegister(controlRegister);
        controlRegister = this->readDebugModuleControlRegister();

        for (DebugModule::HartIndex hartIndex = 0; hartIndex <= controlRegister.selectedHartIndex; ++hartIndex) {
            /*
             * We can't just assume that everything between 0 and the maximum hart index are valid hart indices. We
             * have to test each index until we find one that is non-existent.
             */
            controlRegister = ControlRegister();
            controlRegister.debugModuleActive = true;
            controlRegister.selectedHartIndex = hartIndex;

            this->writeDebugModuleControlRegister(controlRegister);

            /*
             * It's worth noting that some RISC-V targets **do not** set the non-existent flags. I'm not sure why.
             * Has hartsel been hardwired to 0 because they only support a single hart, preventing the selection
             * of non-existent harts?
             *
             * Relying on the maximum hart index seems to be all we can do in this case.
             */
            if (this->readDebugModuleStatusRegister().anyNonExistent) {
                break;
            }

            hartIndices.insert(hartIndex);
        }

        return hartIndices;
    }

    ControlRegister RiscV::readDebugModuleControlRegister() {
        return ControlRegister(
            this->riscVDebugInterface->readDebugModuleRegister(RegisterAddress::CONTROL_REGISTER)
        );
    }

    StatusRegister RiscV::readDebugModuleStatusRegister() {
        return StatusRegister(
            this->riscVDebugInterface->readDebugModuleRegister(RegisterAddress::STATUS_REGISTER)
        );
    }

    AbstractControlStatusRegister RiscV::readDebugModuleAbstractControlStatusRegister() {
        return AbstractControlStatusRegister(
            this->riscVDebugInterface->readDebugModuleRegister(RegisterAddress::ABSTRACT_CONTROL_STATUS_REGISTER)
        );
    }

    DebugControlStatusRegister RiscV::readDebugControlStatusRegister() {
        return DebugControlStatusRegister(this->readRegister(Registers::RegisterNumber::DEBUG_CONTROL_STATUS_REGISTER));
    }

    void RiscV::enableDebugModule() {
        auto controlRegister = ControlRegister();
        controlRegister.debugModuleActive = true;
        controlRegister.selectedHartIndex = this->selectedHartIndex;

        this->writeDebugModuleControlRegister(controlRegister);

        constexpr auto maxAttempts = 10;
        controlRegister = this->readDebugModuleControlRegister();

        for (auto attempts = 1; !controlRegister.debugModuleActive && attempts <= maxAttempts; ++attempts) {
            std::this_thread::sleep_for(std::chrono::microseconds(10));
            controlRegister = this->readDebugModuleControlRegister();
        }

        if (!controlRegister.debugModuleActive) {
            throw Exceptions::Exception("Took too long to enable debug module");
        }
    }

    void RiscV::disableDebugModule() {
        auto controlRegister = ControlRegister();
        controlRegister.debugModuleActive = false;
        controlRegister.selectedHartIndex = this->selectedHartIndex;

        this->writeDebugModuleControlRegister(controlRegister);

        constexpr auto maxAttempts = 10;
        controlRegister = this->readDebugModuleControlRegister();

        for (auto attempts = 1; controlRegister.debugModuleActive && attempts <= maxAttempts; ++attempts) {
            std::this_thread::sleep_for(std::chrono::microseconds(10));
            controlRegister = this->readDebugModuleControlRegister();
        }

        if (controlRegister.debugModuleActive) {
            throw Exceptions::Exception("Took too long to disable debug module");
        }
    }

    RegisterValue RiscV::readRegister(RegisterNumber number) {
        using DebugModule::Registers::RegisterAccessControlField;

        auto command = AbstractCommandRegister();
        command.commandType = AbstractCommandRegister::CommandType::REGISTER_ACCESS;
        command.control = RegisterAccessControlField(
            number,
            false,
            true,
            false,
            false,
            RegisterAccessControlField::RegisterSize::SIZE_32
        ).value();

        this->executeAbstractCommand(command);

        return this->riscVDebugInterface->readDebugModuleRegister(RegisterAddress::ABSTRACT_DATA_0);
    }

    RegisterValue RiscV::readRegister(Registers::RegisterNumber number) {
        return this->readRegister(static_cast<RegisterNumber>(number));
    }

    void RiscV::writeRegister(RegisterNumber number, RegisterValue value) {
        using DebugModule::Registers::RegisterAccessControlField;

        auto command = AbstractCommandRegister();
        command.commandType = AbstractCommandRegister::CommandType::REGISTER_ACCESS;
        command.control = RegisterAccessControlField(
            number,
            true,
            true,
            false,
            false,
            RegisterAccessControlField::RegisterSize::SIZE_32
        ).value();

        this->riscVDebugInterface->writeDebugModuleRegister(RegisterAddress::ABSTRACT_DATA_0, value);
        this->executeAbstractCommand(command);
    }

    void RiscV::writeRegister(Registers::RegisterNumber number, RegisterValue value) {
        this->writeRegister(
            static_cast<RegisterNumber>(number),
            value
        );
    }

    void RiscV::writeDebugModuleControlRegister(const DebugModule::Registers::ControlRegister& controlRegister) {
        this->riscVDebugInterface->writeDebugModuleRegister(
            RegisterAddress::CONTROL_REGISTER,
            controlRegister.value()
        );
    }

    void RiscV::writeDebugControlStatusRegister(const DebugControlStatusRegister& controlRegister) {
        this->writeRegister(Registers::RegisterNumber::DEBUG_CONTROL_STATUS_REGISTER, controlRegister.value());
    }

    void RiscV::executeAbstractCommand(
        const DebugModule::Registers::AbstractCommandRegister& abstractCommandRegister
    ) {
        this->riscVDebugInterface->writeDebugModuleRegister(
            RegisterAddress::ABSTRACT_COMMAND_REGISTER,
            abstractCommandRegister.value()
        );

        auto abstractStatusRegister = this->readDebugModuleAbstractControlStatusRegister();
        if (abstractStatusRegister.commandError != AbstractControlStatusRegister::CommandError::NONE) {
            throw Exceptions::Exception(
                "Failed to execute abstract command - error: "
                    + Services::StringService::toHex(abstractStatusRegister.commandError)
            );
        }

        constexpr auto maxAttempts = 10;
        for (auto attempts = 1; abstractStatusRegister.busy && attempts <= maxAttempts; ++attempts) {
            std::this_thread::sleep_for(std::chrono::microseconds(10));
            abstractStatusRegister = this->readDebugModuleAbstractControlStatusRegister();
        }

        if (abstractStatusRegister.busy) {
            throw Exceptions::Exception("Abstract command took too long to execute");
        }
    }
}
