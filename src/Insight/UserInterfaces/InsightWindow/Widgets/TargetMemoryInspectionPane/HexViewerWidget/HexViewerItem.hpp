#pragma once

#include <QSize>
#include <QPainter>
#include <QGraphicsItem>

#include "src/Targets/TargetMemory.hpp"
#include "HexViewerSharedState.hpp"

namespace Widgets
{
    class GraphicsItem;

#pragma pack(push, 1)
    class HexViewerItem
    {
    public:
        static constexpr int RIGHT_MARGIN = 5;
        static constexpr int BOTTOM_MARGIN = 5;

        const Targets::TargetMemoryAddress startAddress;

        HexViewerItem* parent = nullptr;

        QPoint relativePosition = {};

        explicit HexViewerItem(Targets::TargetMemoryAddress startAddress, HexViewerItem* parent = nullptr);
        [[nodiscard]] QPoint position() const;
        [[nodiscard]] virtual QSize size() const = 0;
    };
#pragma pack(pop)
}
