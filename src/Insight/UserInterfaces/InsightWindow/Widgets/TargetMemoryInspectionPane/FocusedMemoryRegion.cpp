#include "FocusedMemoryRegion.hpp"

#include "src/Exceptions/Exception.hpp"

namespace Bloom
{
    FocusedMemoryRegion::FocusedMemoryRegion(
        const QString& name,
        Targets::TargetMemoryType memoryType,
        const Targets::TargetMemoryAddressRange& addressRange
    )
        : MemoryRegion(name, memoryType, MemoryRegionType::FOCUSED, addressRange)
    {}

    FocusedMemoryRegion::FocusedMemoryRegion(const QJsonObject& jsonObject)
        : MemoryRegion(jsonObject)
    {
        if (this->type != MemoryRegionType::FOCUSED) {
            throw Exceptions::Exception("Invalid memory region type");
        }
    }
}
