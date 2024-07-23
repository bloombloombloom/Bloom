#include "TargetMemorySegmentDescriptor.hpp"

#include "src/Services/StringService.hpp"

namespace Targets
{
    TargetMemorySegmentDescriptor::TargetMemorySegmentDescriptor(
        const std::string& addressSpaceKey,
        const std::string& key,
        const std::string& name,
        TargetMemorySegmentType type,
        const TargetMemoryAddressRange& addressRange,
        std::uint8_t addressSpaceUnitSize,
        bool executable,
        const TargetMemoryAccess& debugModeAccess,
        const TargetMemoryAccess& programmingModeAccess,
        std::optional<TargetMemorySize> pageSize
    )
        : id(static_cast<TargetMemorySegmentId>(Services::StringService::hash(addressSpaceKey + key)))
        , key(key)
        , name(name)
        , type(type)
        , addressRange(addressRange)
        , addressSpaceUnitSize(addressSpaceUnitSize)
        , executable(executable)
        , debugModeAccess(debugModeAccess)
        , programmingModeAccess(programmingModeAccess)
        , pageSize(pageSize)
    {}

    bool TargetMemorySegmentDescriptor::operator == (const TargetMemorySegmentDescriptor& other) const {
        return this->id == other.id;
    }

    bool TargetMemorySegmentDescriptor::operator != (const TargetMemorySegmentDescriptor& other) const {
        return !(*this == other);
    }

    TargetMemorySize TargetMemorySegmentDescriptor::size() const {
        return this->addressRange.size() * this->addressSpaceUnitSize;
    }

    TargetMemorySegmentDescriptor TargetMemorySegmentDescriptor::clone() const {
        return {*this};
    }
}
