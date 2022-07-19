#pragma once

#include <QSize>
#include <QPoint>
#include <optional>

namespace Bloom::Widgets
{
    struct DetachedWindowState
    {
        QSize size;
        QPoint position;

        DetachedWindowState() = default;

        DetachedWindowState(QSize size, QPoint position)
            : size(size)
            , position(position)
        {}
    };

    struct PaneState
    {
        bool activated = false;
        bool attached = true;

        std::optional<DetachedWindowState> detachedWindowState;

        PaneState(
            bool activated,
            bool attached,
            std::optional<DetachedWindowState> detachedWindowState
        )
            : activated(activated)
            , attached(attached)
            , detachedWindowState(detachedWindowState)
        {};
    };
}
