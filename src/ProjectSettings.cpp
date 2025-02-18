#include "ProjectSettings.hpp"

#include <QJsonArray>

#include "src/Helpers/EnumToStringMappings.hpp"
#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"


ProjectSettings::ProjectSettings(const QJsonObject& jsonObject) {
#ifndef EXCLUDE_INSIGHT
    if (jsonObject.contains("insight")) {
        this->insightSettings = InsightProjectSettings{jsonObject.find("insight")->toObject()};
    }
#endif
}

QJsonObject ProjectSettings::toJson() const {
    auto projectSettingsObj = QJsonObject{};

#ifndef EXCLUDE_INSIGHT
    projectSettingsObj.insert("insight", this->insightSettings.toJson());
#endif

    return projectSettingsObj;
}

#ifndef EXCLUDE_INSIGHT
InsightProjectSettings::InsightProjectSettings(const QJsonObject& jsonObject) {
    if (jsonObject.contains("mainWindowSize")) {
        const auto mainWindowSizeObj = jsonObject.find("mainWindowSize")->toObject();

        if (mainWindowSizeObj.contains("width") && mainWindowSizeObj.contains("height")) {
            this->mainWindowSize = QSize{
                mainWindowSizeObj.find("width")->toInt(),
                mainWindowSizeObj.find("height")->toInt()
            };
        }
    }

    if (jsonObject.contains("leftPanelState")) {
        this->leftPanelState = this->panelStateFromJson(jsonObject.find("leftPanelState")->toObject());
    }

    if (jsonObject.contains("bottomPanelState")) {
        this->bottomPanelState = this->panelStateFromJson(jsonObject.find("bottomPanelState")->toObject());
    }

    if (jsonObject.contains("registersPaneState")) {
        this->registersPaneState = this->paneStateFromJson(jsonObject.find("registersPaneState")->toObject());
    }

    if (jsonObject.contains("refreshRegistersOnTargetStopped")) {
        this->refreshRegistersOnTargetStopped = jsonObject.value("refreshRegistersOnTargetStopped").toBool();
    }

    if (jsonObject.contains("refreshGpioOnTargetStopped")) {
        this->refreshGpioOnTargetStopped = jsonObject.value("refreshGpioOnTargetStopped").toBool();
    }

    if (jsonObject.contains("selectedVariantKey")) {
        this->selectedVariantKey = jsonObject.find("selectedVariantKey")->toString().toStdString();
    }

    if (jsonObject.contains("memoryInspectionPaneStatesByKey")) {
        const auto stateMappingObj = jsonObject.find("memoryInspectionPaneStatesByKey")->toObject();

        for (auto stateIt = stateMappingObj.begin(); stateIt != stateMappingObj.end(); stateIt++) {
            this->memoryInspectionPaneStatesByKey.emplace(
                stateIt.key(),
                this->paneStateFromJson(stateIt.value().toObject())
            );
        }
    }

    if (jsonObject.contains("memoryInspectionSettingsByKey")) {
        const auto settingsMappingObj = jsonObject.find("memoryInspectionSettingsByKey")->toObject();

        for (auto settingsIt = settingsMappingObj.begin(); settingsIt != settingsMappingObj.end(); settingsIt++) {
            this->memoryInspectionSettingsByKey.emplace(
                settingsIt.key(),
                this->memoryInspectionPaneSettingsFromJson(settingsIt.value().toObject())
            );
        }
    }
}

QJsonObject InsightProjectSettings::toJson() const {
    auto insightObj = QJsonObject{};

    if (this->mainWindowSize.has_value()) {
        insightObj.insert("mainWindowSize", QJsonObject{
            {"width", this->mainWindowSize->width()},
            {"height", this->mainWindowSize->height()},
        });
    }

    if (this->leftPanelState.has_value()) {
        insightObj.insert("leftPanelState", this->panelStateToJson(this->leftPanelState.value()));
    }

    if (this->bottomPanelState.has_value()) {
        insightObj.insert("bottomPanelState", this->panelStateToJson(this->bottomPanelState.value()));
    }

    if (this->registersPaneState.has_value()) {
        insightObj.insert("registersPaneState", this->paneStateToJson(this->registersPaneState.value()));
    }

    insightObj.insert("refreshRegistersOnTargetStopped", this->refreshRegistersOnTargetStopped);
    insightObj.insert("refreshGpioOnTargetStopped", this->refreshGpioOnTargetStopped);

    if (this->selectedVariantKey.has_value()) {
        insightObj.insert("selectedVariantKey", QString::fromStdString(this->selectedVariantKey.value()));
    }

    auto memoryInspectionPaneStates = QJsonObject{};
    for (const auto& [key, paneState] : this->memoryInspectionPaneStatesByKey) {
        memoryInspectionPaneStates.insert(key, this->paneStateToJson(paneState));
    }

    insightObj.insert("memoryInspectionPaneStatesByKey", memoryInspectionPaneStates);

    auto memoryInspectionSettings = QJsonObject{};
    for (const auto& [key, settings] : this->memoryInspectionSettingsByKey) {
        memoryInspectionSettings.insert(key, this->memoryInspectionPaneSettingsToJson(settings));
    }

    insightObj.insert("memoryInspectionSettingsByKey", memoryInspectionSettings);

    return insightObj;
}

Widgets::PaneState& InsightProjectSettings::findOrCreateMemoryInspectionPaneState(
    const QString& addressSpaceKey,
    const QString& memorySegmentKey
) {
    auto key = addressSpaceKey + "." + memorySegmentKey;
    for (auto& [paneStateKey, paneState] : this->memoryInspectionPaneStatesByKey) {
        if (paneStateKey == key) {
            return paneState;
        }
    }

    return this->memoryInspectionPaneStatesByKey.emplace(
        std::move(key),
        Widgets::PaneState{false, true, std::nullopt}
    ).first->second;
}

Widgets::TargetMemoryInspectionPaneSettings& InsightProjectSettings::findOrCreateMemoryInspectionPaneSettings(
    const QString& addressSpaceKey,
    const QString& memorySegmentKey
) {
    auto key = addressSpaceKey + "." + memorySegmentKey;
    for (auto& [settingsKey, settings] : this->memoryInspectionSettingsByKey) {
        if (settingsKey == key) {
            return settings;
        }
    }

    return this->memoryInspectionSettingsByKey.emplace(
        std::move(key),
        Widgets::TargetMemoryInspectionPaneSettings{addressSpaceKey, memorySegmentKey}
    ).first->second;
}

Widgets::TargetMemoryInspectionPaneSettings InsightProjectSettings::memoryInspectionPaneSettingsFromJson(
    const QJsonObject& jsonObject
) const {
    using Exceptions::Exception;

    auto inspectionPaneSettings = Widgets::TargetMemoryInspectionPaneSettings{
        jsonObject.value("addressSpaceKey").toString(),
        jsonObject.value("memorySegmentKey").toString()
    };

    if (jsonObject.contains("refreshOnTargetStop")) {
        inspectionPaneSettings.refreshOnTargetStop = jsonObject.value("refreshOnTargetStop").toBool();
    }

    if (jsonObject.contains("refreshOnActivation")) {
        inspectionPaneSettings.refreshOnActivation = jsonObject.value("refreshOnActivation").toBool();
    }

    if (jsonObject.contains("hexViewerSettings")) {
        auto& hexViewerSettings = inspectionPaneSettings.hexViewerWidgetSettings;
        const auto hexViewerSettingsObj = jsonObject.find("hexViewerSettings")->toObject();

        if (hexViewerSettingsObj.contains("groupStackMemory")) {
            hexViewerSettings.groupStackMemory = hexViewerSettingsObj.value("groupStackMemory").toBool();
        }

        if (hexViewerSettingsObj.contains("highlightFocusedMemory")) {
            hexViewerSettings.highlightFocusedMemory = hexViewerSettingsObj.value("highlightFocusedMemory").toBool();
        }

        if (hexViewerSettingsObj.contains("highlightHoveredRowAndCol")) {
            hexViewerSettings.highlightHoveredRowAndCol = hexViewerSettingsObj.value(
                "highlightHoveredRowAndCol"
            ).toBool();
        }

        if (hexViewerSettingsObj.contains("displayAsciiValues")) {
            hexViewerSettings.displayAsciiValues = hexViewerSettingsObj.value("displayAsciiValues").toBool();
        }

        if (hexViewerSettingsObj.contains("displayAnnotations")) {
            hexViewerSettings.displayAnnotations = hexViewerSettingsObj.value("displayAnnotations").toBool();
        }

        if (hexViewerSettingsObj.contains("addressLabelType")) {
            hexViewerSettings.addressLabelType = InsightProjectSettings::addressTypesByName.valueAt(
                hexViewerSettingsObj.value("addressLabelType").toString()
            ).value_or(hexViewerSettings.addressLabelType);
        }
    }

    if (jsonObject.contains("focusedRegions")) {
        for (const auto& regionValue : jsonObject.find("focusedRegions")->toArray()) {
            try {
                inspectionPaneSettings.focusedMemoryRegions.emplace_back(regionValue.toObject());

            } catch (const Exception& exception) {
                Logger::warning(
                    "Failed to parse focused memory region from project settings file - "
                        + exception.getMessage() + " - region will be ignored."
                );
            }
        }
    }

    if (jsonObject.contains("excludedRegions")) {
        for (const auto& regionValue : jsonObject.find("excludedRegions")->toArray()) {
            try {
                inspectionPaneSettings.excludedMemoryRegions.emplace_back(regionValue.toObject());

            } catch (const Exception& exception) {
                Logger::warning(
                    "Failed to parse excluded memory region from project settings file - "
                        + exception.getMessage() + " - region will be ignored."
                );
            }
        }
    }

    return inspectionPaneSettings;
}

Widgets::PanelState InsightProjectSettings::panelStateFromJson(const QJsonObject& jsonObject) const {
    return Widgets::PanelState{
        (jsonObject.contains("size") ? static_cast<int>(jsonObject.value("size").toInteger()) : 0),
        (jsonObject.contains("open") ? jsonObject.value("open").toBool() : false)
    };
}

Widgets::PaneState InsightProjectSettings::paneStateFromJson(const QJsonObject& jsonObject) const {
    auto detachedWindowState = std::optional<Widgets::DetachedWindowState>{};

    if (jsonObject.contains("detachedWindowState")) {
        detachedWindowState = Widgets::DetachedWindowState{};

        const auto detachedWindowStateObject = jsonObject.value("detachedWindowState").toObject();

        if (detachedWindowStateObject.contains("size")) {
            const auto sizeObject = detachedWindowStateObject.value("size").toObject();
            detachedWindowState->size = QSize{
                sizeObject.value("width").toInt(0),
                sizeObject.value("height").toInt(0)
            };
        }

        if (detachedWindowStateObject.contains("position")) {
            const auto positionObject = detachedWindowStateObject.value("position").toObject();
            detachedWindowState->position = QPoint{
                positionObject.value("x").toInt(0),
                positionObject.value("y").toInt(0)
            };
        }
    }

    return Widgets::PaneState{
        (jsonObject.contains("activated") ? jsonObject.value("activated").toBool() : false),
        (jsonObject.contains("attached") ? jsonObject.value("attached").toBool() : true),
        detachedWindowState
    };
}

QJsonObject InsightProjectSettings::memoryInspectionPaneSettingsToJson(
    const Widgets::TargetMemoryInspectionPaneSettings& inspectionPaneSettings
) const {
    const auto& addressTypesByName = InsightProjectSettings::addressTypesByName;

    auto settingsObj = QJsonObject{
        {"refreshOnTargetStop", inspectionPaneSettings.refreshOnTargetStop},
        {"refreshOnActivation", inspectionPaneSettings.refreshOnActivation},
    };

    const auto& hexViewerSettings = inspectionPaneSettings.hexViewerWidgetSettings;
    settingsObj.insert("hexViewerSettings", QJsonObject{
        {"groupStackMemory", hexViewerSettings.groupStackMemory},
        {"highlightFocusedMemory", hexViewerSettings.highlightFocusedMemory},
        {"highlightHoveredRowAndCol", hexViewerSettings.highlightHoveredRowAndCol},
        {"displayAsciiValues", hexViewerSettings.displayAsciiValues},
        {"displayAnnotations", hexViewerSettings.displayAnnotations},
        {"addressLabelType", addressTypesByName.valueAt(hexViewerSettings.addressLabelType).value()},
    });

    auto focusedRegions = QJsonArray{};
    for (const auto& focusedRegion : inspectionPaneSettings.focusedMemoryRegions) {
        focusedRegions.push_back(focusedRegion.toJson());
    }

    auto excludedRegions = QJsonArray{};
    for (const auto& excludedRegion : inspectionPaneSettings.excludedMemoryRegions) {
        excludedRegions.push_back(excludedRegion.toJson());
    }

    settingsObj.insert("focusedRegions", focusedRegions);
    settingsObj.insert("excludedRegions", excludedRegions);

    return settingsObj;
}

QJsonObject InsightProjectSettings::panelStateToJson(const Widgets::PanelState& panelState) const {
    return QJsonObject{
        {"size", panelState.size},
        {"open", panelState.open},
    };
}

QJsonObject InsightProjectSettings::paneStateToJson(const Widgets::PaneState& paneState) const {
    auto json = QJsonObject{
        {"activated", paneState.activated},
        {"attached", paneState.attached},
    };

    if (paneState.detachedWindowState.has_value()) {
        json.insert("detachedWindowState", QJsonObject{
            {
                "size",
                QJsonObject{
                    {"width", paneState.detachedWindowState->size.width()},
                    {"height", paneState.detachedWindowState->size.height()},
                }
            },
            {
                "position",
                QJsonObject{
                    {"x", paneState.detachedWindowState->position.x()},
                    {"y", paneState.detachedWindowState->position.y()},
                }
            }
        });
    }

    return json;
}
#endif
