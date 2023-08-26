#include "HexViewerItemIndex.hpp"

#include <cassert>
#include <cmath>

namespace Widgets
{
    HexViewerItemIndex::HexViewerItemIndex(
        const TopLevelGroupItem* topLevelGroupItem,
        const QGraphicsScene* hexViewerScene
    )
        : topLevelGroupItem(topLevelGroupItem)
        , hexViewerScene(hexViewerScene)
    {
        this->refreshFlattenedItems();
    }

    HexViewerItemIndex::ItemRangeType HexViewerItemIndex::items(int yStart, int yEnd) const {
        assert(!this->byteItemGrid.empty());

        const auto gridPointCount = this->byteItemGrid.size();

        const auto startGridPointIndex = static_cast<decltype(this->byteItemGrid)::size_type>(
            std::max(
                static_cast<int>(
                    std::floor(
                        static_cast<float>(yStart) / static_cast<float>(HexViewerItemIndex::GRID_SIZE) - 1
                    )
                ),
                0
            )
        );

        if (startGridPointIndex >= gridPointCount) {
            return HexViewerItemIndex::ItemRangeType();
        }

        const auto endGridPointIndex = static_cast<decltype(this->byteItemGrid)::size_type>(std::min(
            static_cast<decltype(this->byteItemGrid)::size_type>(
                std::ceil(
                    static_cast<float>(yEnd) / static_cast<float>(HexViewerItemIndex::GRID_SIZE)
                ) + 1
            ),
            gridPointCount - 1
        ));

        return HexViewerItemIndex::ItemRangeType(
            this->byteItemGrid[startGridPointIndex],
            endGridPointIndex == gridPointCount - 1
                ? this->flattenedItems.end()
                : this->byteItemGrid[endGridPointIndex]
        );
    }

    ByteItem* HexViewerItemIndex::byteItemAt(const QPointF& position) const {
        const auto gridPointCount = this->byteItemGrid.size();

        const auto startGridPointIndex = static_cast<decltype(this->byteItemGrid)::size_type>(
            std::max(
                static_cast<int>(
                    std::floor(
                        static_cast<float>(std::max(position.y(), static_cast<qreal>(0)))
                            / static_cast<float>(HexViewerItemIndex::GRID_SIZE) - 1
                    )
                ),
                0
            )
        );

        if (startGridPointIndex >= gridPointCount) {
            return nullptr;
        }

        const auto startItemIt = this->byteItemGrid[startGridPointIndex];

        for (auto itemIt = startItemIt; itemIt < this->flattenedItems.end(); ++itemIt) {
            auto* item = *itemIt;

            const auto itemPosition = item->position();

            if (itemPosition.y() > position.y()) {
                break;
            }

            const auto itemSize = item->size();

            if (
                (itemPosition.y() + itemSize.height()) < position.y()
                || itemPosition.x() > position.x()
                || (itemPosition.x() + itemSize.width()) < position.x()
            ) {
                continue;
            }

            auto* byteItem = dynamic_cast<ByteItem*>(item);
            if (byteItem != nullptr) {
                return byteItem;
            }
        }

        return nullptr;
    }

    ByteItem* HexViewerItemIndex::closestByteItem(int yPosition) const {
        assert(!this->byteItemGrid.empty());

        const auto gridPointCount = this->byteItemGrid.size();

        const auto gridPointIndex = static_cast<decltype(this->byteItemGrid)::size_type>(
            std::round(
                static_cast<float>(yPosition) / static_cast<float>(HexViewerItemIndex::GRID_SIZE)
            )
        );

        // Sanity check
        assert(gridPointCount > gridPointIndex);

        return static_cast<ByteItem*>(*(this->byteItemGrid[gridPointIndex]));
    }

    std::vector<ByteItem*> HexViewerItemIndex::intersectingByteItems(const QRectF& rect) const {
        auto output = std::vector<ByteItem*>();

        const auto yStart = static_cast<int>(std::max(rect.y(), static_cast<qreal>(0)));
        const auto yEnd = static_cast<int>(std::max(rect.y() + rect.height(), static_cast<qreal>(0)));

        const auto items = this->items(yStart, yEnd);

        for (auto& item : items) {
            const auto itemRect = QRectF(item->position(), item->size());

            if (itemRect.y() > yEnd) {
                break;
            }

            if (!itemRect.intersects(rect)) {
                continue;
            }

            auto* byteItem = dynamic_cast<ByteItem*>(item);
            if (byteItem != nullptr) {
                output.push_back(byteItem);
            }
        }

        return output;
    }

    const ByteItem* HexViewerItemIndex::leftMostByteItemWithinRange(int yStart, int yEnd) const {
        const ByteItem* leftMostByteItem = nullptr;
        auto leftMostByteItemXPos = 0;

        for (const auto* item : this->items(yStart, yEnd)) {
            const auto itemPos = item->position();

            /*
             * Remember, the HexViewerItemIndex::items() function can return items that are outside the given range,
             * depending on the grid size of the Y-axis index.
             *
             * We need to ensure that we exclude those items here.
             */
            if (itemPos.y() < yStart) {
                continue;
            }

            if (itemPos.y() > yEnd) {
                break;
            }

            const auto* byteItem = dynamic_cast<const ByteItem*>(item);

            if (byteItem == nullptr) {
                continue;
            }

            if (leftMostByteItem == nullptr || itemPos.x() < leftMostByteItemXPos) {
                leftMostByteItem = byteItem;
                leftMostByteItemXPos = itemPos.x();
            }
        }

        return leftMostByteItem;
    }

    const ByteItem* HexViewerItemIndex::rightMostByteItemWithinRange(int yStart, int yEnd) const {
        const ByteItem* rightMostByteItem = nullptr;
        auto rightMostByteItemXPos = 0;

        for (const auto* item : this->items(yStart, yEnd)) {
            const auto itemPos = item->position();

            if (itemPos.y() < yStart) {
                continue;
            }

            if (itemPos.y() > yEnd) {
                break;
            }

            const auto* byteItem = dynamic_cast<const ByteItem*>(item);

            if (byteItem == nullptr) {
                continue;
            }

            const auto itemXPos = item->position().x() + item->size().width();
            if (rightMostByteItem == nullptr || itemXPos > rightMostByteItemXPos) {
                rightMostByteItem = byteItem;
                rightMostByteItemXPos = itemXPos;
            }
        }

        return rightMostByteItem;
    }

    void HexViewerItemIndex::refreshFlattenedItems() {
        this->flattenedItems = this->topLevelGroupItem->flattenedItems();
    }

    void HexViewerItemIndex::refreshIndex() {
        const auto pointsRequired = static_cast<std::uint32_t>(
            this->hexViewerScene->sceneRect().height() / HexViewerItemIndex::GRID_SIZE
        );

        this->byteItemGrid.clear();
        this->byteItemGrid.reserve(pointsRequired);

        this->byteItemLines.clear();
        this->byteItemYStartPositionsByAddress.clear();

        auto currentByteItemGridPoint = 0;
        auto currentLineYPosition = 0;

        for (auto itemIt = this->flattenedItems.begin(); itemIt != this->flattenedItems.end(); ++itemIt) {
            auto& item = *itemIt;

            const auto byteItem = dynamic_cast<const ByteItem*>(item);

            if (byteItem == nullptr) {
                continue;
            }

            const auto itemYStartPosition = byteItem->position().y();
            const auto itemYEndPosition = itemYStartPosition + byteItem->size().height();

            this->byteItemYStartPositionsByAddress.emplace(byteItem->startAddress, itemYStartPosition);

            if (itemYStartPosition > currentLineYPosition) {
                this->byteItemLines.push_back(byteItem);
                currentLineYPosition = itemYStartPosition;
            }

            if (itemYEndPosition >= currentByteItemGridPoint) {
                // This byte item is the first to exceed or intersect with the currentByteItemGridPoint
                this->byteItemGrid.push_back(itemIt);
                currentByteItemGridPoint += HexViewerItemIndex::GRID_SIZE;
            }
        }
    }
}
