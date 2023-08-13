#pragma once

#include <string>
#include <vector>

namespace Targets::TargetDescription
{
    struct Pin
    {
        std::string pad;
        int position;
    };

    struct Pinout
    {
        std::string name;
        std::vector<Pin> pins;
    };
}
