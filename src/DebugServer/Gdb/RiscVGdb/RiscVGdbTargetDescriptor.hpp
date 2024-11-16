#pragma once

#include "src/DebugServer/Gdb/TargetDescriptor.hpp"

#include "src/Targets/TargetDescriptor.hpp"
#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"
#include "src/Targets/TargetPeripheralDescriptor.hpp"
#include "src/Targets/TargetRegisterGroupDescriptor.hpp"

namespace DebugServer::Gdb::RiscVGdb
{
    class RiscVGdbTargetDescriptor: public DebugServer::Gdb::TargetDescriptor
    {
    public:
        const Targets::TargetAddressSpaceDescriptor& systemAddressSpaceDescriptor;
        const Targets::TargetAddressSpaceDescriptor& cpuAddressSpaceDescriptor;

        const Targets::TargetMemorySegmentDescriptor& programMemorySegmentDescriptor;
        const Targets::TargetMemorySegmentDescriptor& gpRegistersMemorySegmentDescriptor;

        const Targets::TargetPeripheralDescriptor& cpuGpPeripheralDescriptor;
        const Targets::TargetRegisterGroupDescriptor& cpuGpRegisterGroupDescriptor;

        const GdbRegisterId programCounterGdbRegisterId;

        explicit RiscVGdbTargetDescriptor(const Targets::TargetDescriptor& targetDescriptor);
    };
}
