#include "EdbgAvr8Session.hpp"

#include "src/Services/StringService.hpp"

#include "src/Exceptions/Exception.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr
{
    EdbgAvr8Session::EdbgAvr8Session(
        const Targets::Microchip::Avr8::TargetDescriptionFile& targetDescriptionFile,
        const Targets::Microchip::Avr8::Avr8TargetConfig& targetConfig
    )
        : targetDescriptionFile(targetDescriptionFile)
        , targetConfig(targetConfig)
        , programAddressSpace(this->targetDescriptionFile.getProgramAddressSpace())
        , registerFileAddressSpace(this->targetDescriptionFile.getRegisterFileAddressSpace())
        , dataAddressSpace(this->targetDescriptionFile.getDataAddressSpace())
        , eepromAddressSpace(this->targetDescriptionFile.getEepromAddressSpace())
        , ioAddressSpace(this->targetDescriptionFile.getIoAddressSpace())
        , signatureAddressSpace(this->targetDescriptionFile.getSignatureAddressSpace())
        , programMemorySegment(this->targetDescriptionFile.getProgramMemorySegment())
        , ramMemorySegment(this->targetDescriptionFile.getRamMemorySegment())
        , eepromMemorySegment(this->targetDescriptionFile.getEepromMemorySegment())
        , ioMemorySegment(this->targetDescriptionFile.getIoMemorySegment())
        , signatureMemorySegment(this->targetDescriptionFile.getSignatureMemorySegment())
        , programAppSection(this->programMemorySegment.tryGetSection("app_section"))
        , programBootSection(this->programMemorySegment.tryGetSection("boot_section"))
    {
        using Services::StringService;

        const auto ocdDataRegisterProperty = this->targetDescriptionFile.tryGetProperty("ocd", "ocd_datareg");
        if (ocdDataRegisterProperty.has_value()) {
            this->ocdDataRegister = StringService::toUint8(ocdDataRegisterProperty->get().value);
        }

        const auto resolvedConfigVariant = EdbgAvr8Session::tryResolveConfigVariant(
            this->targetDescriptionFile.getAvrFamily(),
            this->targetConfig.physicalInterface
        );

        if (!resolvedConfigVariant.has_value()) {
            throw Exceptions::Exception{
                "Failed to resolve EDBG config variant from the selected physical interface and the AVR target family"
                    " - please review the selected physical interface"
            };
        }

        this->configVariant = *resolvedConfigVariant;
    }

    std::optional<Avr8ConfigVariant> EdbgAvr8Session::tryResolveConfigVariant(
        Targets::Microchip::Avr8::Family avrFamily,
        Targets::TargetPhysicalInterface physicalInterface
    ) {
        using Targets::Microchip::Avr8::Family;
        using Targets::TargetPhysicalInterface;

        if (avrFamily == Family::MEGA || avrFamily == Family::TINY) {
            switch (physicalInterface) {
                case TargetPhysicalInterface::JTAG: {
                    return Avr8ConfigVariant::MEGAJTAG;
                }
                case TargetPhysicalInterface::DEBUG_WIRE: {
                    return Avr8ConfigVariant::DEBUG_WIRE;
                }
                case TargetPhysicalInterface::UPDI: {
                    return Avr8ConfigVariant::UPDI;
                }
                default: {
                    break;
                }
            }
        }

        if (avrFamily == Family::XMEGA) {
            switch (physicalInterface) {
                case TargetPhysicalInterface::JTAG:
                case TargetPhysicalInterface::PDI: {
                    return Avr8ConfigVariant::XMEGA;
                }
                default: {
                    break;
                }
            }
        }

        if (avrFamily == Family::DA || avrFamily == Family::DB || avrFamily == Family::DD || avrFamily == Family::EA) {
            switch (physicalInterface) {
                case TargetPhysicalInterface::UPDI: {
                    return Avr8ConfigVariant::UPDI;
                }
                default: {
                    break;
                }
            }
        }

        return std::nullopt;
    }
}
