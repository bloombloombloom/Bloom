#pragma once

#include <QMetaType>

namespace Bloom::Targets
{
    enum class TargetState: int
    {
        UNKNOWN,
        STOPPED,
        RUNNING,
    };
}

Q_DECLARE_METATYPE(Bloom::Targets::TargetState)

