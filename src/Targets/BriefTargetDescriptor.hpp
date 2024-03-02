#pragma once

#include <string>
#include <cstdint>

#include "TargetFamily.hpp"

namespace Targets
{
    /**
     * The BriefTargetDescriptor struct provides limited information on a particular target.
     *
     * This struct serves as a starting point for constructing a Target object - it provides us with all the relevant
     * information to construct a TargetDescriptionFile object, and subsequently, a Target object.
     *
     * At build time, we generate a BriefTargetDescriptor object for every target supported by Bloom. These objects
     * can be found in the TargetService::descriptorsByConfigValue member. See the TargetService class for more.
     */
    struct BriefTargetDescriptor
    {
        /**
         * Target's market name.
         */
        std::string name;

        /**
         * Target's configuration value.
         */
        std::string configValue;

        /**
         * Target's family (AVR8/RISC-V)
         */
        TargetFamily family;

        /**
         * The file path to to the target's TDF, relative to Bloom's TDF directory.
         */
        std::string relativeTdfPath;

        constexpr BriefTargetDescriptor(
            const std::string& name,
            const std::string& configValue,
            TargetFamily family,
            const std::string& relativeTdfPath
        )
            : name(name)
            , configValue(configValue)
            , family(family)
            , relativeTdfPath(relativeTdfPath)
        {}
    };
}
