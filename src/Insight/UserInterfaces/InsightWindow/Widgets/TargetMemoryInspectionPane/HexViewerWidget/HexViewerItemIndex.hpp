#pragma once

#include <vector>
#include <ranges>
#include <QGraphicsScene>
#include <QPointF>
#include <unordered_map>

#include "HexViewerItem.hpp"
#include "TopLevelGroupItem.hpp"
#include "ByteItem.hpp"

namespace Widgets
{
    /**
     * This class maintains an index of hex viewer item positions and provides fast lookups for items within certain
     * positions.
     */
    class HexViewerItemIndex
    {
    public:
        using FlattenedItems = std::vector<HexViewerItem*>;
        using FlattenedItemIt = FlattenedItems::const_iterator;
        using ItemRange = std::ranges::subrange<FlattenedItemIt>;

        std::vector<const ByteItem*> byteItemLines;
        std::unordered_map<Targets::TargetMemoryAddress, int> byteItemYStartPositionsByAddress;

        explicit HexViewerItemIndex(
            const TopLevelGroupItem* topLevelGroupItem,
            const QGraphicsScene* hexViewerScene
        );

        /**
         * Identifies the items between two points on the Y axis, and returns them in the form of a subrange, in
         * constant-time.
         *
         * This member function can return items that are outside of the range, by HexViewerItemIndex::GRID_SIZE.
         * The caller should tolerate this.
         *
         * CAUTION: The returned range can be invalidated! This member function should only be used immediately before
         * you intend to do work on the returned range. Do **NOT** keep hold of the returned range. You should consider
         * any data returned by this function to be invalid as soon as program control has returned to the main event
         * loop.
         *
         * @param yStart
         * @param yEnd
         *
         * @return
         */
        ItemRange items(int yStart, int yEnd) const;

        /**
         * Returns the byte item at the given position. Byte items do not overlap.
         *
         * @param position
         * @return
         *  nullptr if there is no byte item at the given position.
         */
        ByteItem* byteItemAt(const QPointF& position) const;

        /**
         * Returns the closest byte item to the given position on the Y-axis.
         *
         * @param yPosition
         * @return
         */
        ByteItem* closestByteItem(int yPosition) const;

        /**
         * Returns all byte items that intersect with the given rectangle.
         *
         * @param rect
         * @return
         */
        std::vector<ByteItem*> intersectingByteItems(const QRectF& rect) const;

        /**
         * Returns the left-most (smallest position on the X-axis) byte item, within a Y-axis range.
         *
         * @param yStart
         * @param yEnd
         * @return
         */
        const ByteItem* leftMostByteItemWithinRange(int yStart, int yEnd) const;

        /**
         * Returns the right-most (largest position on the X-axis) byte item, within a Y-axis range.
         *
         * @param yStart
         * @param yEnd
         * @return
         */
        const ByteItem* rightMostByteItemWithinRange(int yStart, int yEnd) const;

        void refreshFlattenedItems();
        void refreshIndex();

    protected:
        static constexpr auto GRID_SIZE = 100;

        const TopLevelGroupItem* topLevelGroupItem;
        const QGraphicsScene* hexViewerScene;

        /**
         * An std::vector of all HexViewerItems along with their parents and children, sorted by position.
         *
         * Some of the lookup member functions return subranges from this container.
         */
        FlattenedItems flattenedItems;

        /**
         * Byte item Y-axis grid (one-dimensional index)
         *
         * Each element in this std::vector represents a point on the Y-axis grid. The distance between each point is
         * equal to HexViewerItemIndex::GRID_SIZE (pixels).
         *
         * The value of each element is an iterator addressing the HexViewerItem* at that point on the grid.
         *
         * Although the iterators address hex viewer items, all elements can be safely cast to ByteItem*, as we only
         * consider byte items when populating this grid. See HexViewerItemIndex::refreshIndex() for more.
         *
         * We use an std::vector here because it provides constant-time access to any element.
         */
        std::vector<FlattenedItemIt> byteItemGrid;
    };
}
