#include "UpdiParameters.hpp"

#include "src/Services/StringService.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::Parameters::Avr8Generic
{
    UpdiParameters::UpdiParameters(
        const Targets::Microchip::Avr::Avr8Bit::TargetDescriptionFile& targetDescriptionFile
    ) {
        using Services::StringService;

        const auto& updiGroup = targetDescriptionFile.getPropertyGroup("updi_interface");

        this->programMemoryStartAddress = StringService::toUint32(updiGroup.getProperty("progmem_offset").value);
        this->ocdModuleAddress = StringService::toUint16(updiGroup.getProperty("ocd_base_addr").value);

        const auto& programMemorySegment = targetDescriptionFile.getProgramMemorySegment();
        const auto& eepromMemorySegment = targetDescriptionFile.getEepromMemorySegment();
        const auto& signatureMemorySegment = targetDescriptionFile.getSignatureMemorySegment();
        const auto& fuseMemorySegment = targetDescriptionFile.getFuseMemorySegment();
        const auto& lockbitMemorySegment = targetDescriptionFile.getLockbitMemorySegment();

        this->flashSize = programMemorySegment.size;
        this->flashPageSize = static_cast<std::uint16_t>(programMemorySegment.pageSize.value());
        this->eepromStartAddress = static_cast<std::uint16_t>(eepromMemorySegment.startAddress);
        this->eepromSize = static_cast<std::uint16_t>(eepromMemorySegment.size);
        this->eepromPageSize = static_cast<std::uint8_t>(eepromMemorySegment.pageSize.value());
        this->signatureSegmentStartAddress = static_cast<std::uint16_t>(signatureMemorySegment.startAddress);
        this->fuseSegmentSize = static_cast<std::uint16_t>(fuseMemorySegment.size);
        this->fuseSegmentStartAddress = static_cast<std::uint16_t>(fuseMemorySegment.startAddress);
        this->lockbitSegmentStartAddress = static_cast<std::uint16_t>(lockbitMemorySegment.startAddress);

        const auto nvmCtrlPeripheralDescriptor = targetDescriptionFile.getTargetPeripheralDescriptor("nvmctrl");
        this->nvmModuleBaseAddress = static_cast<std::uint16_t>(
            nvmCtrlPeripheralDescriptor.getRegisterGroupDescriptor("nvmctrl").startAddress()
        );
    }
}
