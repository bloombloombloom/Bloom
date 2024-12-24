#include "MemorySnapshot.hpp"

#include <QUuid>
#include <QByteArray>
#include <QJsonArray>

#include "src/Exceptions/Exception.hpp"

MemorySnapshot::MemorySnapshot(
    const QString& name,
    const QString& description,
    const QString& addressSpaceKey,
    const QString& memorySegmentKey,
    const Targets::TargetMemoryBuffer& data,
    Targets::TargetMemoryAddress programCounter,
    Targets::TargetStackPointer stackPointer,
    const std::vector<FocusedMemoryRegion>& focusedRegions,
    const std::vector<ExcludedMemoryRegion>& excludedRegions
)
    : id(QUuid::createUuid().toString(QUuid::StringFormat::WithoutBraces))
    , name(name)
    , description(description)
    , addressSpaceKey(addressSpaceKey)
    , memorySegmentKey(memorySegmentKey)
    , data(data)
    , programCounter(programCounter)
    , stackPointer(stackPointer)
    , focusedRegions(focusedRegions)
    , excludedRegions(excludedRegions)
{}

MemorySnapshot::MemorySnapshot(const QJsonObject& jsonObject, const Targets::TargetDescriptor& targetDescriptor) {
    using Exceptions::Exception;

    if (
        !jsonObject.contains("id")
        || !jsonObject.contains("name")
        || !jsonObject.contains("description")
        || (
            !jsonObject.contains("memoryType")
            && (!jsonObject.contains("addressSpaceKey") || !jsonObject.contains("memorySegmentKey"))
        )
        || !jsonObject.contains("hexData")
        || !jsonObject.contains("programCounter")
        || !jsonObject.contains("stackPointer")
        || !jsonObject.contains("createdTimestamp")
        || !jsonObject.contains("focusedRegions")
        || !jsonObject.contains("excludedRegions")
    ) {
        throw Exception{"Missing data"};
    }

    this->id = jsonObject.find("id")->toString();
    this->name = jsonObject.find("name")->toString();
    this->description = jsonObject.find("description")->toString();

    if (!jsonObject.contains("addressSpaceKey") || !jsonObject.contains("memorySegmentKey")) {
        /*
         * Temporary fallback for backwards compatability with snapshot files that were created by a version prior
         * to v2.0.0. Those files will contain a 'memoryType' field, which we map to AVR address spaces and memory
         * segments here.
         *
         * Bloom only supported AVR targets before v2.0.0, which is why we only need to be concerned with AVR address
         * spaces and memory segments, here.
         *
         * @TODO: Bin this after a few versions from v2.0.0.
         */

        const auto memoryType = jsonObject.find("memoryType")->toString();

        if (memoryType == "ram") {
            this->addressSpaceKey = "data";
            this->memorySegmentKey = "internal_ram";

        } else if (memoryType == "eeprom") {
            // Some AVR targets have a separate address space for EEPROM, others use the `data` address space.
            this->addressSpaceKey = targetDescriptor.tryGetAddressSpaceDescriptor("eeprom").has_value()
                ? "eeprom"
                : "data";
            this->memorySegmentKey = "internal_eeprom";

        } else if (memoryType == "flash") {
            this->addressSpaceKey = "prog";
            this->memorySegmentKey = "internal_program_memory";
        }

    } else {
        this->addressSpaceKey = jsonObject.find("addressSpaceKey")->toString();
        this->memorySegmentKey = jsonObject.find("memorySegmentKey")->toString();
    }

    this->programCounter = static_cast<Targets::TargetMemoryAddress>(jsonObject.find("programCounter")->toInteger());
    this->stackPointer = static_cast<Targets::TargetStackPointer>(jsonObject.find("stackPointer")->toInteger());
    this->createdDate.setSecsSinceEpoch(jsonObject.find("createdTimestamp")->toInteger());

    const auto hexData = QByteArray::fromHex(jsonObject.find("hexData")->toString().toUtf8());
    this->data = Targets::TargetMemoryBuffer{hexData.begin(), hexData.end()};

    if (jsonObject.contains("focusedRegions")) {
        for (const auto& regionValue : jsonObject.find("focusedRegions")->toArray()) {
            try {
                this->focusedRegions.emplace_back(regionValue.toObject());

            } catch (const Exception& exception) {
                throw Exception{"Invalid focused memory region"};
            }
        }
    }

    if (jsonObject.contains("excludedRegions")) {
        for (const auto& regionValue : jsonObject.find("excludedRegions")->toArray()) {
            try {
                this->excludedRegions.emplace_back(regionValue.toObject());

            } catch (const Exception& exception) {
                throw Exception{"Invalid excluded memory region"};
            }
        }
    }
}

QJsonObject MemorySnapshot::toJson() const {
    auto focusedRegions = QJsonArray{};
    for (const auto& focusedRegion : this->focusedRegions) {
        focusedRegions.push_back(focusedRegion.toJson());
    }

    auto excludedRegions = QJsonArray{};
    for (const auto& excludedRegion : this->excludedRegions) {
        excludedRegions.push_back(excludedRegion.toJson());
    }

    return QJsonObject{
        {"id", this->id},
        {"name", this->name},
        {"description", this->description},
        {"addressSpaceKey", this->addressSpaceKey},
        {"memorySegmentKey", this->memorySegmentKey},
        {"hexData", QString{
            QByteArray{
                reinterpret_cast<const char*>(this->data.data()),
                static_cast<qsizetype>(this->data.size())
            }.toHex()
        }},
        {"programCounter", static_cast<qint64>(this->programCounter)},
        {"stackPointer", static_cast<qint64>(this->stackPointer)},
        {"createdTimestamp", this->createdDate.toSecsSinceEpoch()},
        {"focusedRegions", focusedRegions},
        {"excludedRegions", excludedRegions},
    };
}

bool MemorySnapshot::isCompatible(const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor) const {
    if (this->data.size() != memorySegmentDescriptor.size()) {
        return false;
    }

    for (const auto& focusedRegion : this->focusedRegions) {
        if (!memorySegmentDescriptor.addressRange.contains(focusedRegion.addressRange)) {
            return false;
        }
    }

    for (const auto& excludedRegion : this->excludedRegions) {
        if (!memorySegmentDescriptor.addressRange.contains(excludedRegion.addressRange)) {
            return false;
        }
    }

    return true;
}

std::set<Targets::TargetMemoryAddress> MemorySnapshot::excludedAddresses() const {
    auto output = std::set<Targets::TargetMemoryAddress>{};

    for (const auto& excludedRegion : this->excludedRegions) {
        const auto regionAddresses = excludedRegion.addressRange.addresses();
        output.insert(regionAddresses.begin(), regionAddresses.end());
    }

    return output;
}
