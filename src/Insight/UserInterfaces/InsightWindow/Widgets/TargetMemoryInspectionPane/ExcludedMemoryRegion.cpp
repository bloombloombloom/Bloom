#include "ExcludedMemoryRegion.hpp"

#include "src/Exceptions/Exception.hpp"

ExcludedMemoryRegion::ExcludedMemoryRegion(
    const QString& name,
    const Targets::TargetMemoryAddressRange& addressRange
)
    : MemoryRegion(
        name,
        MemoryRegionType::EXCLUDED,
        addressRange
    )
{}

ExcludedMemoryRegion::ExcludedMemoryRegion(const QJsonObject& jsonObject)
    : MemoryRegion(jsonObject)
{
    if (this->type != MemoryRegionType::EXCLUDED) {
        throw Exceptions::Exception{"Invalid memory region type"};
    }
}
