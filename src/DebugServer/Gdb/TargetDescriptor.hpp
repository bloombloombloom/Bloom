#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "src/Targets/TargetDescriptor.hpp"
#include "src/Targets/TargetRegister.hpp"

#include "RegisterDescriptor.hpp"

namespace Bloom::DebugServer::Gdb
{
    /**
     * GDB target descriptor.
     */
    struct TargetDescriptor
    {
        Targets::TargetDescriptor targetDescriptor;

        explicit TargetDescriptor(const Targets::TargetDescriptor& targetDescriptor)
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
        ) const = 0;

        /**
         * Should retrieve the GDB register descriptor for a given GDB register number.
         *
         * @param number
         * @return
         */
        virtual const RegisterDescriptor& getRegisterDescriptorFromNumber(GdbRegisterNumberType number) const = 0;

        /**
         * Should retrieve the mapped target register descriptor for a given GDB register number.
         *
         * @param number
         * @return
         */
        virtual const Targets::TargetRegisterDescriptor& getTargetRegisterDescriptorFromNumber(
            GdbRegisterNumberType number
        ) const = 0;

        /**
         * Should return all allocated GDB register numbers for the target.
         *
         * @return
         */
        virtual const std::vector<GdbRegisterNumberType>& getRegisterNumbers() const = 0;
    };
}
