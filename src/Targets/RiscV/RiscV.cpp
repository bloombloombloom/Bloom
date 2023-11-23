#include "RiscV.hpp"

#include <thread>
#include <chrono>

#include "DebugModule/Registers/RegisterAddresses.hpp"
#include "DebugModule/Registers/RegisterAccessControlField.hpp"

#include "src/Exceptions/Exception.hpp"
#include "src/TargetController/Exceptions/TargetOperationFailure.hpp"

#include "src/Logger/Logger.hpp"

namespace Targets::RiscV
{
    using Registers::RegisterNumber;
    using Registers::DebugControlStatusRegister;

    using DebugModule::Registers::RegisterAddresses;
    using DebugModule::Registers::ControlRegister;
    using DebugModule::Registers::StatusRegister;
    using DebugModule::Registers::AbstractControlStatusRegister;
    using DebugModule::Registers::AbstractCommandRegister;

    RiscV::RiscV(const TargetConfig& targetConfig)
        : name("CH32X035C8T6") // TODO: TDF
    {}

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

        this->stop();

        auto debugControlStatusRegister = this->readDebugControlStatusRegister();
        debugControlStatusRegister.breakUMode = true;
        debugControlStatusRegister.breakSMode = true;
        debugControlStatusRegister.breakMMode = true;

        this->writeDebugControlStatusRegister(debugControlStatusRegister);
    }

    void RiscV::deactivate() {
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
            {},
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

    void RiscV::writeRegisters(TargetRegisters registers) {

    }

    TargetRegisters RiscV::readRegisters(const Targets::TargetRegisterDescriptorIds& descriptorIds) {
        return {};
    }

    TargetMemoryBuffer RiscV::readMemory(
        TargetMemoryType memoryType,
        TargetMemoryAddress startAddress,
        TargetMemorySize bytes,
        const std::set<Targets::TargetMemoryAddressRange>& excludedAddressRanges
    ) {
        return {};
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
        return 0;
    }

    void RiscV::setProgramCounter(TargetMemoryAddress programCounter) {

    }

    TargetStackPointer RiscV::getStackPointer() {
        return 0;
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
            this->riscVDebugInterface->readDebugModuleRegister(RegisterAddresses::CONTROL_REGISTER)
        );
    }

    StatusRegister RiscV::readDebugModuleStatusRegister() {
        return StatusRegister(this->riscVDebugInterface->readDebugModuleRegister(RegisterAddresses::STATUS_REGISTER));
    }

    AbstractControlStatusRegister RiscV::readDebugModuleAbstractControlStatusRegister() {
        return AbstractControlStatusRegister(
            this->riscVDebugInterface->readDebugModuleRegister(RegisterAddresses::ABSTRACT_CONTROL_STATUS_REGISTER)
        );
    }

    DebugControlStatusRegister RiscV::readDebugControlStatusRegister() {
        return DebugControlStatusRegister(this->readRegister(RegisterNumber::DEBUG_CONTROL_STATUS_REGISTER));
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

        return this->riscVDebugInterface->readDebugModuleRegister(RegisterAddresses::ABSTRACT_DATA_0);
    }

    void RiscV::writeRegister(Registers::RegisterNumber number, RegisterValue value) {
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

        this->riscVDebugInterface->writeDebugModuleRegister(RegisterAddresses::ABSTRACT_DATA_0, value);
        this->executeAbstractCommand(command);
    }

    void RiscV::writeDebugModuleControlRegister(const DebugModule::Registers::ControlRegister &controlRegister) {
        this->riscVDebugInterface->writeDebugModuleRegister(
            RegisterAddresses::CONTROL_REGISTER,
            controlRegister.value()
        );
    }

    void RiscV::writeDebugControlStatusRegister(const DebugControlStatusRegister& controlRegister) {
        this->writeRegister(RegisterNumber::DEBUG_CONTROL_STATUS_REGISTER, controlRegister.value());
    }

    void RiscV::executeAbstractCommand(
        const DebugModule::Registers::AbstractCommandRegister& abstractCommandRegister
    ) {
        this->riscVDebugInterface->writeDebugModuleRegister(
            RegisterAddresses::ABSTRACT_COMMAND_REGISTER,
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
