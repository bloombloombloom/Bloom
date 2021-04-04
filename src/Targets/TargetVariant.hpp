#pragma once

#include <string>
#include <cstdint>
#include <map>

#include "TargetPinDescriptor.hpp"

namespace Bloom::Targets
{
    enum class TargetPackage: int {
        UNKNOWN,

        /**
         * Quad flat package (QFP)
         */
        QFP,

        /**
         * Dual inline package (DIP)
         */
        DIP,

        /**
         * Small outline integrated circuit (SOIC) package.
         *
         * Because of the similarities between SOIC and DIP, Insight treats SOIC packages as DIP packages. That is,
         * it uses the same package widget.
         */
        SOIC,

        /**
         * Quad flat no-lead (QFN) package
         */
        QFN,
    };

    struct TargetVariant
    {
        int id;
        std::string name;
        std::string packageName;
        TargetPackage package = TargetPackage::UNKNOWN;
        std::map<int, TargetPinDescriptor> pinDescriptorsByNumber;
    };
}
