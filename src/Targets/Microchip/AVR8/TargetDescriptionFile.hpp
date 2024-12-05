#pragma once

#include <set>
#include <optional>

#include "src/Targets/TargetDescription/TargetDescriptionFile.hpp"

#include "src/Targets/TargetRegisterDescriptor.hpp"

#include "src/Targets/Microchip/AVR8/TargetSignature.hpp"
#include "src/Targets/Microchip/AVR8/Fuse.hpp"

#include "src/Targets/Microchip/AVR8/Family.hpp"

namespace Targets::Microchip::Avr8
{
    /**
     * AVR8 TDF
     *
     * For more information of TDFs, see src/Targets/TargetDescription/README.md
     */
    class TargetDescriptionFile: public TargetDescription::TargetDescriptionFile
    {
    public:
        /**
         * Extends TDF initialisation to include the loading of physical interfaces for debugging AVR8 targets, among
         * other things.
         *
         * @param xml
         */
        explicit TargetDescriptionFile(const std::string& xmlFilePath);

        /**
         * Extracts the AVR8 target signature from the TDF.
         *
         * @return
         */
        [[nodiscard]] TargetSignature getTargetSignature() const;

        /**
         * Extracts the AVR8 target family from the TDF.
         *
         * @return
         */
        [[nodiscard]] Family getAvrFamily() const;

        [[nodiscard]] const TargetDescription::AddressSpace& getRegisterFileAddressSpace() const;
        [[nodiscard]] const TargetDescription::AddressSpace& getProgramAddressSpace() const;
        [[nodiscard]] const TargetDescription::AddressSpace& getDataAddressSpace() const;
        [[nodiscard]] const TargetDescription::AddressSpace& getEepromAddressSpace() const;
        [[nodiscard]] const TargetDescription::AddressSpace& getIoAddressSpace() const;
        [[nodiscard]] const TargetDescription::AddressSpace& getSignatureAddressSpace() const;
        [[nodiscard]] const TargetDescription::AddressSpace& getFuseAddressSpace() const;
        [[nodiscard]] const TargetDescription::AddressSpace& getLockbitAddressSpace() const;

        [[nodiscard]] const TargetDescription::MemorySegment& getProgramMemorySegment() const;
        [[nodiscard]] const TargetDescription::MemorySegment& getRamMemorySegment() const;
        [[nodiscard]] const TargetDescription::MemorySegment& getEepromMemorySegment() const;
        [[nodiscard]] const TargetDescription::MemorySegment& getIoMemorySegment() const;
        [[nodiscard]] const TargetDescription::MemorySegment& getSignatureMemorySegment() const;
        [[nodiscard]] const TargetDescription::MemorySegment& getFuseMemorySegment() const;
        [[nodiscard]] const TargetDescription::MemorySegment& getLockbitMemorySegment() const;

        [[nodiscard]] TargetAddressSpaceDescriptor getProgramAddressSpaceDescriptor() const;
        [[nodiscard]] TargetAddressSpaceDescriptor getDataAddressSpaceDescriptor() const;
        [[nodiscard]] TargetAddressSpaceDescriptor getFuseAddressSpaceDescriptor() const;

        [[nodiscard]] TargetMemorySegmentDescriptor getProgramMemorySegmentDescriptor() const;
        [[nodiscard]] TargetMemorySegmentDescriptor getRamMemorySegmentDescriptor() const;
        [[nodiscard]] TargetMemorySegmentDescriptor getFuseMemorySegmentDescriptor() const;
        [[nodiscard]] TargetMemorySegmentDescriptor getIoMemorySegmentDescriptor() const;

        [[nodiscard]] TargetPeripheralDescriptor getFuseTargetPeripheralDescriptor() const;
        [[nodiscard]] Pair<
            TargetRegisterDescriptor,
            TargetBitFieldDescriptor
        > getFuseRegisterBitFieldDescriptorPair(const std::string& fuseBitFieldKey) const;

        /**
         * Extracts the target's fuse enable strategy.
         *
         * @return
         *  std::nullopt if the TDF doesn't contain a fuse enable strategy.
         */
        [[nodiscard]] std::optional<FuseEnableStrategy> getFuseEnableStrategy() const;
    };
}
