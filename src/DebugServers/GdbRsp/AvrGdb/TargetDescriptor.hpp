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

        explicit TargetDescriptor(const Targets::TargetDescriptor& targetDescriptor);

        /**
         * Should retrieve the GDB register number, given a target register descriptor. Or std::nullopt if the target
         * register descriptor isn't mapped to any GDB register.
         *
         * @param registerDescriptor
         * @return
         */
        std::optional<GdbRegisterNumberType> getRegisterNumberFromTargetRegisterDescriptor(
            const Targets::TargetRegisterDescriptor& registerDescriptor
        ) const override;

        /**
         * Should retrieve the GDB register descriptor for a given GDB register number.
         *
         * @param number
         * @return
         */
        const RegisterDescriptor& getRegisterDescriptorFromNumber(GdbRegisterNumberType number) const override;

        /**
         * Should retrieve the mapped target register descriptor for a given GDB register number.
         *
         * @param number
         * @return
         */
        const Targets::TargetRegisterDescriptor& getTargetRegisterDescriptorFromNumber(
            GdbRegisterNumberType number
        ) const override;

        const std::vector<GdbRegisterNumberType>& getRegisterNumbers() const override;

    private:
        void loadRegisterMappings();
    };
}
