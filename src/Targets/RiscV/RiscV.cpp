#include "RiscV.hpp"

#include <cassert>
#include <cmath>
#include <iterator>
#include <algorithm>

#include "src/Helpers/Pair.hpp"
#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"

#include "Exceptions/IllegalMemoryAccess.hpp"

#include "src/Exceptions/Exception.hpp"
#include "src/Exceptions/InvalidConfig.hpp"
#include "src/TargetController/Exceptions/TargetOperationFailure.hpp"

namespace Targets::RiscV
{
    RiscV::RiscV(const TargetConfig& targetConfig, const TargetDescriptionFile& targetDescriptionFile)
        : targetConfig(RiscVTargetConfig{targetConfig})
        , targetDescriptionFile(targetDescriptionFile)
        , isaDescriptor(this->targetDescriptionFile.getIsaDescriptor())
        , csrAddressSpaceDescriptor(this->targetDescriptionFile.getCsrAddressSpaceDescriptor())
        , csrMemorySegmentDescriptor(this->csrAddressSpaceDescriptor.getMemorySegmentDescriptor("csr"))
        , gprAddressSpaceDescriptor(this->targetDescriptionFile.getGprAddressSpaceDescriptor())
        , gprMemorySegmentDescriptor(this->gprAddressSpaceDescriptor.getMemorySegmentDescriptor("gpr"))
        , cpuPeripheralDescriptor(this->targetDescriptionFile.getTargetPeripheralDescriptor("cpu"))
        , csrGroupDescriptor(this->cpuPeripheralDescriptor.getRegisterGroupDescriptor("csr"))
        , gprGroupDescriptor(
            RiscV::generateGeneralPurposeRegisterGroupDescriptor(
                this->isaDescriptor,
                this->gprAddressSpaceDescriptor,
                this->gprMemorySegmentDescriptor,
                this->cpuPeripheralDescriptor
            )
        )
        , pcRegisterDescriptor(this->csrGroupDescriptor.getRegisterDescriptor("dpc"))
        , spRegisterDescriptor(this->gprGroupDescriptor.getRegisterDescriptor("x2"))
        , sysAddressSpaceDescriptor(this->targetDescriptionFile.getSystemAddressSpaceDescriptor())
    {}

    bool RiscV::supportsDebugTool(DebugTool* debugTool) {
        return debugTool->getRiscVDebugInterface(this->targetDescriptionFile, this->targetConfig) != nullptr;
    }

    void RiscV::setDebugTool(DebugTool* debugTool) {
        this->riscVDebugInterface = debugTool->getRiscVDebugInterface(this->targetDescriptionFile, this->targetConfig);
    }

    void RiscV::activate() {
        this->riscVDebugInterface->activate();
        this->stop();
        this->reset();
    }

    void RiscV::deactivate() {
        // TODO: Is this "tidy-up" code better placed in the TC? Review after v2.0.0.

        if (this->getExecutionState() != TargetExecutionState::STOPPED) {
            this->stop();
        }

        this->riscVDebugInterface->deactivate();
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

    TargetRegisterDescriptorAndValuePairs RiscV::readRegisters(const TargetRegisterDescriptors& descriptors) {
        using Services::StringService;

        auto output = TargetRegisterDescriptorAndValuePairs{};

        /*
         * A "system register" is simply a register that we can access via the system address space.
         *
         * CPU registers (GPRs, CSRs, etc) cannot be accessed via the system address space, so we separate them from
         * system registers, in order to access them separately.
         */
        auto cpuRegisterDescriptors = TargetRegisterDescriptors{};

        for (const auto& descriptor : descriptors) {
            if (
                descriptor->addressSpaceId == this->csrAddressSpaceDescriptor.id
                || descriptor->addressSpaceId == this->gprAddressSpaceDescriptor.id
            ) {
                if (
                    descriptor->addressSpaceId == this->csrAddressSpaceDescriptor.id
                    && !this->csrMemorySegmentDescriptor.addressRange.contains(descriptor->startAddress)
                ) {
                    throw Exceptions::Exception{
                        "Cannot access CPU CSR " + StringService::formatKey(descriptor->key) + " - unknown memory segment"
                    };
                }

                if (
                    descriptor->addressSpaceId == this->gprAddressSpaceDescriptor.id
                    && !this->gprMemorySegmentDescriptor.addressRange.contains(descriptor->startAddress)
                ) {
                    throw Exceptions::Exception{
                        "Cannot access CPU GPR " + StringService::formatKey(descriptor->key) + " - unknown memory segment"
                    };
                }

                cpuRegisterDescriptors.emplace_back(descriptor);
                continue;
            }

            if (descriptor->addressSpaceId != this->sysAddressSpaceDescriptor.id) {
                throw Exceptions::Exception{
                    "Cannot access register " + StringService::formatKey(descriptor->key) + " - unknown address space"
                };
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
        using Services::StringService;

        for (const auto& pair : registers) {
            const auto& descriptor = pair.first;

            if (
                descriptor.addressSpaceId == this->csrAddressSpaceDescriptor.id
                || descriptor.addressSpaceId == this->gprAddressSpaceDescriptor.id
            ) {
                if (
                    descriptor.addressSpaceId == this->csrAddressSpaceDescriptor.id
                    && !this->csrMemorySegmentDescriptor.addressRange.contains(descriptor.startAddress)
                ) {
                    throw Exceptions::Exception{
                        "Cannot access CPU CSR " + StringService::formatKey(descriptor.key) + " - unknown memory segment"
                    };
                }

                if (
                    descriptor.addressSpaceId == this->gprAddressSpaceDescriptor.id
                    && !this->gprMemorySegmentDescriptor.addressRange.contains(descriptor.startAddress)
                ) {
                    throw Exceptions::Exception{
                        "Cannot access CPU GPR " + StringService::formatKey(descriptor.key)
                            + " - unknown memory segment"
                    };
                }

                this->riscVDebugInterface->writeCpuRegisters({pair});
                continue;
            }

            if (descriptor.addressSpaceId != this->sysAddressSpaceDescriptor.id) {
                throw Exceptions::Exception{
                    "Cannot access register " + StringService::formatKey(descriptor.key)
                        + " - unknown address space"
                };
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
        TargetMemoryBufferSpan buffer
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
        this->riscVDebugInterface->eraseMemory(addressSpaceDescriptor, memorySegmentDescriptor);
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

    void RiscV::enableProgrammingMode() {
        this->riscVDebugInterface->enableProgrammingMode();
        this->programmingMode = true;
    }

    void RiscV::disableProgrammingMode() {
        this->riscVDebugInterface->disableProgrammingMode();
        this->programmingMode = false;
    }

    bool RiscV::programmingModeEnabled() {
        return this->programmingMode;
    }

    TargetMemoryBuffer RiscV::readRegister(const TargetRegisterDescriptor& descriptor) {
        return this->readRegisters({&descriptor}).front().second;
    }

    DynamicRegisterValue RiscV::readRegisterDynamicValue(const TargetRegisterDescriptor& descriptor) {
        return DynamicRegisterValue{descriptor, this->readRegister(descriptor)};
    }

    void RiscV::writeRegister(const DynamicRegisterValue& dynamicRegister) {
        this->writeRegister(dynamicRegister.registerDescriptor, dynamicRegister.data());
    }

    void RiscV::writeRegister(const TargetRegisterDescriptor& descriptor, TargetMemoryBufferSpan value) {
        this->writeRegisters({{descriptor, TargetMemoryBuffer{value.begin(), value.end()}}});
    }

    void RiscV::writeRegister(const TargetRegisterDescriptor& descriptor, std::uint64_t value) {
        auto data = TargetMemoryBuffer(descriptor.size, 0x00);

        for (auto i = TargetMemorySize{0}; i < descriptor.size; ++i) {
            data[i] = static_cast<unsigned char>(value >> ((descriptor.size - i - 1) * 8));
        }

        this->writeRegister(descriptor, data);
    }

    bool RiscV::probeMemory(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        Targets::TargetMemoryAddress address
    ) {
        try {
            this->riscVDebugInterface->readMemory(addressSpaceDescriptor, memorySegmentDescriptor, address, 4, {});
            return true;

        } catch (const Exceptions::IllegalMemoryAccess&) {
            return false;
        }
    }

    void RiscV::applyDebugInterfaceAccessRestrictions(TargetAddressSpaceDescriptor& addressSpaceDescriptor) {
        for (auto& [segmentKey, segmentDescriptor] : addressSpaceDescriptor.segmentDescriptorsByKey) {
            this->riscVDebugInterface->applyAccessRestrictions(segmentDescriptor);
        }
    }

    void RiscV::applyDebugInterfaceAccessRestrictions(TargetRegisterGroupDescriptor& registerGroupDescriptor) {
        for (auto& [registerKey, registerDescriptor] : registerGroupDescriptor.registerDescriptorsByKey) {
            this->riscVDebugInterface->applyAccessRestrictions(registerDescriptor);
        }

        for (auto& [subgroupKey, subgroupDescriptor] : registerGroupDescriptor.subgroupDescriptorsByKey) {
            this->applyDebugInterfaceAccessRestrictions(subgroupDescriptor);
        }
    }

    const TargetMemorySegmentDescriptor& RiscV::resolveRegisterMemorySegmentDescriptor(
        const TargetRegisterDescriptor& regDescriptor,
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor
    ) {
        using Services::StringService;

        const auto segmentDescriptors = addressSpaceDescriptor.getIntersectingMemorySegmentDescriptors(
            TargetMemoryAddressRange{
                regDescriptor.startAddress,
                (regDescriptor.startAddress + (regDescriptor.size / addressSpaceDescriptor.unitSize) - 1)
            }
        );

        if (segmentDescriptors.empty()) {
            throw Exceptions::Exception{
                "Cannot access system register " + StringService::formatKey(regDescriptor.key)
                    + " - unknown memory segment"
            };
        }

        if (segmentDescriptors.size() != 1) {
            throw Exceptions::Exception{
                "Cannot access system register " + StringService::formatKey(regDescriptor.key)
                    + " - register spans multiple memory segments"
            };
        }

        return *(segmentDescriptors.front());
    }

    const TargetRegisterGroupDescriptor& RiscV::generateGeneralPurposeRegisterGroupDescriptor(
        const IsaDescriptor& isaDescriptor,
        const TargetAddressSpaceDescriptor& gprAddressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& gprMemorySegmentDescriptor,
        TargetPeripheralDescriptor& cpuPeripheralDescriptor
    ) {
        auto& gprGroup = cpuPeripheralDescriptor.registerGroupDescriptorsByKey.emplace(
            "gpr",
            TargetRegisterGroupDescriptor{
                "gpr",
                "gpr",
                "GPR",
                cpuPeripheralDescriptor.key,
                gprAddressSpaceDescriptor.key,
                std::nullopt,
                {},
                {}
            }
        ).first->second;

        for (auto i = std::uint8_t{0}; i <= static_cast<std::uint8_t>(isaDescriptor.isReduced() ? 15 : 31); ++i) {
            const auto key = "x" + std::to_string(i);
            gprGroup.registerDescriptorsByKey.emplace(
                key,
                TargetRegisterDescriptor{
                    key,
                    "X" + std::to_string(i),
                    gprGroup.absoluteKey,
                    cpuPeripheralDescriptor.key,
                    gprAddressSpaceDescriptor.key,
                    gprMemorySegmentDescriptor.addressRange.startAddress + i,
                    4,
                    TargetRegisterType::GENERAL_PURPOSE_REGISTER,
                    TargetRegisterAccess{true, true},
                    std::nullopt,
                    {}
                }
            );
        }

        return gprGroup;
    }
}
