#pragma once

#include <memory>
#include <map>
#include <string>
#include <optional>
#include <QSize>
#include <QJsonObject>

#include "src/Targets/TargetMemory.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/TargetMemoryInspectionPaneSettings.hpp"

#include "src/Helpers/BiMap.hpp"

namespace Bloom
{
    struct InsightProjectSettings
    {
    public:
        std::optional<QSize> mainWindowSize;

        std::map<
            Targets::TargetMemoryType,
            Widgets::TargetMemoryInspectionPaneSettings
        > memoryInspectionPaneSettingsByMemoryType;

        InsightProjectSettings() = default;
        explicit InsightProjectSettings(const QJsonObject& jsonObject);

        [[nodiscard]] QJsonObject toJson() const;

    private:
        static const inline BiMap<Targets::TargetMemoryType, QString> memoryTypesByName = {
            {Targets::TargetMemoryType::RAM, "ram"},
            {Targets::TargetMemoryType::EEPROM, "eeprom"},
        };

        static const inline BiMap<MemoryRegionDataType, QString> regionDataTypesByName = {
            {MemoryRegionDataType::UNKNOWN, "other"},
            {MemoryRegionDataType::UNSIGNED_INTEGER, "unsigned_int"},
            {MemoryRegionDataType::ASCII_STRING, "ascii_string"},
        };

        static const inline BiMap<MemoryRegionAddressInputType, QString> addressRangeInputTypesByName = {
            {MemoryRegionAddressInputType::ABSOLUTE, "absolute"},
            {MemoryRegionAddressInputType::RELATIVE, "relative"},
        };
    };

    struct ProjectSettings
    {
        InsightProjectSettings insightSettings;

        ProjectSettings() = default;
        explicit ProjectSettings(const QJsonObject& jsonObject);

        [[nodiscard]] QJsonObject toJson() const;
    };
}
