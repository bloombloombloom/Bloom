#include "RiscV.hpp"

#include <cassert>
#include <cmath>
#include <iterator>

#include "src/Helpers/Pair.hpp"
#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"

#include "src/Exceptions/Exception.hpp"
#include "src/Exceptions/InvalidConfig.hpp"
#include "src/TargetController/Exceptions/TargetOperationFailure.hpp"

namespace Targets::RiscV
{
    RiscV::RiscV(
        const TargetConfig& targetConfig,
        TargetDescriptionFile&& targetDescriptionFile
    )
        : targetConfig(RiscVTargetConfig(targetConfig))
        , targetDescriptionFile(std::move(targetDescriptionFile))
        , cpuRegisterAddressSpaceDescriptor(RiscV::generateCpuRegisterAddressSpaceDescriptor())
        , csrMemorySegmentDescriptor(this->cpuRegisterAddressSpaceDescriptor.getMemorySegmentDescriptor("cs_registers"))
        , gprMemorySegmentDescriptor(this->cpuRegisterAddressSpaceDescriptor.getMemorySegmentDescriptor("gp_registers"))
        , cpuPeripheralDescriptor(
            RiscV::generateCpuPeripheralDescriptor(
                this->cpuRegisterAddressSpaceDescriptor,
                this->csrMemorySegmentDescriptor,
                this->gprMemorySegmentDescriptor
            )
        )
        , csrGroupDescriptor(this->cpuPeripheralDescriptor.getRegisterGroupDescriptor("csr"))
        , gprGroupDescriptor(this->cpuPeripheralDescriptor.getRegisterGroupDescriptor("gpr"))
        , pcRegisterDescriptor(this->csrGroupDescriptor.getRegisterDescriptor("dpc"))
        , spRegisterDescriptor(this->gprGroupDescriptor.getRegisterDescriptor("x2"))
        , sysAddressSpaceDescriptor(this->targetDescriptionFile.getSystemAddressSpaceDescriptor())
    {}

    bool RiscV::supportsDebugTool(DebugTool* debugTool) {
        return
            debugTool->getRiscVDebugInterface(this->targetDescriptionFile, this->targetConfig) != nullptr
            && debugTool->getRiscVIdentificationInterface(this->targetDescriptionFile, this->targetConfig) != nullptr
        ;
    }

    void RiscV::setDebugTool(DebugTool* debugTool) {
        this->riscVDebugInterface = debugTool->getRiscVDebugInterface(this->targetDescriptionFile, this->targetConfig);
        this->riscVProgramInterface = debugTool->getRiscVProgramInterface(
            this->targetDescriptionFile,
            this->targetConfig
        );
        this->riscVIdInterface = debugTool->getRiscVIdentificationInterface(
            this->targetDescriptionFile,
            this->targetConfig
        );
    }

    void RiscV::activate() {
        this->riscVDebugInterface->activate();

        const auto deviceId = this->riscVIdInterface->getDeviceId();
        const auto tdfDeviceId = this->targetDescriptionFile.getTargetId();
        if (deviceId != tdfDeviceId) {
            throw Exceptions::InvalidConfig(
                "RISC-V target ID mismatch - expected " + tdfDeviceId + " but got " + deviceId +
                    ". Please check target configuration."
            );
        }
    }

    void RiscV::deactivate() {
        if (this->getExecutionState() != TargetExecutionState::RUNNING) {
            this->run();
        }

        this->riscVDebugInterface->deactivate();
    }

    void RiscV::postActivate() {
        Logger::info("WCH target ID: " + this->riscVIdInterface->getDeviceId());
    }

    TargetDescriptor RiscV::targetDescriptor() {
        auto descriptor = TargetDescriptor{
            this->targetDescriptionFile.getName(),
            this->targetDescriptionFile.getFamily(),
            this->targetDescriptionFile.getTargetId(),
            this->targetDescriptionFile.getVendorName(),
            this->targetDescriptionFile.targetAddressSpaceDescriptorsByKey(),
            this->targetDescriptionFile.targetPeripheralDescriptorsByKey(),
            this->targetDescriptionFile.targetPinoutDescriptorsByKey(),
            this->targetDescriptionFile.targetVariantDescriptors(),
            {} // TODO: populate this
        };

        // Copy the RISC-V CPU register address space and peripheral descriptor
        descriptor.addressSpaceDescriptorsByKey.emplace(
            this->cpuRegisterAddressSpaceDescriptor.key,
            this->cpuRegisterAddressSpaceDescriptor.clone()
        );

        descriptor.peripheralDescriptorsByKey.emplace(
            this->cpuPeripheralDescriptor.key,
            this->cpuPeripheralDescriptor.clone()
        );

        return descriptor;
    }

    void RiscV::run(std::optional<TargetMemoryAddress> toAddress) {
        this->riscVDebugInterface->run();
    }

    void RiscV::stop() {
        this->riscVDebugInterface->stop();
    }

    void RiscV::step() {
        this->riscVDebugInterface->step();
    }

    void RiscV::reset() {
        this->riscVDebugInterface->reset();
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

    TargetRegisterDescriptorAndValuePairs RiscV::readRegisters(const TargetRegisterDescriptors& descriptors) {
        auto output = TargetRegisterDescriptorAndValuePairs{};

        /*
         * A "system register" is simply a register that we can access via the system address space.
         *
         * CPU registers (GPRs, CSRs, etc) cannot be accessed via the system address space, so we separate them from
         * system registers, in order to access them separately.
         */
        auto cpuRegisterDescriptors = TargetRegisterDescriptors{};

        for (const auto& descriptor : descriptors) {
            if (descriptor->addressSpaceId == this->cpuRegisterAddressSpaceDescriptor.id) {
                if (
                    !this->csrMemorySegmentDescriptor.addressRange.contains(descriptor->startAddress)
                    && !this->gprMemorySegmentDescriptor.addressRange.contains(descriptor->startAddress)
                ) {
                    throw Exceptions::Exception(
                        "Cannot access CPU register \"" + descriptor->key + "\" - unknown memory segment"
                    );
                }

                cpuRegisterDescriptors.emplace_back(descriptor);
                continue;
            }

            if (descriptor->addressSpaceId != this->sysAddressSpaceDescriptor.id) {
                throw Exceptions::Exception(
                    "Cannot access register \"" + descriptor->key + "\" - unknown address space"
                );
            }

            auto value = this->riscVDebugInterface->readMemory(
                this->sysAddressSpaceDescriptor,
                this->resolveRegisterMemorySegmentDescriptor(*descriptor, this->sysAddressSpaceDescriptor),
                descriptor->startAddress,
                descriptor->size
            );

            if (value.size() > 1 && this->sysAddressSpaceDescriptor.endianness == TargetMemoryEndianness::LITTLE) {
                // LSB to MSB
                std::reverse(value.begin(), value.end());
            }

            output.emplace_back(*descriptor, std::move(value));
        }

        if (!cpuRegisterDescriptors.empty()) {
            auto cpuRegisterValues = this->riscVDebugInterface->readCpuRegisters(cpuRegisterDescriptors);
            std::move(cpuRegisterValues.begin(), cpuRegisterValues.end(), std::back_inserter(output));
        }

        return output;
    }

    void RiscV::writeRegisters(const TargetRegisterDescriptorAndValuePairs& registers) {
        for (const auto& pair : registers) {
            const auto& descriptor = pair.first;

            if (descriptor.addressSpaceId == this->cpuRegisterAddressSpaceDescriptor.id) {
                if (
                    !this->csrMemorySegmentDescriptor.addressRange.contains(descriptor.startAddress)
                    && !this->gprMemorySegmentDescriptor.addressRange.contains(descriptor.startAddress)
                ) {
                    throw Exceptions::Exception("Cannot access CPU register - unknown memory segment");
                }

                this->riscVDebugInterface->writeCpuRegisters({pair});
                continue;
            }

            if (descriptor.addressSpaceId != this->sysAddressSpaceDescriptor.id) {
                throw Exceptions::Exception(
                    "Cannot access register \"" + descriptor.key + "\" - unknown address space"
                );
            }

            auto value = pair.second;

            if (value.size() > 1 && this->sysAddressSpaceDescriptor.endianness == TargetMemoryEndianness::LITTLE) {
                // MSB to LSB
                std::reverse(value.begin(), value.end());
            }

            this->riscVDebugInterface->writeMemory(
                this->sysAddressSpaceDescriptor,
                this->resolveRegisterMemorySegmentDescriptor(descriptor, this->sysAddressSpaceDescriptor),
                descriptor.startAddress,
                value
            );
        }
    }

    TargetMemoryBuffer RiscV::readMemory(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        TargetMemoryAddress startAddress,
        TargetMemorySize bytes,
        const std::set<TargetMemoryAddressRange>& excludedAddressRanges
    ) {
        assert(bytes > 0);

        assert(addressSpaceDescriptor.addressRange.contains(startAddress));
        assert(addressSpaceDescriptor.addressRange.contains(startAddress + bytes - 1));

        assert(memorySegmentDescriptor.addressRange.contains(startAddress));
        assert(memorySegmentDescriptor.addressRange.contains(startAddress + bytes - 1));

        return this->riscVDebugInterface->readMemory(
            addressSpaceDescriptor,
            memorySegmentDescriptor,
            startAddress,
            bytes,
            excludedAddressRanges
        );
    }

    void RiscV::writeMemory(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        TargetMemoryAddress startAddress,
        const TargetMemoryBuffer& buffer
    ) {
        assert(!buffer.empty());

        assert(addressSpaceDescriptor.addressRange.contains(startAddress));
        assert(addressSpaceDescriptor.addressRange.contains(
            static_cast<TargetMemoryAddress>(startAddress + buffer.size()) - 1)
        );

        assert(memorySegmentDescriptor.addressRange.contains(startAddress));
        assert(memorySegmentDescriptor.addressRange.contains(
            static_cast<TargetMemoryAddress>(startAddress + buffer.size()) - 1)
        );

        if (memorySegmentDescriptor.type == TargetMemorySegmentType::FLASH && this->riscVProgramInterface) {
            return this->riscVProgramInterface->writeFlashMemory(startAddress, buffer);
        }

        return this->riscVDebugInterface->writeMemory(
            addressSpaceDescriptor,
            memorySegmentDescriptor,
            startAddress,
            buffer
        );
    }

    bool RiscV::isProgramMemory(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        TargetMemoryAddress startAddress,
        TargetMemorySize size
    ) {
        return memorySegmentDescriptor.executable;
    }

    void RiscV::eraseMemory(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor
    ) {

    }

    TargetExecutionState RiscV::getExecutionState() {
        return this->riscVDebugInterface->getExecutionState();
    }

    TargetMemoryAddress RiscV::getProgramCounter() {
        const auto value = this->riscVDebugInterface->readCpuRegisters({&(this->pcRegisterDescriptor)}).at(0).second;
        assert(value.size() == 4);
        return static_cast<TargetMemoryAddress>(
            (value[0] << 24) | (value[1] << 16) | (value[2] << 8) | (value[3])
        );
    }

    void RiscV::setProgramCounter(TargetMemoryAddress programCounter) {
        // TODO: test this
        this->riscVDebugInterface->writeCpuRegisters({
            {
                this->pcRegisterDescriptor,
                TargetMemoryBuffer{
                    static_cast<unsigned char>(programCounter >> 24),
                    static_cast<unsigned char>(programCounter >> 16),
                    static_cast<unsigned char>(programCounter >> 8),
                    static_cast<unsigned char>(programCounter)
                }
            }
        });
    }

    TargetStackPointer RiscV::getStackPointer() {
        const auto value = this->riscVDebugInterface->readCpuRegisters({&(this->spRegisterDescriptor)}).at(0).second;
        assert(value.size() == 4);
        return static_cast<TargetStackPointer>(
            (value[0] << 24) | (value[1] << 16) | (value[2] << 8) | (value[3])
        );
    }

    void RiscV::setStackPointer(TargetStackPointer stackPointer) {
        this->riscVDebugInterface->writeCpuRegisters({
            {
                this->spRegisterDescriptor,
                TargetMemoryBuffer{
                    static_cast<unsigned char>(stackPointer >> 24),
                    static_cast<unsigned char>(stackPointer >> 16),
                    static_cast<unsigned char>(stackPointer >> 8),
                    static_cast<unsigned char>(stackPointer)
                }
            }
        });
    }

    TargetGpioPinDescriptorAndStatePairs RiscV::getGpioPinStates(const TargetPinoutDescriptor& pinoutDescriptor) {
        return {};
    }

    void RiscV::setGpioPinState(const TargetPinDescriptor& pinDescriptor, const TargetGpioPinState& state) {

    }

    void RiscV::enableProgrammingMode() {

    }

    void RiscV::disableProgrammingMode() {

    }

    bool RiscV::programmingModeEnabled() {
        return false;
    }

    const TargetMemorySegmentDescriptor& RiscV::resolveRegisterMemorySegmentDescriptor(
        const TargetRegisterDescriptor& regDescriptor,
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor
    ) {
        const auto segmentDescriptors = addressSpaceDescriptor.getIntersectingMemorySegmentDescriptors(
            TargetMemoryAddressRange{
                regDescriptor.startAddress,
                (regDescriptor.startAddress + (regDescriptor.size / addressSpaceDescriptor.unitSize) - 1)
            }
        );

        if (segmentDescriptors.empty()) {
            throw Exceptions::Exception(
                "Cannot access system register \"" + regDescriptor.key + "\" - unknown memory segment"
            );
        }

        if (segmentDescriptors.size() != 1) {
            throw Exceptions::Exception(
                "Cannot access system register \"" + regDescriptor.key
                    + "\" - register spans multiple memory segments"
            );
        }

        return *(segmentDescriptors.front());
    }

    TargetAddressSpaceDescriptor RiscV::generateCpuRegisterAddressSpaceDescriptor() {
        auto addressSpace = TargetAddressSpaceDescriptor{
            "debug_module",
            {0x0000, 0xFFFF},
            TargetMemoryEndianness::LITTLE,
            {},
            4
        };

        addressSpace.segmentDescriptorsByKey.emplace(
            "cs_registers",
            TargetMemorySegmentDescriptor{
                addressSpace.key,
                "cs_registers",
                "Control Status Registers",
                TargetMemorySegmentType::REGISTERS,
                {0x0000, 0x0FFF},
                addressSpace.unitSize,
                false,
                {true, true},
                {false, false},
                std::nullopt
            }
        );

        addressSpace.segmentDescriptorsByKey.emplace(
            "gp_registers",
            TargetMemorySegmentDescriptor{
                addressSpace.key,
                "gp_registers",
                "General Purpose Registers",
                TargetMemorySegmentType::GENERAL_PURPOSE_REGISTERS,
                {0x1000, 0x101F},
                addressSpace.unitSize,
                false,
                {true, true},
                {false, false},
                std::nullopt
            }
        );

        return addressSpace;
    }

    TargetPeripheralDescriptor RiscV::generateCpuPeripheralDescriptor(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& csrMemorySegmentDescriptor,
        const TargetMemorySegmentDescriptor& gprMemorySegmentDescriptor
    ) {
        auto cpuPeripheralDescriptor = TargetPeripheralDescriptor{
            "cpu",
            "RISC-V CPU",
            {},
            {}
        };

        auto& gprGroup = cpuPeripheralDescriptor.registerGroupDescriptorsByKey.emplace(
            "gpr",
            TargetRegisterGroupDescriptor{
                "gpr",
                "gpr",
                "General Purpose Registers",
                cpuPeripheralDescriptor.key,
                addressSpaceDescriptor.key,
                std::nullopt,
                {},
                {}
            }
        ).first->second;

        for (auto i = std::uint8_t{0}; i <= 31; ++i) {
            const auto key = "x" + std::to_string(i);
            gprGroup.registerDescriptorsByKey.emplace(
                key,
                TargetRegisterDescriptor{
                    key,
                    "X" + std::to_string(i),
                    gprGroup.absoluteKey,
                    cpuPeripheralDescriptor.key,
                    addressSpaceDescriptor.key,
                    gprMemorySegmentDescriptor.addressRange.startAddress + i,
                    4,
                    TargetRegisterType::GENERAL_PURPOSE_REGISTER,
                    TargetRegisterAccess(true, true),
                    std::nullopt,
                    {}
                }
            );
        }

        auto& csrGroup = cpuPeripheralDescriptor.registerGroupDescriptorsByKey.emplace(
            "csr",
            TargetRegisterGroupDescriptor{
                "csr",
                "csr",
                "Control Status Registers",
                cpuPeripheralDescriptor.key,
                addressSpaceDescriptor.key,
                std::nullopt,
                {},
                {}
            }
        ).first->second;

        csrGroup.registerDescriptorsByKey.emplace(
            "marchid",
            TargetRegisterDescriptor{
                "marchid",
                "MARCHID",
                csrGroup.absoluteKey,
                cpuPeripheralDescriptor.key,
                addressSpaceDescriptor.key,
                csrMemorySegmentDescriptor.addressRange.startAddress + 0xF12,
                4,
                TargetRegisterType::OTHER,
                TargetRegisterAccess(true, false),
                "Architecture ID",
                {}
            }
        );

        csrGroup.registerDescriptorsByKey.emplace(
            "mimpid",
            TargetRegisterDescriptor{
                "mimpid",
                "MIMPID",
                csrGroup.absoluteKey,
                cpuPeripheralDescriptor.key,
                addressSpaceDescriptor.key,
                csrMemorySegmentDescriptor.addressRange.startAddress + 0xF13,
                4,
                TargetRegisterType::OTHER,
                TargetRegisterAccess(true, false),
                "Implementation ID",
                {}
            }
        );

        csrGroup.registerDescriptorsByKey.emplace(
            "mstatus",
            TargetRegisterDescriptor{
                "mstatus",
                "MSTATUS",
                csrGroup.absoluteKey,
                cpuPeripheralDescriptor.key,
                addressSpaceDescriptor.key,
                csrMemorySegmentDescriptor.addressRange.startAddress + 0x300,
                4,
                TargetRegisterType::OTHER,
                TargetRegisterAccess(true, true),
                "Machine status",
                {}
            }
        );

        csrGroup.registerDescriptorsByKey.emplace(
            "misa",
            TargetRegisterDescriptor{
                "misa",
                "MISA",
                csrGroup.absoluteKey,
                cpuPeripheralDescriptor.key,
                addressSpaceDescriptor.key,
                csrMemorySegmentDescriptor.addressRange.startAddress + 0x301,
                4,
                TargetRegisterType::OTHER,
                TargetRegisterAccess(true, true),
                "ISA and extensions",
                {}
            }
        );

        csrGroup.registerDescriptorsByKey.emplace(
            "mtvec",
            TargetRegisterDescriptor{
                "mtvec",
                "MTVEC",
                csrGroup.absoluteKey,
                cpuPeripheralDescriptor.key,
                addressSpaceDescriptor.key,
                csrMemorySegmentDescriptor.addressRange.startAddress + 0x305,
                4,
                TargetRegisterType::OTHER,
                TargetRegisterAccess(true, true),
                "Machine trap-handler base address",
                {}
            }
        );

        csrGroup.registerDescriptorsByKey.emplace(
            "mcounteren",
            TargetRegisterDescriptor{
                "mcounteren",
                "MCOUNTEREN",
                csrGroup.absoluteKey,
                cpuPeripheralDescriptor.key,
                addressSpaceDescriptor.key,
                csrMemorySegmentDescriptor.addressRange.startAddress + 0x306,
                4,
                TargetRegisterType::OTHER,
                TargetRegisterAccess(true, true),
                "Machine counter enable",
                {}
            }
        );

        csrGroup.registerDescriptorsByKey.emplace(
            "mscratch",
            TargetRegisterDescriptor{
                "mscratch",
                "MSCRATCH",
                csrGroup.absoluteKey,
                cpuPeripheralDescriptor.key,
                addressSpaceDescriptor.key,
                csrMemorySegmentDescriptor.addressRange.startAddress + 0x340,
                4,
                TargetRegisterType::OTHER,
                TargetRegisterAccess(true, true),
                "Scratch register for machine trap handlers",
                {}
            }
        );

        csrGroup.registerDescriptorsByKey.emplace(
            "mepc",
            TargetRegisterDescriptor{
                "mepc",
                "MEPC",
                csrGroup.absoluteKey,
                cpuPeripheralDescriptor.key,
                addressSpaceDescriptor.key,
                csrMemorySegmentDescriptor.addressRange.startAddress + 0x341,
                4,
                TargetRegisterType::OTHER,
                TargetRegisterAccess(true, true),
                "Machine exception program counter",
                {}
            }
        );

        csrGroup.registerDescriptorsByKey.emplace(
            "mcause",
            TargetRegisterDescriptor{
                "mcause",
                "MCAUSE",
                csrGroup.absoluteKey,
                cpuPeripheralDescriptor.key,
                addressSpaceDescriptor.key,
                csrMemorySegmentDescriptor.addressRange.startAddress + 0x342,
                4,
                TargetRegisterType::OTHER,
                TargetRegisterAccess(true, true),
                "Machine trap cause",
                {}
            }
        );

        csrGroup.registerDescriptorsByKey.emplace(
            "mtval",
            TargetRegisterDescriptor{
                "mtval",
                "MTVAL",
                csrGroup.absoluteKey,
                cpuPeripheralDescriptor.key,
                addressSpaceDescriptor.key,
                csrMemorySegmentDescriptor.addressRange.startAddress + 0x343,
                4,
                TargetRegisterType::OTHER,
                TargetRegisterAccess(true, true),
                "Machine bad address or instruction",
                {}
            }
        );

        csrGroup.registerDescriptorsByKey.emplace(
            "mip",
            TargetRegisterDescriptor{
                "mip",
                "MIP",
                csrGroup.absoluteKey,
                cpuPeripheralDescriptor.key,
                addressSpaceDescriptor.key,
                csrMemorySegmentDescriptor.addressRange.startAddress + 0x344,
                4,
                TargetRegisterType::OTHER,
                TargetRegisterAccess(true, true),
                "Machine interrupt pending",
                {}
            }
        );

        csrGroup.registerDescriptorsByKey.emplace(
            "dcsr",
            TargetRegisterDescriptor{
                "dcsr",
                "DCSR",
                csrGroup.absoluteKey,
                cpuPeripheralDescriptor.key,
                addressSpaceDescriptor.key,
                csrMemorySegmentDescriptor.addressRange.startAddress + 0x7B0,
                4,
                TargetRegisterType::OTHER,
                TargetRegisterAccess(true, true),
                "Debug control and status",
                {}
            }
        );

        csrGroup.registerDescriptorsByKey.emplace(
            "dpc",
            TargetRegisterDescriptor{
                "dpc",
                "DPC",
                csrGroup.absoluteKey,
                cpuPeripheralDescriptor.key,
                addressSpaceDescriptor.key,
                csrMemorySegmentDescriptor.addressRange.startAddress + 0x7B1,
                4,
                TargetRegisterType::OTHER,
                TargetRegisterAccess(true, true),
                "Debug program counter",
                {}
            }
        );

        csrGroup.registerDescriptorsByKey.emplace(
            "dscratch0",
            TargetRegisterDescriptor{
                "dscratch0",
                "DSCRATCH0",
                csrGroup.absoluteKey,
                cpuPeripheralDescriptor.key,
                addressSpaceDescriptor.key,
                csrMemorySegmentDescriptor.addressRange.startAddress + 0x7B2,
                4,
                TargetRegisterType::OTHER,
                TargetRegisterAccess(true, true),
                "Debug scratch 0",
                {}
            }
        );

        csrGroup.registerDescriptorsByKey.emplace(
            "dscratch1",
            TargetRegisterDescriptor{
                "dscratch1",
                "DSCRATCH1",
                csrGroup.absoluteKey,
                cpuPeripheralDescriptor.key,
                addressSpaceDescriptor.key,
                csrMemorySegmentDescriptor.addressRange.startAddress + 0x7B3,
                4,
                TargetRegisterType::OTHER,
                TargetRegisterAccess(true, true),
                "Debug scratch 1",
                {}
            }
        );

        return cpuPeripheralDescriptor;
    }
}
