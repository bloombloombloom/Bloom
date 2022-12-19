#include "MemoryRegion.hpp"

#include "src/Helpers/EnumToStringMappings.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Bloom
{
    MemoryRegion::MemoryRegion(
        const QString& name,
        Targets::TargetMemoryType memoryType,
        MemoryRegionType type,
        const Targets::TargetMemoryAddressRange& addressRange
    )
        : name(name)
        , memoryType(memoryType)
        , type(type)
        , addressRange(addressRange)
    {}

    MemoryRegion::MemoryRegion(const QJsonObject& jsonObject) {
        using Exceptions::Exception;

        if (
            !jsonObject.contains("name")
            || !jsonObject.contains("memoryType")
            || !jsonObject.contains("type")
            || !jsonObject.contains("createdTimestamp")
            || !jsonObject.contains("addressInputType")
            || !jsonObject.contains("addressRange")
        ) {
            throw Exception("Missing data");
        }

        const auto addressRangeObj = jsonObject.find("addressRange")->toObject();
        if (
            !addressRangeObj.contains("startAddress")
            || !addressRangeObj.contains("endAddress")
        ) {
            throw Exception("Missing address range data");
        }

        this->name = jsonObject.find("name")->toString();
        this->memoryType = EnumToStringMappings::targetMemoryTypes.at(jsonObject.find("memoryType")->toString());
        this->type = MemoryRegion::memoryRegionTypesByName.at(jsonObject.find("type")->toString());
        this->createdDate.setSecsSinceEpoch(jsonObject.find("createdTimestamp")->toInteger());
        this->addressRangeInputType = MemoryRegion::addressTypesByName.at(jsonObject.find("addressInputType")->toString());

        this->addressRange = {
            static_cast<std::uint32_t>(addressRangeObj.find("startAddress")->toInteger()),
            static_cast<std::uint32_t>(addressRangeObj.find("endAddress")->toInteger()),
        };
    }

    QJsonObject MemoryRegion::toJson() const {
        return QJsonObject({
            {"name", this->name},
            {"memoryType", EnumToStringMappings::targetMemoryTypes.at(this->memoryType)},
            {"type", MemoryRegion::memoryRegionTypesByName.at(this->type)},
            {"createdTimestamp", this->createdDate.toSecsSinceEpoch()},
            {"addressInputType", MemoryRegion::addressTypesByName.at(this->addressRangeInputType)},
            {"addressRange", QJsonObject({
                {"startAddress", static_cast<qint64>(this->addressRange.startAddress)},
                {"endAddress", static_cast<qint64>(this->addressRange.endAddress)},
            })},
        });
    }
}
