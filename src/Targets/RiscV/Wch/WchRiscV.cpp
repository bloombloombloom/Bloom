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
        const auto deviceId = this->riscVDebugInterface->getDeviceId();

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

        /*
         * WCH targets typically possess a memory segment that is mapped to program memory. We cannot write to this
         * segment directly, which is why it's described as read-only in Bloom's TDFs. However, we enable writing to
         * the segment by forwarding any write operations to the appropriate (aliased) segment.
         *
         * For this reason, we adjust the access member on the memory segment descriptor so that other components
         * within Bloom will see the segment as writeable.
         *
         * See the overridden WchRiscV::writeMemory() member function below, for more.
         */
        sysAddressSpaceDescriptor.getMemorySegmentDescriptor(
            this->mappedProgramMemorySegmentDescriptor.key
        ).programmingModeAccess.writeable = true;

        return descriptor;
    }

    void WchRiscV::writeMemory(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        TargetMemoryAddress startAddress,
        TargetMemoryBufferSpan buffer
    ) {
        /*
         * WCH targets have an alias segment that maps to either the program memory segment or the boot program
         * memory segment.
         *
         * Reading directly from this memory segment is fine, but we cannot write to it - the operation just fails
         * silently. We handle this by forwarding any write operations on that segment to the appropriate (aliased)
         * segment.
         *
         * @TODO: Currently, this just assumes that the alias segment always maps to the program memory segment, but I
         *        believe it may map to the boot program memory segment in some cases. This needs to be revisited
         *        before v1.1.0.
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
