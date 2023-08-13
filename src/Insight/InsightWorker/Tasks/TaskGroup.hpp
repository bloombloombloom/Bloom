#pragma once

#include <cstdint>
#include <set>

enum class TaskGroup: std::uint16_t
{
    USES_TARGET_CONTROLLER,
};

using TaskGroups = std::set<TaskGroup>;
