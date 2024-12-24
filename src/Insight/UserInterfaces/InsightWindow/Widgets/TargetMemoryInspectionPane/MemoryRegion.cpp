#include "MemoryRegion.hpp"

#include "src/Helpers/EnumToStringMappings.hpp"
#include "src/Exceptions/Exception.hpp"

MemoryRegion::MemoryRegion(
    const QString& name,
    MemoryRegionType type,
    const Targets::TargetMemoryAddressRange& addressRange
)
    : name(name)
    , type(type)
    , addressRange(addressRange)
{}

MemoryRegion::MemoryRegion(const QJsonObject& jsonObject) {
    using Exceptions::Exception;

    if (
        !jsonObject.contains("name")
        || !jsonObject.contains("type")
        || !jsonObject.contains("createdTimestamp")
        || !jsonObject.contains("addressInputType")
        || !jsonObject.contains("addressRange")
    ) {
        throw Exception{"Missing data"};
    }

    const auto addressRangeObj = jsonObject.find("addressRange")->toObject();
    if (!addressRangeObj.contains("startAddress") || !addressRangeObj.contains("endAddress")) {
        throw Exception{"Missing address range data"};
    }

    this->name = jsonObject.find("name")->toString();
    this->type = MemoryRegion::memoryRegionTypesByName.at(jsonObject.find("type")->toString());
    this->createdDate.setSecsSinceEpoch(jsonObject.find("createdTimestamp")->toInteger());
    this->addressRangeInputType = MemoryRegion::addressTypesByName.at(jsonObject.find("addressInputType")->toString());
    this->addressRange = {
        static_cast<Targets::TargetMemoryAddress>(addressRangeObj.find("startAddress")->toInteger()),
        static_cast<Targets::TargetMemoryAddress>(addressRangeObj.find("endAddress")->toInteger()),
    };
}

QJsonObject MemoryRegion::toJson() const {
    return {
        {"name", this->name},
        {"type", MemoryRegion::memoryRegionTypesByName.at(this->type)},
        {"createdTimestamp", this->createdDate.toSecsSinceEpoch()},
        {"addressInputType", MemoryRegion::addressTypesByName.at(this->addressRangeInputType)},
        {"addressRange", QJsonObject{
            {"startAddress", static_cast<qint64>(this->addressRange.startAddress)},
            {"endAddress", static_cast<qint64>(this->addressRange.endAddress)},
        }},
    };
}
