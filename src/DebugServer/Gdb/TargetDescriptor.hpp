#pragma once

#include <cstdint>
#include <optional>
#include <vector>
#include <set>
#include <map>

#include "src/Targets/TargetDescriptor.hpp"
#include "src/Targets/TargetRegisterDescriptor.hpp"
#include "src/Targets/TargetMemory.hpp"

#include "RegisterDescriptor.hpp"

namespace DebugServer::Gdb
{
    using GdbMemoryAddress = std::uint32_t;

    /**
     * Generic GDB target descriptor.
     */
    class TargetDescriptor
    {
    public:
        std::map<GdbRegisterId, RegisterDescriptor> gdbRegisterDescriptorsById;
        std::map<GdbRegisterId, const Targets::TargetRegisterDescriptor*> targetRegisterDescriptorsByGdbId;

        virtual ~TargetDescriptor() = default;
    };
}
