#pragma once

#include <yaml-cpp/yaml.h>

namespace Bloom
{
    class YamlUtilities
    {
    public:
        template <typename Type>
        static bool isType(const YAML::Node& node) {
            try {
                node.as<Type>();
                return true;

            } catch (YAML::BadConversion&) {
                return false;
            }
        }
    };
}
