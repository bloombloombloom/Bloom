#include "RiscVGdbTargetDescriptor.hpp"

#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::RiscVGdb
{
    using Targets::TargetRegisterDescriptor;
    using Targets::TargetRegisterType;

    using Exceptions::Exception;

    RiscVGdbTargetDescriptor::RiscVGdbTargetDescriptor(const Targets::TargetDescriptor& targetDescriptor)
        : systemAddressSpaceDescriptor(targetDescriptor.getAddressSpaceDescriptor("system"))
        , cpuAddressSpaceDescriptor(targetDescriptor.getAddressSpaceDescriptor("csr"))
        , programMemorySegmentDescriptor(this->systemAddressSpaceDescriptor.getMemorySegmentDescriptor("main_program"))
        , gpRegistersMemorySegmentDescriptor(this->cpuAddressSpaceDescriptor.getMemorySegmentDescriptor("gp_registers"))
        , cpuGpPeripheralDescriptor(targetDescriptor.getPeripheralDescriptor("cpu"))
        , cpuGpRegisterGroupDescriptor(this->cpuGpPeripheralDescriptor.getRegisterGroupDescriptor("gpr"))
        , programCounterGdbRegisterId(static_cast<GdbRegisterId>(this->cpuGpRegisterGroupDescriptor.registerDescriptorsByKey.size()))
    {
        // Create the GDB register descriptors and populate the mappings for the general purpose registers (ID 0->31)
        for (const auto& [key, descriptor] : this->cpuGpRegisterGroupDescriptor.registerDescriptorsByKey) {
            if (descriptor.type != TargetRegisterType::GENERAL_PURPOSE_REGISTER) {
                continue;
            }

            const auto gdbRegisterId = static_cast<GdbRegisterId>(
                descriptor.startAddress - this->gpRegistersMemorySegmentDescriptor.addressRange.startAddress
            );

            this->gdbRegisterDescriptorsById.emplace(gdbRegisterId, RegisterDescriptor{gdbRegisterId, 4});
            this->targetRegisterDescriptorsByGdbId.emplace(gdbRegisterId, &descriptor);
        }

        this->gdbRegisterDescriptorsById.emplace(
            this->programCounterGdbRegisterId,
            RegisterDescriptor{this->programCounterGdbRegisterId, 4}
        );
    }
}
