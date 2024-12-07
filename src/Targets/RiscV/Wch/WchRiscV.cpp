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
    }

    TargetDescriptor WchRiscV::targetDescriptor() {
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
            this->riscVDebugInterface->getBreakpointResources()
        };

        if (
            this->targetConfig.reserveSteppingBreakpoint.value_or(false)
            && descriptor.breakpointResources.hardwareBreakpoints > 0
        ) {
            descriptor.breakpointResources.reservedHardwareBreakpoints = 1;
        }

        // Copy the RISC-V CPU register address space and peripheral descriptor
        descriptor.addressSpaceDescriptorsByKey.emplace(
            this->cpuRegisterAddressSpaceDescriptor.key,
            this->cpuRegisterAddressSpaceDescriptor.clone()
        );

        descriptor.peripheralDescriptorsByKey.emplace(
            this->cpuPeripheralDescriptor.key,
            this->cpuPeripheralDescriptor.clone()
        );

        for (auto& [addressSpaceKey, addressSpaceDescriptor] : descriptor.addressSpaceDescriptorsByKey) {
            this->applyDebugInterfaceAccessRestrictions(addressSpaceDescriptor);
        }

        for (auto& [peripheralKey, peripheralDescriptor] : descriptor.peripheralDescriptorsByKey) {
            for (auto& [groupKey, groupDescriptor] : peripheralDescriptor.registerGroupDescriptorsByKey) {
                this->applyDebugInterfaceAccessRestrictions(groupDescriptor);
            }
        }

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

    void WchRiscV::setProgramBreakpoint(const TargetProgramBreakpoint& breakpoint) {
        if (
            breakpoint.type == TargetProgramBreakpoint::Type::SOFTWARE
            && breakpoint.memorySegmentDescriptor == this->mappedProgramMemorySegmentDescriptor
        ) {
            this->riscVDebugInterface->setProgramBreakpoint(TargetProgramBreakpoint{
                .addressSpaceDescriptor = this->sysAddressSpaceDescriptor,
                .memorySegmentDescriptor = this->getDestinationProgramMemorySegmentDescriptor(),
                .address = this->transformAliasedProgramMemoryAddress(breakpoint.address),
                .size = breakpoint.size,
                .type = breakpoint.type
            });

            return;
        }

        this->riscVDebugInterface->setProgramBreakpoint(breakpoint);
    }

    void WchRiscV::removeProgramBreakpoint(const TargetProgramBreakpoint& breakpoint) {
        if (
            breakpoint.type == TargetProgramBreakpoint::Type::SOFTWARE
            && breakpoint.memorySegmentDescriptor == this->mappedProgramMemorySegmentDescriptor
        ) {
            this->riscVDebugInterface->removeProgramBreakpoint(TargetProgramBreakpoint{
                .addressSpaceDescriptor = this->sysAddressSpaceDescriptor,
                .memorySegmentDescriptor = this->getDestinationProgramMemorySegmentDescriptor(),
                .address = this->transformAliasedProgramMemoryAddress(breakpoint.address),
                .size = breakpoint.size,
                .type = breakpoint.type
            });

            return;
        }

        this->riscVDebugInterface->removeProgramBreakpoint(breakpoint);
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
         *        before v2.0.0.
         */
        if (memorySegmentDescriptor == this->mappedProgramMemorySegmentDescriptor) {
            const auto transformedAddress = this->transformAliasedProgramMemoryAddress(startAddress);
            assert(this->programMemorySegmentDescriptor.addressRange.contains(transformedAddress));

            return RiscV::writeMemory(
                addressSpaceDescriptor,
                this->programMemorySegmentDescriptor,
                transformedAddress,
                buffer
            );
        }

        return RiscV::writeMemory(addressSpaceDescriptor, memorySegmentDescriptor, startAddress, buffer);
    }

    const TargetMemorySegmentDescriptor& WchRiscV::getDestinationProgramMemorySegmentDescriptor() {
        return this->programMemorySegmentDescriptor;
    }

    TargetMemoryAddress WchRiscV::transformAliasedProgramMemoryAddress(TargetMemoryAddress address) const {
        using Services::StringService;

        const auto transformedAddress = address - this->mappedProgramMemorySegmentDescriptor.addressRange.startAddress
            + this->programMemorySegmentDescriptor.addressRange.startAddress;

        Logger::debug(
            "Transformed mapped program memory address 0x" + StringService::toHex(address) + " to 0x"
                + StringService::toHex(transformedAddress)
        );

        return transformedAddress;
    }
}
