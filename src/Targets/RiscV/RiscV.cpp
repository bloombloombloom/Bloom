#include "RiscV.hpp"

#include <thread>
#include <chrono>

#include "DebugModule/Registers/RegisterAddresses.hpp"

#include "src/Exceptions/Exception.hpp"

#include "src/Logger/Logger.hpp"

namespace Targets::RiscV
{
    using DebugModule::Registers::RegisterAddresses;
    using DebugModule::Registers::ControlRegister;
    using DebugModule::Registers::StatusRegister;

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

        this->discoverHartIndices();
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

        this->writeControlRegister(controlRegister);

        constexpr auto maxAttempts = 10;
        auto statusRegister = this->readStatusRegister();

        for (auto attempts = 1; !statusRegister.allResumeAcknowledge && attempts <= maxAttempts; ++attempts) {
            std::this_thread::sleep_for(std::chrono::microseconds(10));
            statusRegister = this->readStatusRegister();
        }

        controlRegister.resumeRequest = false;
        this->writeControlRegister(controlRegister);

        if (!statusRegister.allResumeAcknowledge) {
            throw Exceptions::Exception("Target took too long to acknowledge resume request");
        }
    }

    void RiscV::stop() {
        auto controlRegister = ControlRegister();
        controlRegister.debugModuleActive = true;
        controlRegister.selectedHartIndex = this->selectedHartIndex;
        controlRegister.haltRequest = true;

        this->writeControlRegister(controlRegister);

        constexpr auto maxAttempts = 10;
        auto statusRegister = this->readStatusRegister();

        for (auto attempts = 1; !statusRegister.allHalted && attempts <= maxAttempts; ++attempts) {
            std::this_thread::sleep_for(std::chrono::microseconds(10));
            statusRegister = this->readStatusRegister();
        }

        controlRegister.haltRequest = false;
        this->writeControlRegister(controlRegister);

        if (!statusRegister.allHalted) {
            throw Exceptions::Exception("Target took too long to halt selected harts");
        }
    }

    void RiscV::step() {

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
        return this->readStatusRegister().anyRunning ? TargetState::RUNNING : TargetState::STOPPED;
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

    void RiscV::discoverHartIndices() {
        /*
         * We can obtain the maximum hart index by setting all of the hartsel bits in the control register and then
         * reading the value back.
         */
        auto controlRegister = ControlRegister();
        controlRegister.debugModuleActive = true;
        controlRegister.selectedHartIndex = 0xFFFFF;

        this->writeControlRegister(controlRegister);
        controlRegister = this->readControlRegister();

        for (DebugModule::HartIndex hartIndex = 0; hartIndex <= controlRegister.selectedHartIndex; ++hartIndex) {
            /*
             * We can't just assume that everything between 0 and the maximum hart index are valid hart indices. We
             * have to test each index until we find one that is non-existent.
             */
            controlRegister = ControlRegister();
            controlRegister.debugModuleActive = true;
            controlRegister.selectedHartIndex = hartIndex;

            this->writeControlRegister(controlRegister);

            /*
             * It's worth noting that some RISC-V targets **do not** set the non-existent flags. I'm not sure why.
             * Have they just hardwired hartsel to 0 because they only support a single hart, preventing the selection
             * of non-existent harts?
             *
             * Relying on the maximum hart index seems to be all we can do in this case.
             */
            if (this->readStatusRegister().anyNonExistent) {
                break;
            }

            this->hartIndices.insert(hartIndex);
        }
    }

    ControlRegister RiscV::readControlRegister() {
        return ControlRegister(
            this->riscVDebugInterface->readDebugModuleRegister(RegisterAddresses::CONTROL_REGISTER)
        );
    }

    StatusRegister RiscV::readStatusRegister() {
        return StatusRegister(this->riscVDebugInterface->readDebugModuleRegister(RegisterAddresses::STATUS_REGISTER));
    }

    void RiscV::writeControlRegister(const ControlRegister& controlRegister) {
        this->riscVDebugInterface->writeDebugModuleRegister(
            RegisterAddresses::CONTROL_REGISTER,
            controlRegister.value()
        );
    }
}
