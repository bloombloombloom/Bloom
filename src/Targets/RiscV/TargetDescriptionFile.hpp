#pragma once

#include "src/Targets/TargetDescription/TargetDescriptionFile.hpp"

namespace Targets::RiscV
{
    /**
     * Represents an RISC-V TDF.
     *
     * For more information of TDFs, see src/Targets/TargetDescription/README.md
     */
    class TargetDescriptionFile: public Targets::TargetDescription::TargetDescriptionFile
    {
    public:
        explicit TargetDescriptionFile(const std::string& xmlFilePath);

        /**
         * Returns the RISC-V target ID from the TDF.
         *
         * @return
         */
        [[nodiscard]] std::string getTargetId() const;

        [[nodiscard]] TargetAddressSpaceDescriptor getSystemAddressSpaceDescriptor() const;
    };
}
