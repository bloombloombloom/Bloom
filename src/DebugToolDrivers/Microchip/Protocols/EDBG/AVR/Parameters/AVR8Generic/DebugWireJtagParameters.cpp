#include "DebugWireJtagParameters.hpp"

#include "src/Services/StringService.hpp"

#include "src/Exceptions/InternalFatalErrorException.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::Parameters::Avr8Generic
{
    DebugWireJtagParameters::DebugWireJtagParameters(
        const Targets::Microchip::Avr::Avr8Bit::TargetDescriptionFile& targetDescriptionFile
    ) {
        using Services::StringService;

        const auto& programMemorySegment = targetDescriptionFile.getProgramMemorySegment();
        const auto& ramMemorySegment = targetDescriptionFile.getRamMemorySegment();
        const auto& eepromMemorySegment = targetDescriptionFile.getEepromMemorySegment();

        this->flashPageSize = static_cast<std::uint16_t>(programMemorySegment.pageSize.value());
        this->flashSize = programMemorySegment.size;
        this->flashStartWordAddress = static_cast<std::uint32_t>(programMemorySegment.startAddress / 2);

        const auto firstBootSectionOptionGroup = targetDescriptionFile.tryGetPropertyGroup(
            "boot_section_options.boot_section_1"
        );
        if (firstBootSectionOptionGroup.has_value()) {
            this->bootSectionStartWordAddress = static_cast<std::uint32_t>(
                StringService::toUint32(firstBootSectionOptionGroup->get().getProperty("start_address").value) / 2
            );
        }

        this->ramStartAddress = static_cast<std::uint16_t>(ramMemorySegment.startAddress);
        this->eepromSize = static_cast<std::uint16_t>(eepromMemorySegment.size);
        this->eepromPageSize = static_cast<std::uint8_t>(eepromMemorySegment.pageSize.value());

        const auto& ocdPropertyGroup = targetDescriptionFile.getPropertyGroup("ocd");
        this->ocdRevision = StringService::toUint8(ocdPropertyGroup.getProperty("ocd_revision").value);
        this->ocdDataRegister = StringService::toUint8(ocdPropertyGroup.getProperty("ocd_datareg").value);

        const auto& eepromRegisterGroupDescriptor = targetDescriptionFile.getTargetPeripheralDescriptor("eeprom")
            .getRegisterGroupDescriptor("eeprom");

        const auto& eearDescriptor = eepromRegisterGroupDescriptor.tryGetRegisterDescriptor("eear");
        if (eearDescriptor.has_value()) {
            const auto startAddress = eearDescriptor->get().startAddress;
            this->eepromAddressRegisterLow = static_cast<std::uint8_t>(startAddress);
            this->eepromAddressRegisterHigh = static_cast<std::uint8_t>(
                eearDescriptor->get().size > 1 ? startAddress >> 2 : startAddress
            );

        } else {
            const auto& eearlDescriptor = eepromRegisterGroupDescriptor.getRegisterDescriptor("eearl");
            this->eepromAddressRegisterLow = static_cast<std::uint8_t>(eearlDescriptor.startAddress);
            this->eepromAddressRegisterHigh = static_cast<std::uint8_t>(
                eearlDescriptor.size > 1 ? eearlDescriptor.startAddress >> 2 : eearlDescriptor.startAddress
            );

            const auto eearhDescriptor = eepromRegisterGroupDescriptor.tryGetRegisterDescriptor("eearh");
            if (eearhDescriptor.has_value()) {
                this->eepromAddressRegisterHigh = static_cast<std::uint8_t>(eearhDescriptor->get().startAddress);
            }
        }

        this->eepromDataRegisterAddress = static_cast<std::uint8_t>(
            eepromRegisterGroupDescriptor.getRegisterDescriptor("eedr").startAddress
        );

        this->eepromControlRegisterAddress = static_cast<std::uint8_t>(
            eepromRegisterGroupDescriptor.getRegisterDescriptor("eecr").startAddress
        );

        const auto& cpuRegisterGroupDescriptor = targetDescriptionFile.getTargetPeripheralDescriptor("cpu")
            .getRegisterGroupDescriptor("cpu");

        const auto spmcsrDescriptor = cpuRegisterGroupDescriptor.tryGetRegisterDescriptor("spmcsr")
            ?: cpuRegisterGroupDescriptor.tryGetRegisterDescriptor("spmcr");

        if (spmcsrDescriptor.has_value()) {
            this->spmcRegisterStartAddress = static_cast<std::uint8_t>(spmcsrDescriptor->get().startAddress);

        } else {
            const auto& bootLoaderRegisterGroupDescriptor = targetDescriptionFile.getTargetPeripheralDescriptor(
                "boot_load"
            ).getRegisterGroupDescriptor("boot_load");

            const auto spmcsrDescriptor = bootLoaderRegisterGroupDescriptor.tryGetRegisterDescriptor("spmcsr")
                ?: bootLoaderRegisterGroupDescriptor.tryGetRegisterDescriptor("spmcr");

            if (!spmcsrDescriptor.has_value()) {
                throw Exceptions::InternalFatalErrorException("Could not extract SPMCS register from TDF");
            }

            this->spmcRegisterStartAddress = static_cast<std::uint8_t>(spmcsrDescriptor->get().startAddress);
        }

        const auto osccalDescriptor = cpuRegisterGroupDescriptor.tryGetRegisterDescriptor("osccal")
            ?: cpuRegisterGroupDescriptor.tryGetRegisterDescriptor("osccal0")
            ?: cpuRegisterGroupDescriptor.tryGetRegisterDescriptor("osccal1")
            ?: cpuRegisterGroupDescriptor.tryGetRegisterDescriptor("fosccal")
            ?: cpuRegisterGroupDescriptor.tryGetRegisterDescriptor("sosccala");

        if (!osccalDescriptor.has_value()) {
            throw Exceptions::InternalFatalErrorException("Could not extract OSCCAL register from TDF");
        }

        this->osccalAddress = static_cast<std::uint8_t>(osccalDescriptor->get().startAddress);

        /*
         * All addresses for registers that reside in the IO memory segment include the IO segment offset
         * (start address). But the EDBG protocol requires *some* of these addresses to be stripped of this offset
         * before sending them as target parameters.
         *
         * This applies to the following addresses:
         *
         *  - OSCALL Address
         *  - EEARL Address
         *  - EEARH Address
         *  - EECR Address
         *  - EEDR Address
         *
         * It does *not* seem to apply to the SPMCR or OCDDR address.
         */
        const auto& ioMemorySegment = targetDescriptionFile.getIoMemorySegment();

        this->osccalAddress -= ioMemorySegment.startAddress;
        this->eepromAddressRegisterLow -= ioMemorySegment.startAddress;
        this->eepromAddressRegisterHigh -= ioMemorySegment.startAddress;
        this->eepromControlRegisterAddress -= ioMemorySegment.startAddress;
        this->eepromDataRegisterAddress -= ioMemorySegment.startAddress;
    }
}
