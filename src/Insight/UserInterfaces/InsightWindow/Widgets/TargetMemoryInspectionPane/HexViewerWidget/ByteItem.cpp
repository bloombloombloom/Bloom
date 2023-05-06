#include "ByteItem.hpp"

namespace Bloom::Widgets
{
    ByteItem::ByteItem(Targets::TargetMemoryAddress address)
        : HexViewerItem(address)
    {}
}
