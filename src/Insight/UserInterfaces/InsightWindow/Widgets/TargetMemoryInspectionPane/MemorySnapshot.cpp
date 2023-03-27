#include "MemorySnapshot.hpp"

#include <QByteArray>
#include <QJsonArray>

#include "src/Helpers/EnumToStringMappings.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Bloom
{
    MemorySnapshot::MemorySnapshot(
        const QString& name,
        const QString& description,
        Targets::TargetMemoryType memoryType,
        const Targets::TargetMemoryBuffer& data,
        Targets::TargetProgramCounter programCounter,
        const std::vector<FocusedMemoryRegion>& focusedRegions,
        const std::vector<ExcludedMemoryRegion>& excludedRegions
    )
        : name(name)
        , description(description)
        , memoryType(memoryType)
        , data(data)
        , programCounter(programCounter)
        , focusedRegions(focusedRegions)
        , excludedRegions(excludedRegions)
    {}

    MemorySnapshot::MemorySnapshot(const QJsonObject& jsonObject) {
        using Exceptions::Exception;

        if (
            !jsonObject.contains("id")
            || !jsonObject.contains("name")
            || !jsonObject.contains("description")
            || !jsonObject.contains("memoryType")
            || !jsonObject.contains("hexData")
            || !jsonObject.contains("programCounter")
            || !jsonObject.contains("createdTimestamp")
            || !jsonObject.contains("focusedRegions")
            || !jsonObject.contains("excludedRegions")
        ) {
            throw Exception("Missing data");
        }

        this->id = jsonObject.find("id")->toString();
        this->name = jsonObject.find("name")->toString();
        this->description = jsonObject.find("description")->toString();
        this->memoryType = EnumToStringMappings::targetMemoryTypes.at(jsonObject.find("memoryType")->toString());
        this->programCounter = static_cast<Targets::TargetProgramCounter>(jsonObject.find("programCounter")->toInteger());
        this->createdDate.setSecsSinceEpoch(jsonObject.find("createdTimestamp")->toInteger());

        const auto hexData = QByteArray::fromHex(jsonObject.find("hexData")->toString().toUtf8());
        this->data = Targets::TargetMemoryBuffer(hexData.begin(), hexData.end());

        if (jsonObject.contains("focusedRegions")) {
            for (const auto& regionValue : jsonObject.find("focusedRegions")->toArray()) {
                try {
                    this->focusedRegions.emplace_back(regionValue.toObject());

                } catch (Exception exception) {
                    throw Exception("Invalid focused memory region");
                }
            }
        }

        if (jsonObject.contains("excludedRegions")) {
            for (const auto& regionValue : jsonObject.find("excludedRegions")->toArray()) {
                try {
                    this->excludedRegions.emplace_back(regionValue.toObject());

                } catch (Exception exception) {
                    throw Exception("Invalid excluded memory region");
                }
            }
        }
    }

    QJsonObject MemorySnapshot::toJson() const {
        auto focusedRegions = QJsonArray();
        for (const auto& focusedRegion : this->focusedRegions) {
            focusedRegions.push_back(focusedRegion.toJson());
        }

        auto excludedRegions = QJsonArray();
        for (const auto& excludedRegion : this->excludedRegions) {
            excludedRegions.push_back(excludedRegion.toJson());
        }

        return QJsonObject({
            {"id", this->id},
            {"name", this->name},
            {"description", this->description},
            {"memoryType", EnumToStringMappings::targetMemoryTypes.at(this->memoryType)},
            {"hexData", QString(QByteArray(
                reinterpret_cast<const char*>(this->data.data()),
                static_cast<qsizetype>(this->data.size())
            ).toHex())},
            {"programCounter", static_cast<qint64>(this->programCounter)},
            {"createdTimestamp", this->createdDate.toSecsSinceEpoch()},
            {"focusedRegions", focusedRegions},
            {"excludedRegions", excludedRegions},
        });
    }

    bool MemorySnapshot::isCompatible(const Targets::TargetMemoryDescriptor& memoryDescriptor) const {
        if (this->memoryType != memoryDescriptor.type) {
            return false;
        }

        if (this->data.size() != memoryDescriptor.size()) {
            return false;
        }

        const auto& memoryAddressRange = memoryDescriptor.addressRange;
        for (const auto& focusedRegion : this->focusedRegions) {
            if (
                focusedRegion.addressRange.startAddress < memoryAddressRange.startAddress
                || focusedRegion.addressRange.endAddress > memoryAddressRange.endAddress
            ) {
                return false;
            }
        }

        for (const auto& excludedRegion : this->excludedRegions) {
            if (
                excludedRegion.addressRange.startAddress < memoryAddressRange.startAddress
                || excludedRegion.addressRange.endAddress > memoryAddressRange.endAddress
            ) {
                return false;
            }
        }

        return true;
    }
}
