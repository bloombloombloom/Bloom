#pragma once

#include <QSize>
#include <QPainter>
#include <QGraphicsItem>

#include "src/Targets/TargetMemory.hpp"
#include "HexViewerSharedState.hpp"

namespace Bloom::Widgets
{
    class GraphicsItem;

#pragma pack(push, 1)
    class HexViewerItem
    {
    public:
        static constexpr int RIGHT_MARGIN = 5;
        static constexpr int BOTTOM_MARGIN = 5;

        const Targets::TargetMemoryAddress startAddress = 0;

        QPoint relativePosition = {};

        HexViewerItem* parent = nullptr;
        GraphicsItem* allocatedGraphicsItem = nullptr;

        HexViewerItem(Targets::TargetMemoryAddress startAddress, HexViewerItem* parent = nullptr);

        virtual ~HexViewerItem();

        QPoint position() const;

        virtual QSize size() const = 0;

        virtual void paint(
            QPainter* painter,
            const HexViewerSharedState* hexViewerState,
            const QGraphicsItem* graphicsItem
        ) const = 0;
    };
#pragma pack(pop)
}
