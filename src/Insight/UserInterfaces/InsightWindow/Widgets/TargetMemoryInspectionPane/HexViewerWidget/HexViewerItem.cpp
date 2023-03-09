#include "HexViewerItem.hpp"

#include "GraphicsItem.hpp"

namespace Bloom::Widgets
{
    HexViewerItem::HexViewerItem(Targets::TargetMemoryAddress startAddress, HexViewerItem* parent)
        : startAddress(startAddress)
        , parent(parent)
    {}

    HexViewerItem::~HexViewerItem() {
        if (this->allocatedGraphicsItem != nullptr) {
            this->allocatedGraphicsItem->setHexViewerItem(nullptr);
        }
    }

    QPoint HexViewerItem::position() const {
        if (this->parent != nullptr) {
            return this->parent->position() + this->relativePosition;
        }

        return this->relativePosition;
    }
}
