#pragma once

#include <string>
#include <QString>
#include <map>

namespace Bloom::Targets::TargetDescription
{
    struct Property
    {
        std::string name;

        /*
         * We use QString here as we're dealing with arbitrary values and QString provides many helpful
         * functions to make this easier.
         */
        QString value;
    };

    struct PropertyGroup
    {
        std::string name;
        std::map<std::string, Property> propertiesMappedByName;
    };
}
