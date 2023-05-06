#include "HexViewerItem.hpp"

namespace Bloom::Widgets
{
    HexViewerItem::HexViewerItem(Targets::TargetMemoryAddress startAddress, HexViewerItem* parent)
        : startAddress(startAddress)
        , parent(parent)
    {}

    QPoint HexViewerItem::position() const {
        if (this->parent != nullptr) {
            return this->parent->position() + this->relativePosition;
        }

        return this->relativePosition;
    }
}
