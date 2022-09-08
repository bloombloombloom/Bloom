#pragma once

#include <cstdint>
#include <set>

namespace Bloom
{
    enum class TaskGroup: std::uint16_t
    {};

    using TaskGroups = std::set<TaskGroup>;
}
