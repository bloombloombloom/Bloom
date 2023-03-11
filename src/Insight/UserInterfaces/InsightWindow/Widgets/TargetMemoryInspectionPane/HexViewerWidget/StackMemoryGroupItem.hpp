#pragma once

#include <list>

#include "GroupItem.hpp"
#include "ByteItem.hpp"
#include "FocusedRegionGroupItem.hpp"

#include "src/Targets/TargetMemory.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/FocusedMemoryRegion.hpp"

namespace Bloom::Widgets
{
    class StackMemoryGroupItem: public GroupItem
    {
    public:
        StackMemoryGroupItem(
            Targets::TargetStackPointer stackPointer,
            const HexViewerSharedState& hexViewerState,
            const std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
            std::unordered_map<Targets::TargetMemoryAddress, ByteItem>& byteItemsByAddress,
            HexViewerItem* parent
        );

        ~StackMemoryGroupItem();

        void adjustItemPositions(const int maximumWidth, const HexViewerSharedState* hexViewerState) override;

        void refreshValues();

        void paint(
            QPainter* painter,
            const HexViewerSharedState* hexViewerState,
            const QGraphicsItem* graphicsItem
        ) const override;

    protected:
        QMargins groupMargins(const HexViewerSharedState* hexViewerState, const int maximumWidth) const override;

        bool positionOnNewLine(const int maximumWidth) override {
            return true;
        }

    private:
        Targets::TargetStackPointer stackPointer;
        const HexViewerSharedState& hexViewerState;

        std::list<FocusedRegionGroupItem> focusedRegionGroupItems;
    };
}
