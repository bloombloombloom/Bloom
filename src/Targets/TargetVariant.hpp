#pragma once

#include <cstdint>
#include <string>
#include <map>

#include "TargetPinDescriptor.hpp"

namespace Targets
{
    enum class TargetPackage: std::uint8_t
    {
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
         * "Small outline integrated circuit" package (SOIC).
         *
         * Because of the similarities between SOIC and DIP, Insight treats SOIC packages as DIP packages. That is,
         * it uses the same package widget.
         */
        SOIC,

        /**
         * "Shrink small outline" package (SSOP)
         *
         * Because of the similarities between this and DIP, Insight treats SSOP packages as DIP packages. That is,
         * it uses the same package widget.
         */
        SSOP,

        /**
         * Quad flat no-lead (QFN) package
         *
         * Because of the similarities between this and QFP, Insight treats QFN packages as QFP.
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

        bool operator == (const TargetVariant& variant) const {
            return this->name == variant.name
                && this->packageName == variant.packageName
                && this->package == variant.package
                && this->pinDescriptorsByNumber == variant.pinDescriptorsByNumber;
        }
    };
}
