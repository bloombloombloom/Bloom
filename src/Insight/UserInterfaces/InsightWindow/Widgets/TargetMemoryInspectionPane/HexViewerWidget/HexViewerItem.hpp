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

        const Targets::TargetMemoryAddress startAddress = 0;

        HexViewerItem* parent = nullptr;

        QPoint relativePosition = {};

        HexViewerItem(Targets::TargetMemoryAddress startAddress, HexViewerItem* parent = nullptr);
        QPoint position() const;
        virtual QSize size() const = 0;
    };
#pragma pack(pop)
}
