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
        using Exceptions::Exception;

        if (this->type != MemoryRegionType::FOCUSED) {
            throw Exception("Invalid memory region type");
        }

        if (!jsonObject.contains("dataType") || !jsonObject.contains("endianness")) {
            throw Exception("Missing data");
        }

        this->dataType = FocusedMemoryRegion::regionDataTypesByName.at(jsonObject.find("dataType")->toString());
        this->endianness = FocusedMemoryRegion::regionEndiannessByName.at(jsonObject.find("endianness")->toString());
    }

    QJsonObject FocusedMemoryRegion::toJson() const {
        auto jsonObject = MemoryRegion::toJson();
        jsonObject.insert("dataType", FocusedMemoryRegion::regionDataTypesByName.at(this->dataType));
        jsonObject.insert("endianness", FocusedMemoryRegion::regionEndiannessByName.at(this->endianness));

        return jsonObject;
    }
}
