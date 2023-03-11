#pragma once

#include <optional>
#include <QString>

#include "GroupItem.hpp"
#include "ByteItem.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/FocusedMemoryRegion.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace Bloom::Widgets
{
    class FocusedRegionGroupItem: public GroupItem
    {
    public:
        FocusedRegionGroupItem(
            const FocusedMemoryRegion& focusedRegion,
            std::unordered_map<Targets::TargetMemoryAddress, ByteItem>& byteItemsByAddress,
            HexViewerItem* parent
        );

        ~FocusedRegionGroupItem();

        void refreshValue(const HexViewerSharedState& hexViewerState);

        void paint(
            QPainter* painter,
            const HexViewerSharedState* hexViewerState,
            const QGraphicsItem* graphicsItem
        ) const override;

    protected:
        QMargins groupMargins(const HexViewerSharedState* hexViewerState, const int maximumWidth) const override;

    private:
        static constexpr int ANNOTATION_HEIGHT = 35;

        const FocusedMemoryRegion& focusedMemoryRegion;
        std::optional<QString> valueLabel;

        __attribute__((always_inline)) inline void paintRegionNameAnnotation(
            QPainter* painter,
            const HexViewerSharedState* hexViewerState,
            const QGraphicsItem* graphicsItem
        ) const;

        __attribute__((always_inline)) inline void paintValueAnnotation(
            QPainter* painter,
            const HexViewerSharedState* hexViewerState,
            const QGraphicsItem* graphicsItem
        ) const;
    };
}
