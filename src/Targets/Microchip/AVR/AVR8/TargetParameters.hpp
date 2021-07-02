#pragma once

#include <cstdint>
#include <optional>

#include "../TargetSignature.hpp"
#include "Family.hpp"

namespace Bloom::Targets::Microchip::Avr::Avr8Bit
{
    struct TargetParameters
    {
        std::optional<std::uint32_t> bootSectionStartAddress;
        std::optional<std::uint32_t> gpRegisterStartAddress;
        std::optional<std::uint32_t> gpRegisterSize;
        std::optional<std::uint16_t> flashPageSize;
        std::optional<std::uint32_t> flashSize;
        std::optional<std::uint32_t> flashStartAddress;
        std::optional<std::uint16_t> ramStartAddress;
        std::optional<std::uint32_t> ramSize;
        std::optional<std::uint16_t> eepromStartAddress;
        std::optional<std::uint16_t> eepromSize;
        std::optional<std::uint16_t> eepromPageSize;
        std::optional<std::uint8_t> eepromAddressRegisterHigh;
        std::optional<std::uint8_t> eepromAddressRegisterLow;
        std::optional<std::uint8_t> eepromDataRegisterAddress;
        std::optional<std::uint8_t> eepromControlRegisterAddress;
        std::optional<std::uint8_t> ocdRevision;
        std::optional<std::uint8_t> ocdDataRegister;
        std::optional<std::uint16_t> statusRegisterStartAddress;
        std::optional<std::uint16_t> statusRegisterSize;
        std::optional<std::uint16_t> stackPointerRegisterLowAddress;
        std::optional<std::uint16_t> stackPointerRegisterSize;
        std::optional<std::uint8_t> spmcRegisterStartAddress;
        std::optional<std::uint8_t> osccalAddress;

        // XMega/PDI specific target params
        std::optional<std::uint32_t> appSectionPdiOffset;
        std::optional<std::uint16_t> bootSectionSize;
        std::optional<std::uint32_t> bootSectionPdiOffset;
        std::optional<std::uint32_t> eepromPdiOffset;
        std::optional<std::uint32_t> ramPdiOffset;
        std::optional<std::uint32_t> fuseRegistersPdiOffset;
        std::optional<std::uint32_t> lockRegistersPdiOffset;
        std::optional<std::uint32_t> userSignaturesPdiOffset;
        std::optional<std::uint32_t> productSignaturesPdiOffset;
        std::optional<std::uint16_t> nvmBaseAddress;

        // UPDI specific target params
        std::optional<std::uint16_t> ocdModuleAddress;
        std::optional<std::uint32_t> programMemoryUpdiStartAddress;
        std::optional<std::uint16_t> signatureSegmentStartAddress;
        std::optional<std::uint16_t> signatureSegmentSize;
        std::optional<std::uint16_t> fuseSegmentStartAddress;
        std::optional<std::uint16_t> fuseSegmentSize;
        std::optional<std::uint16_t> lockbitsSegmentStartAddress;

        std::optional<std::uint32_t> ioPortAddressRangeStart;
        std::optional<std::uint32_t> ioPortAddressRangeEnd;
    };
}
