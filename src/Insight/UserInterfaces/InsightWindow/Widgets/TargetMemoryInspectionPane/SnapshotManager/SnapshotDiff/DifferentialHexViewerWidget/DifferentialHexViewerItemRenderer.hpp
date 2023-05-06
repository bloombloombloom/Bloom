#pragma once

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/HexViewerItemRenderer.hpp"
#include "DifferentialHexViewerWidgetType.hpp"

namespace Bloom::Widgets
{
    class DifferentialHexViewerItemRenderer: public HexViewerItemRenderer
    {
    public:
        DifferentialHexViewerItemRenderer(
            DifferentialHexViewerWidgetType differentialHexViewerWidgetType,
            const HexViewerSharedState& hexViewerState,
            const HexViewerItemIndex& itemIndex,
            const QGraphicsView* view
        );

        void setOther(const DifferentialHexViewerItemRenderer* other);

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    protected:
        DifferentialHexViewerWidgetType differentialHexViewerWidgetType;
        const DifferentialHexViewerItemRenderer* other = nullptr;

        inline void paintChangedLinePolygon(
            const ByteItem* firstByteItem,
            const ByteItem* lastByteItem,
            int viewportHeight,
            int vScrollBarValue,
            int otherVScrollBarValue,
            QPainter* painter
        ) __attribute__((__always_inline__));
    };
}
