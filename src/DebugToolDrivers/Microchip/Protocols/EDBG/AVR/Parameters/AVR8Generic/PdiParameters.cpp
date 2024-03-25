#include "PdiParameters.hpp"

#include "src/Services/StringService.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::Parameters::Avr8Generic
{
    PdiParameters::PdiParameters(
        const Targets::Microchip::Avr::Avr8Bit::TargetDescriptionFile& targetDescriptionFile
    ) {
        using Services::StringService;

        const auto& pdiGroup = targetDescriptionFile.getPropertyGroup("pdi_interface");

        this->appSectionPdiOffset = StringService::toUint32(pdiGroup.getProperty("app_section_offset").value);
        this->bootSectionPdiOffset = StringService::toUint32(pdiGroup.getProperty("boot_section_offset").value);
        this->eepromPdiOffset = StringService::toUint32(pdiGroup.getProperty("eeprom_offset").value);
        this->fuseRegistersPdiOffset = StringService::toUint32(pdiGroup.getProperty("fuse_registers_offset").value);
        this->lockRegistersPdiOffset = StringService::toUint32(pdiGroup.getProperty("lock_registers_offset").value);
        this->userSignaturesPdiOffset = StringService::toUint32(pdiGroup.getProperty("user_signatures_offset").value);
        this->prodSignaturesPdiOffset = StringService::toUint32(pdiGroup.getProperty("prod_signatures_offset").value);
        this->ramPdiOffset = StringService::toUint32(pdiGroup.getProperty("datamem_offset").value);
        this->signaturesPdiOffset = StringService::toUint16(pdiGroup.getProperty("signature_offset").value);

        const auto& programMemorySegment = targetDescriptionFile.getProgramMemorySegment();
        const auto& eepromMemorySegment = targetDescriptionFile.getEepromMemorySegment();

        this->appSectionSize = programMemorySegment.getSection("app_section").size;
        this->bootSectionSize = static_cast<std::uint16_t>(programMemorySegment.getSection("boot_section").size);
        this->flashPageSize = static_cast<std::uint16_t>(programMemorySegment.pageSize.value());
        this->eepromSize = static_cast<std::uint16_t>(eepromMemorySegment.size);
        this->eepromPageSize = static_cast<std::uint8_t>(eepromMemorySegment.pageSize.value());

        this->nvmModuleBaseAddress = static_cast<std::uint16_t>(
            targetDescriptionFile.getTargetPeripheralDescriptor("nvm").getRegisterGroupDescriptor("nvm").startAddress()
        );
    }
}
