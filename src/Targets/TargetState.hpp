#pragma once

#include <QMetaType>

namespace Bloom::Targets
{
    enum class TargetState: std::uint8_t
    {
        UNKNOWN,
        STOPPED,
        RUNNING,
    };
}

Q_DECLARE_METATYPE(Bloom::Targets::TargetState)
