#pragma once

#include <memory>
#include <map>
#include <string>
#include <QJsonObject>

#include "src/Targets/TargetMemory.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/TargetMemoryInspectionPaneSettings.hpp"

namespace Bloom
{
    struct InsightProjectSettings
    {
        std::map<
            Targets::TargetMemoryType,
            Widgets::TargetMemoryInspectionPaneSettings
        > memoryInspectionPaneSettingsByMemoryType;

        InsightProjectSettings() = default;
        InsightProjectSettings(const QJsonObject& jsonObject);
    };

    struct ProjectSettings
    {
        InsightProjectSettings insightSettings;

        ProjectSettings() = default;
        explicit ProjectSettings(const QJsonObject& jsonObject);
    };
}
