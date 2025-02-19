#include "DebugWireJtagParameters.hpp"

#include "src/Services/StringService.hpp"
#include "src/Exceptions/InternalFatalErrorException.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::Parameters::Avr8Generic
{
    DebugWireJtagParameters::DebugWireJtagParameters(
        const Targets::Microchip::Avr8::TargetDescriptionFile& targetDescriptionFile
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
            this->bootSectionStartWordAddress = StringService::toUint32(
                firstBootSectionOptionGroup->get().getProperty("start_address").value
            ) / 2;
        }

        this->ramStartAddress = static_cast<std::uint16_t>(ramMemorySegment.startAddress);
        this->eepromSize = static_cast<std::uint16_t>(eepromMemorySegment.size);
        this->eepromPageSize = static_cast<std::uint8_t>(eepromMemorySegment.pageSize.value());

        const auto& ocdPropertyGroup = targetDescriptionFile.getPropertyGroup("ocd");
        this->ocdRevision = StringService::toUint8(ocdPropertyGroup.getProperty("ocd_revision").value);
        this->ocdDataRegisterAddress = StringService::toUint8(ocdPropertyGroup.getProperty("ocd_datareg").value);

        const auto buffersPerFlashPageProperty = ocdPropertyGroup.tryGetProperty("buffers_per_flash_page");
        if (buffersPerFlashPageProperty.has_value()) {
            this->buffersPerFlashPage = StringService::toUint8(buffersPerFlashPageProperty->get().value);
        }

        const auto eepromPeripheralDescriptor = targetDescriptionFile.getTargetPeripheralDescriptor("eeprom");
        const auto& eepromRegisterGroupDescriptor = eepromPeripheralDescriptor.getRegisterGroupDescriptor("eeprom");

        const auto& eearDescriptor = eepromRegisterGroupDescriptor.tryGetRegisterDescriptor("eear");
        if (eearDescriptor.has_value()) {
            const auto startAddress = eearDescriptor->get().startAddress;
            this->eearAddressLow = static_cast<std::uint8_t>(startAddress);

            /*
             * If the target doesn't have a high byte in the `EEAR` address, the `eearAddressHigh` parameter should be
             * equal to the `eearAddressLow` parameter, as stated in the "EDBG-based Tools Protocols" document.
             */
            this->eearAddressHigh = static_cast<std::uint8_t>(startAddress + (eearDescriptor->get().size - 1));

        } else {
            const auto& eearlDescriptor = eepromRegisterGroupDescriptor.getRegisterDescriptor("eearl");
            this->eearAddressLow = static_cast<std::uint8_t>(eearlDescriptor.startAddress);

            /*
             * Some debugWIRE targets only have a single-byte `EEARL` register.
             *
             * In the absence of an `EEARH` register, and if there is no high byte in the `EEARL` register, the
             * `eearAddressHigh` parameter should be equal to the `eearAddressLow` parameter, as stated in the
             * "EDBG-based Tools Protocols" document.
             */
            const auto eearhDescriptor = eepromRegisterGroupDescriptor.tryGetRegisterDescriptor("eearh");
            this->eearAddressHigh = static_cast<std::uint8_t>(
                eearhDescriptor.has_value()
                    ? eearhDescriptor->get().startAddress
                    : eearlDescriptor.startAddress + (eearlDescriptor.size - 1)
            );
        }

        this->eedrAddress = static_cast<std::uint8_t>(
            eepromRegisterGroupDescriptor.getRegisterDescriptor("eedr").startAddress
        );

        this->eecrAddress = static_cast<std::uint8_t>(
            eepromRegisterGroupDescriptor.getRegisterDescriptor("eecr").startAddress
        );

        const auto cpuPeripheralDescriptor = targetDescriptionFile.getTargetPeripheralDescriptor("cpu");
        const auto& cpuRegisterGroupDescriptor = cpuPeripheralDescriptor.getRegisterGroupDescriptor("cpu");

        const auto spmcsrDescriptor = cpuRegisterGroupDescriptor.tryGetFirstRegisterDescriptor({"spmcsr", "spmcr"});

        if (spmcsrDescriptor.has_value()) {
            this->spmcrAddress = static_cast<std::uint8_t>(spmcsrDescriptor->get().startAddress);

        } else {
            const auto bootLoadPeripheral = targetDescriptionFile.getTargetPeripheralDescriptor("boot_load");
            const auto& bootLoaderRegisterGroupDescriptor = bootLoadPeripheral.getRegisterGroupDescriptor("boot_load");

            const auto spmcsrDescriptor = bootLoaderRegisterGroupDescriptor.tryGetFirstRegisterDescriptor(
                {"spmcsr", "spmcr"}
            );

            if (!spmcsrDescriptor.has_value()) {
                throw Exceptions::InternalFatalErrorException{"Could not extract SPMCS register from TDF"};
            }

            this->spmcrAddress = static_cast<std::uint8_t>(spmcsrDescriptor->get().startAddress);
        }

        const auto osccalDescriptor = cpuRegisterGroupDescriptor.tryGetFirstRegisterDescriptor({
            "osccal",
            "osccal0",
            "osccal1",
            "fosccal",
            "sosccala",
        });

        if (!osccalDescriptor.has_value()) {
            throw Exceptions::InternalFatalErrorException{"Could not extract OSCCAL register from TDF"};
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
        const auto& ioSegmentStartAddress = static_cast<std::uint8_t>(
            targetDescriptionFile.getIoMemorySegment().startAddress
        );

        // This is enforced in TDF validation
        assert(this->osccalAddress >= ioSegmentStartAddress);
        assert(this->eearAddressLow >= ioSegmentStartAddress);
        assert(this->eearAddressHigh >= ioSegmentStartAddress);
        assert(this->eecrAddress >= ioSegmentStartAddress);
        assert(this->eedrAddress >= ioSegmentStartAddress);

        this->osccalAddress -= ioSegmentStartAddress;
        this->eearAddressLow -= ioSegmentStartAddress;
        this->eearAddressHigh -= ioSegmentStartAddress;
        this->eecrAddress -= ioSegmentStartAddress;
        this->eedrAddress -= ioSegmentStartAddress;
    }
}
