#pragma once

#include <cstdint>
#include <optional>
#include <functional>

#include "src/Targets/Microchip/AVR8/TargetDescriptionFile.hpp"
#include "src/Targets/Microchip/AVR8/Avr8TargetConfig.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"
#include "src/Targets/TargetRegisterDescriptor.hpp"

#include "Avr8Generic.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr
{
    /**
     * This struct holds all required target info for an EDBG AVR8 session.
     */
    struct EdbgAvr8Session
    {
        /**
         * AVR8 TDF, from which we extract all target info to configure the EDBG debug tool.
         */
        const Targets::Microchip::Avr8::TargetDescriptionFile& targetDescriptionFile;

        /**
         * Project's AVR8 target configuration.
         */
        const Targets::Microchip::Avr8::Avr8TargetConfig& targetConfig;

        /**
         * The EDBG config variant parameter.
         *
         * See the "AVR8_CONFIG_VARIANT" parameter in section 7.1.3.1 of Microchip's "EDBG-based Tools Protocols"
         * document for more.
         */
        Avr8ConfigVariant configVariant = Avr8ConfigVariant::NONE;

        const Targets::TargetDescription::AddressSpace& programAddressSpace;
        const Targets::TargetDescription::AddressSpace& registerFileAddressSpace;
        const Targets::TargetDescription::AddressSpace& dataAddressSpace;
        const Targets::TargetDescription::AddressSpace& eepromAddressSpace;
        const Targets::TargetDescription::AddressSpace& ioAddressSpace;
        const Targets::TargetDescription::AddressSpace& signatureAddressSpace;

        const Targets::TargetDescription::MemorySegment& programMemorySegment;
        const Targets::TargetDescription::MemorySegment& ramMemorySegment;
        const Targets::TargetDescription::MemorySegment& eepromMemorySegment;
        const Targets::TargetDescription::MemorySegment& ioMemorySegment;
        const Targets::TargetDescription::MemorySegment& fuseMemorySegment;
        const Targets::TargetDescription::MemorySegment& signatureMemorySegment;

        const std::optional<
            std::reference_wrapper<const Targets::TargetDescription::MemorySegmentSection>
        > programAppSection;

        const std::optional<
            std::reference_wrapper<const Targets::TargetDescription::MemorySegmentSection>
        > programBootSection;

        std::optional<std::uint8_t> ocdDataRegister;

        EdbgAvr8Session(
            const Targets::Microchip::Avr8::TargetDescriptionFile& targetDescriptionFile,
            const Targets::Microchip::Avr8::Avr8TargetConfig& targetConfig
        );

    private:
        /**
         * Attempts to determine the EDBG config variant for a given AVR family and physical interface.
         *
         * See the "AVR8_CONFIG_VARIANT" parameter in section 7.1.3.1 of Microchip's "EDBG-based Tools Protocols"
         * document for more.
         *
         * @return
         *  The resolved config variant, or std::nullopt if the given AVR family and physical interface do not map to
         *  any particular EDBG config variant.
         */
        static std::optional<Avr8ConfigVariant> tryResolveConfigVariant(
            Targets::Microchip::Avr8::Family avrFamily,
            Targets::TargetPhysicalInterface physicalInterface
        );
    };
}
