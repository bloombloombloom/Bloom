#pragma once

#include <QMetaType>

namespace Targets
{
    enum class TargetState: std::uint8_t
    {
        UNKNOWN,
        STOPPED,
        RUNNING,
    };
}

Q_DECLARE_METATYPE(Targets::TargetState)
