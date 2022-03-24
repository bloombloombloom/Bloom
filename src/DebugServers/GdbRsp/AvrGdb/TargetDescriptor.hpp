#pragma once

#include "src/DebugServers/GdbRsp/TargetDescriptor.hpp"

#include "src/Helpers/BiMap.hpp"

namespace Bloom::DebugServers::Gdb::AvrGdb
{
    class TargetDescriptor: public DebugServers::Gdb::TargetDescriptor
    {
    public:
        BiMap<GdbRegisterNumberType, RegisterDescriptor> registerDescriptorsByGdbNumber = {};
        BiMap<GdbRegisterNumberType, Targets::TargetRegisterDescriptor> targetRegisterDescriptorsByGdbNumber = {};

        const Bloom::Targets::TargetDescriptor& targetDescriptor;

        explicit TargetDescriptor(const Bloom::Targets::TargetDescriptor& targetDescriptor);

        /**
         * Should retrieve the GDB register number, given a target register descriptor. Or std::nullopt if the target
         * register descriptor isn't mapped to any GDB register.
         *
         * @param registerDescriptor
         * @return
         */
        std::optional<GdbRegisterNumberType> getRegisterNumberFromTargetRegisterDescriptor(
            const Targets::TargetRegisterDescriptor& registerDescriptor
        ) override;

        /**
         * Should retrieve the GDB register descriptor for a given GDB register number.
         *
         * @param number
         * @return
         */
        const RegisterDescriptor& getRegisterDescriptorFromNumber(GdbRegisterNumberType number) override;

        /**
         * Should retrieve the mapped target register descriptor for a given GDB register number.
         *
         * @param number
         * @return
         */
        const Targets::TargetRegisterDescriptor& getTargetRegisterDescriptorFromNumber(
            GdbRegisterNumberType number
        ) override;

    private:
        void loadRegisterMappings();
    };
}
