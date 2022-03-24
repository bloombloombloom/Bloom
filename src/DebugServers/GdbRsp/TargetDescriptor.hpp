#pragma once

#include <cstdint>
#include <optional>

#include "RegisterDescriptor.hpp"

#include "src/Targets/TargetDescriptor.hpp"
#include "src/Targets/TargetRegister.hpp"

namespace Bloom::DebugServers::Gdb
{
    struct TargetDescriptor
    {
        const Bloom::Targets::TargetDescriptor& targetDescriptor;

        explicit TargetDescriptor(const Bloom::Targets::TargetDescriptor& targetDescriptor)
            : targetDescriptor(targetDescriptor)
        {}

        /**
         * Should retrieve the GDB register number, given a target register descriptor. Or std::nullopt if the target
         * register descriptor isn't mapped to any GDB register.
         *
         * @param registerDescriptor
         * @return
         */
        virtual std::optional<GdbRegisterNumberType> getRegisterNumberFromTargetRegisterDescriptor(
            const Targets::TargetRegisterDescriptor& registerDescriptor
        ) = 0;

        /**
         * Should retrieve the GDB register descriptor for a given GDB register number.
         *
         * @param number
         * @return
         */
        virtual const RegisterDescriptor& getRegisterDescriptorFromNumber(GdbRegisterNumberType number) = 0;

        /**
         * Should retrieve the mapped target register descriptor for a given GDB register number.
         *
         * @param number
         * @return
         */
        virtual const Targets::TargetRegisterDescriptor& getTargetRegisterDescriptorFromNumber(
            GdbRegisterNumberType number
        ) = 0;
    };
}
