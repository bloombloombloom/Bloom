#pragma once

#include <memory>
#include <map>
#include <string>
#include <optional>
#include <QSize>
#include <QJsonObject>
#include <functional>

#include "src/Targets/TargetMemory.hpp"

#ifndef EXCLUDE_INSIGHT
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PanelState.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PaneState.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/TargetMemoryInspectionPaneSettings.hpp"
#endif

#include "src/Helpers/BiMap.hpp"

#ifndef EXCLUDE_INSIGHT
struct InsightProjectSettings
{
public:
    std::optional<QSize> mainWindowSize;
    std::optional<Widgets::PanelState> leftPanelState;
    std::optional<Widgets::PanelState> bottomPanelState;
    std::optional<Widgets::PaneState> registersPaneState;

    bool refreshRegistersOnTargetStopped = false;
    bool refreshGpioOnTargetStopped = false;

    /**
     * The key of the previously selected variant
     */
    std::optional<std::string> selectedVariantKey;

    std::map<QString, Widgets::PaneState> memoryInspectionPaneStatesByKey;
    std::map<QString, Widgets::TargetMemoryInspectionPaneSettings> memoryInspectionSettingsByKey;

    InsightProjectSettings() = default;
    explicit InsightProjectSettings(const QJsonObject& jsonObject);

    [[nodiscard]] QJsonObject toJson() const;

    Widgets::TargetMemoryInspectionPaneSettings& findOrCreateMemoryInspectionPaneSettings(
        const QString& addressSpaceKey,
        const QString& memorySegmentKey
    );

    Widgets::PaneState& findOrCreateMemoryInspectionPaneState(
        const QString& addressSpaceKey,
        const QString& memorySegmentKey
    );

private:
    static const inline BiMap<AddressType, QString> addressTypesByName = {
        {AddressType::ABSOLUTE, "absolute"},
        {AddressType::RELATIVE, "relative"},
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
#endif

struct ProjectSettings
{
#ifndef EXCLUDE_INSIGHT
    InsightProjectSettings insightSettings;
#endif

    ProjectSettings() = default;
    explicit ProjectSettings(const QJsonObject& jsonObject);

    [[nodiscard]] QJsonObject toJson() const;
};
