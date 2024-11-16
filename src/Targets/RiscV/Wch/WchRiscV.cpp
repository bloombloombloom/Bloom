#include "WchRiscV.hpp"

#include <utility>
#include <cassert>

#include "src/Logger/Logger.hpp"
#include "src/Exceptions/InvalidConfig.hpp"

namespace Targets::RiscV::Wch
{
    WchRiscV::WchRiscV(
        const TargetConfig& targetConfig,
        TargetDescriptionFile&& targetDescriptionFile
    )
        : RiscV(targetConfig, targetDescriptionFile)
        , targetDescriptionFile(std::move(targetDescriptionFile))
        , programMemorySegmentDescriptor(this->sysAddressSpaceDescriptor.getMemorySegmentDescriptor("internal_program_memory"))
        , mappedProgramMemorySegmentDescriptor(this->sysAddressSpaceDescriptor.getMemorySegmentDescriptor("mapped_progmem"))
    {}

    void WchRiscV::activate() {
        RiscV::activate();

        /*
         * WCH target IDs are specific to the variant. Each variant in the TDF should have a property group that holds
         * the variant ID.
         */
        const auto variantsById = this->targetDescriptionFile.getVariantsByWchVariantId();
        const auto deviceId = this->riscVIdInterface->getDeviceId();

        const auto variantIt = variantsById.find(deviceId);
        if (variantIt == variantsById.end()) {
            throw Exceptions::InvalidConfig{
                "Unknown WCH variant ID \"" + deviceId + "\". Please check your configuration."
            };
        }

        this->variant = *(variantIt->second);
    }

    void WchRiscV::postActivate() {
        assert(this->variant.has_value());

        const auto& variant = this->variant->get();
        Logger::info("WCH variant ID: " + variant.getProperty("vendor", "variant_id").value);
        Logger::info("WCH variant name: " + variant.name);

        RiscV::postActivate();
    }

    TargetDescriptor WchRiscV::targetDescriptor() {
        const auto hardwareBreakpointCount = this->riscVDebugInterface->getHardwareBreakpointCount();

        auto descriptor = TargetDescriptor{
            this->targetDescriptionFile.getName(),
            this->targetDescriptionFile.getFamily(),
            this->variant->get().getProperty("vendor", "variant_id").value,
            this->targetDescriptionFile.getVendorName(),
            this->targetDescriptionFile.targetAddressSpaceDescriptorsByKey(),
            this->targetDescriptionFile.targetPeripheralDescriptorsByKey(),
            this->targetDescriptionFile.targetPadDescriptorsByKey(),
            this->targetDescriptionFile.targetPinoutDescriptorsByKey(),
            this->targetDescriptionFile.targetVariantDescriptorsByKey(),
            BreakpointResources{
                hardwareBreakpointCount,
                std::nullopt,
                static_cast<std::uint16_t>(
                    this->targetConfig.reserveSteppingBreakpoint && hardwareBreakpointCount > 0 ? 1 : 0
                )
            }
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

        auto& sysAddressSpaceDescriptor = descriptor.getAddressSpaceDescriptor("system");
        sysAddressSpaceDescriptor.getMemorySegmentDescriptor("internal_program_memory").inspectionEnabled = true;
        sysAddressSpaceDescriptor.getMemorySegmentDescriptor("internal_ram").inspectionEnabled = true;

        return descriptor;
    }

    void WchRiscV::writeMemory(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        TargetMemoryAddress startAddress,
        const TargetMemoryBuffer& buffer
    ) {
        /*
         * WCH targets have a memory segment that maps to either the program memory segment or the boot program
         * memory segment.
         *
         * Reading directly from the mapped memory segment is fine, but we cannot write to it - the operation just
         * fails silently. We handle this by altering the write operation so that we write to the appropriate,
         * non-mapped segment.
         */
        if (memorySegmentDescriptor == this->mappedProgramMemorySegmentDescriptor) {
            const auto newAddress = startAddress - this->mappedProgramMemorySegmentDescriptor.addressRange.startAddress
                + this->programMemorySegmentDescriptor.addressRange.startAddress;
            assert(this->programMemorySegmentDescriptor.addressRange.contains(newAddress));

            return RiscV::writeMemory(addressSpaceDescriptor, this->programMemorySegmentDescriptor, newAddress, buffer);
        }

        return RiscV::writeMemory(addressSpaceDescriptor, memorySegmentDescriptor, startAddress, buffer);
    }
}
