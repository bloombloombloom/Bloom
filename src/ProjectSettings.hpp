#pragma once

#include <memory>
#include <map>
#include <string>
#include <optional>
#include <QSize>
#include <QJsonObject>

#include "src/Targets/TargetMemory.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PanelState.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PaneState.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/TargetMemoryInspectionPaneSettings.hpp"

#include "src/Helpers/BiMap.hpp"

namespace Bloom
{
    struct InsightProjectSettings
    {
    public:
        std::optional<QSize> mainWindowSize;
        std::optional<Widgets::PanelState> leftPanelState;
        std::optional<Widgets::PanelState> bottomPanelState;
        std::optional<Widgets::PaneState> registersPaneState;
        std::optional<Widgets::PaneState> ramInspectionPaneState;
        std::optional<Widgets::PaneState> eepromInspectionPaneState;

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
            {MemoryRegionDataType::SIGNED_INTEGER, "signed_int"},
            {MemoryRegionDataType::ASCII_STRING, "ascii_string"},
        };

        static const inline BiMap<Targets::TargetMemoryEndianness, QString> regionEndiannessByName = {
            {Targets::TargetMemoryEndianness::LITTLE, "little"},
            {Targets::TargetMemoryEndianness::BIG, "big"},
        };

        static const inline BiMap<MemoryRegionAddressInputType, QString> addressRangeInputTypesByName = {
            {MemoryRegionAddressInputType::ABSOLUTE, "absolute"},
            {MemoryRegionAddressInputType::RELATIVE, "relative"},
        };

        [[nodiscard]] Widgets::TargetMemoryInspectionPaneSettings memoryInspectionPaneSettingsFromJson(
            const QJsonObject& jsonObject
        ) const;

        [[nodiscard]] Widgets::PanelState panelStateFromJson(const QJsonObject& jsonObject) const;

        [[nodiscard]] Widgets::PaneState paneStateFromJson(const QJsonObject& jsonObject) const;

        [[nodiscard]] QJsonObject memoryInspectionPaneSettingsToJson(
            const Widgets::TargetMemoryInspectionPaneSettings& inspectionPaneSettings
        ) const;

        [[nodiscard]] QJsonObject panelStateToJson(const Widgets::PanelState& panelState) const;

        [[nodiscard]] QJsonObject paneStateToJson(const Widgets::PaneState& paneState) const;
    };

    struct ProjectSettings
    {
        InsightProjectSettings insightSettings;

        ProjectSettings() = default;
        explicit ProjectSettings(const QJsonObject& jsonObject);

        [[nodiscard]] QJsonObject toJson() const;
    };
}
