#include "GroupItem.hpp"

namespace Widgets
{
    GroupItem::~GroupItem() {
        for (auto& byteItem : this->items) {
            byteItem->parent = nullptr;
        }
    }

    void GroupItem::adjustItemPositions(const int maximumWidth, const HexViewerSharedState* hexViewerState) {
        const auto margins = this->groupMargins(hexViewerState, maximumWidth);

        int width = margins.left();
        int height = margins.top();

        this->multiLine = false;
        auto position = QPoint(margins.left(), margins.top());

        auto currentLineItems = std::vector<HexViewerItem*>();
        const ByteItem* lastByteItem = nullptr;

        for (const auto& item : this->items) {
            auto* const groupItem = dynamic_cast<GroupItem*>(item);
            if (groupItem != nullptr) {
                groupItem->adjustItemPositions(maximumWidth, hexViewerState);
            }

            const ByteItem* byteItem = groupItem != nullptr
                ? groupItem->firstByteItem()
                : dynamic_cast<ByteItem*>(item);

            const auto itemSize = item->size();
            const auto availableWidth = maximumWidth - margins.right() - position.x();

            if (
                (groupItem != nullptr && groupItem->positionOnNewLine(maximumWidth))
                || itemSize.width() > availableWidth
            ) {
                position.setX(margins.left());
                position.setY(height + HexViewerItem::BOTTOM_MARGIN);
                this->multiLine = true;
                currentLineItems.clear();
            }

            item->relativePosition = position;

            if (byteItem != nullptr && lastByteItem != nullptr && !currentLineItems.empty()) {
                // Align byte items on each row
                const auto offset = lastByteItem->position().y() - byteItem->position().y();

                if (offset > 0) {
                    item->relativePosition.setY(item->relativePosition.y() + offset);
                }

                if (offset < 0) {
                    for (auto& item : currentLineItems) {
                        item->relativePosition.setY(item->relativePosition.y() + std::abs(offset));
                    }
                }
            }

            height = std::max(static_cast<int>(position.y() + itemSize.height()), height);
            width = std::max(static_cast<int>(position.x() + itemSize.width()), width);

            position.setX(static_cast<int>(position.x() + itemSize.width() + HexViewerItem::RIGHT_MARGIN));

            currentLineItems.push_back(item);
            lastByteItem = byteItem;
        }

        this->groupSize = QSize(width + margins.right(), height + margins.bottom());
    }

    std::vector<HexViewerItem*> GroupItem::flattenedItems() const {
        auto flattenedItems = std::vector<HexViewerItem*>();

        for (const auto& item : this->items) {
            flattenedItems.push_back(item);

            const auto* groupItem = dynamic_cast<const GroupItem*>(item);
            if (groupItem != nullptr) {
                auto groupFlattenedItems = groupItem->flattenedItems();
                std::move(groupFlattenedItems.begin(), groupFlattenedItems.end(), std::back_inserter(flattenedItems));
            }
        }

        return flattenedItems;
    }

    GroupItem::GroupItem(
        Targets::TargetMemoryAddress startAddress,
        HexViewerItem* parent
    )
        : HexViewerItem(startAddress, parent)
    {}

    ByteItem* GroupItem::firstByteItem() const {
        for (const auto& item : this->items) {
            auto* byteItem = dynamic_cast<ByteItem*>(item);
            if (byteItem != nullptr) {
                return byteItem;
            }
        }

        return nullptr;
    }

    void GroupItem::sortItems() {
        std::sort(
            this->items.begin(),
            this->items.end(),
            [] (const HexViewerItem* itemA, const HexViewerItem* itemB) {
                return itemA->startAddress < itemB->startAddress;
            }
        );
    }
}
