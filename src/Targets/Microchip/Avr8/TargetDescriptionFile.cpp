#include "TargetDescriptionFile.hpp"

#include "src/Services/PathService.hpp"
#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"

#include "src/Exceptions/Exception.hpp"
#include "src/Targets/TargetDescription/Exceptions/TargetDescriptionParsingFailureException.hpp"

namespace Targets::Microchip::Avr8
{
    using Targets::TargetDescription::RegisterGroup;
    using Targets::TargetDescription::AddressSpace;
    using Targets::TargetDescription::MemorySegment;
    using Targets::TargetDescription::Register;
    using Targets::TargetDescription::Exceptions::InvalidTargetDescriptionDataException;
    using Targets::TargetRegisterDescriptor;
    using Services::StringService;

    TargetDescriptionFile::TargetDescriptionFile(const std::string& xmlFilePath)
        : Targets::TargetDescription::TargetDescriptionFile(xmlFilePath)
    {}

    TargetSignature TargetDescriptionFile::getTargetSignature() const {
        const auto& signatureGroup = this->getPropertyGroup("signatures");

        return {
            StringService::toUint8(signatureGroup.getProperty("signature0").value, 16),
            StringService::toUint8(signatureGroup.getProperty("signature1").value, 16),
            StringService::toUint8(signatureGroup.getProperty("signature2").value, 16)
        };
    }

    Family TargetDescriptionFile::getAvrFamily() const {
        static const auto targetFamiliesByName = std::map<std::string, Family>{
            {"MEGA", Family::MEGA},
            {"XMEGA", Family::XMEGA},
            {"TINY", Family::TINY},
            {"DA", Family::DA},
            {"DB", Family::DB},
            {"DD", Family::DD},
            {"DU", Family::DU},
            {"EA", Family::EA},
            {"EB", Family::EB},
        };

        const auto familyIt = targetFamiliesByName.find(this->getDeviceAttribute("avr-family"));
        if (familyIt == targetFamiliesByName.end()) {
            throw InvalidTargetDescriptionDataException{"Unknown AVR family name in target description file"};
        }

        return familyIt->second;
    }

    const TargetDescription::AddressSpace& TargetDescriptionFile::getRegisterFileAddressSpace() const {
        /*
         * On some AVRs, the register file is accessible via the data address space. On the newer UPDI and PDI AVRs,
         * it has a dedicated address space.
         */
        const auto addressSpace = this->tryGetAddressSpace("register_file");
        return addressSpace.has_value()
            ? addressSpace->get()
            : this->getAddressSpace("data");
    }

    const TargetDescription::AddressSpace& TargetDescriptionFile::getProgramAddressSpace() const {
        return this->getAddressSpace("prog");
    }

    const TargetDescription::AddressSpace& TargetDescriptionFile::getDataAddressSpace() const {
        return this->getAddressSpace("data");
    }

    const TargetDescription::AddressSpace& TargetDescriptionFile::getEepromAddressSpace() const {
        const auto addressSpace = this->tryGetAddressSpace("eeprom");
        return addressSpace.has_value()
            ? addressSpace->get()
            : this->getAddressSpace("data");
    }

    const TargetDescription::AddressSpace& TargetDescriptionFile::getIoAddressSpace() const {
        return this->getAddressSpace("data");
    }

    const TargetDescription::AddressSpace& TargetDescriptionFile::getSignatureAddressSpace() const {
        const auto addressSpace = this->tryGetAddressSpace("signatures");
        return addressSpace.has_value()
            ? addressSpace->get()
            : this->getAddressSpace("data");
    }

    const TargetDescription::AddressSpace& TargetDescriptionFile::getFuseAddressSpace() const {
        const auto addressSpace = this->tryGetAddressSpace("fuses");
        return addressSpace.has_value()
            ? addressSpace->get()
            : this->getAddressSpace("data");
    }

    const TargetDescription::AddressSpace& TargetDescriptionFile::getLockbitAddressSpace() const {
        const auto addressSpace = this->tryGetAddressSpace("lockbits");
        return addressSpace.has_value()
            ? addressSpace->get()
            : this->getAddressSpace("data");
    }

    const TargetDescription::MemorySegment& TargetDescriptionFile::getProgramMemorySegment() const {
        return this->getProgramAddressSpace().getMemorySegment("internal_program_memory");
    }

    const TargetDescription::MemorySegment& TargetDescriptionFile::getRamMemorySegment() const {
        return this->getDataAddressSpace().getMemorySegment("internal_ram");
    }

    const TargetDescription::MemorySegment& TargetDescriptionFile::getEepromMemorySegment() const {
        return this->getEepromAddressSpace().getMemorySegment("internal_eeprom");
    }

    const TargetDescription::MemorySegment& TargetDescriptionFile::getIoMemorySegment() const {
        const auto& addressSpace = this->getIoAddressSpace();
        const auto segment = addressSpace.tryGetMemorySegment("io");
        return segment.has_value()
            ? segment->get()
            : addressSpace.getMemorySegment("mapped_io");
    }

    const TargetDescription::MemorySegment& TargetDescriptionFile::getSignatureMemorySegment() const {
        return this->getSignatureAddressSpace().getMemorySegment("signatures");
    }

    const TargetDescription::MemorySegment& TargetDescriptionFile::getFuseMemorySegment() const {
        return this->getFuseAddressSpace().getMemorySegment("fuses");
    }

    const TargetDescription::MemorySegment& TargetDescriptionFile::getLockbitMemorySegment() const {
        return this->getLockbitAddressSpace().getMemorySegment("lockbits");
    }

    TargetAddressSpaceDescriptor TargetDescriptionFile::getProgramAddressSpaceDescriptor() const {
        return this->targetAddressSpaceDescriptorFromAddressSpace(this->getProgramAddressSpace());
    }

    TargetAddressSpaceDescriptor TargetDescriptionFile::getDataAddressSpaceDescriptor() const {
        return this->targetAddressSpaceDescriptorFromAddressSpace(this->getDataAddressSpace());
    }

    TargetAddressSpaceDescriptor TargetDescriptionFile::getFuseAddressSpaceDescriptor() const {
        return this->targetAddressSpaceDescriptorFromAddressSpace(this->getFuseAddressSpace());
    }

    TargetMemorySegmentDescriptor TargetDescriptionFile::getProgramMemorySegmentDescriptor() const {
        return this->targetMemorySegmentDescriptorFromMemorySegment(
            this->getProgramMemorySegment(),
            this->getProgramAddressSpace()
        );
    }

    TargetMemorySegmentDescriptor TargetDescriptionFile::getRamMemorySegmentDescriptor() const {
        return this->targetMemorySegmentDescriptorFromMemorySegment(
            this->getRamMemorySegment(),
            this->getDataAddressSpace()
        );
    }

    TargetMemorySegmentDescriptor TargetDescriptionFile::getFuseMemorySegmentDescriptor() const {
        return this->targetMemorySegmentDescriptorFromMemorySegment(
            this->getFuseMemorySegment(),
            this->getFuseAddressSpace()
        );
    }

    TargetMemorySegmentDescriptor TargetDescriptionFile::getIoMemorySegmentDescriptor() const {
        return this->targetMemorySegmentDescriptorFromMemorySegment(
            this->getIoMemorySegment(),
            this->getIoAddressSpace()
        );
    }

    TargetPeripheralDescriptor TargetDescriptionFile::getFuseTargetPeripheralDescriptor() const {
        return this->getTargetPeripheralDescriptor("fuse");
    }

    Pair<
        TargetRegisterDescriptor,
        TargetBitFieldDescriptor
    > TargetDescriptionFile::getFuseRegisterBitFieldDescriptorPair(const std::string& fuseBitFieldKey) const {
        const auto peripheralDescriptor = this->getFuseTargetPeripheralDescriptor();
        const auto pair = peripheralDescriptor.getRegisterGroupDescriptor("fuse").getRegisterBitFieldDescriptorPair(
            fuseBitFieldKey
        );

        return {pair.first.clone(), pair.second.clone()};
    }

    std::optional<FuseEnableStrategy> TargetDescriptionFile::getFuseEnableStrategy() const {
        const auto fuseEnabledValueProperty = this->tryGetProperty("programming_info", "fuse_enabled_value");
        if (fuseEnabledValueProperty.has_value()) {
            if (fuseEnabledValueProperty->get().value == "0") {
                return FuseEnableStrategy::CLEAR;
            }

            if (fuseEnabledValueProperty->get().value == "1") {
                return FuseEnableStrategy::SET;
            }
        }

        return std::nullopt;
    }
}
