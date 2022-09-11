#include "ProjectSettings.hpp"

#include <QJsonArray>

namespace Bloom
{
    ProjectSettings::ProjectSettings(const QJsonObject& jsonObject) {
        if (jsonObject.contains("insight")) {
            this->insightSettings = InsightProjectSettings(jsonObject.find("insight")->toObject());
        }
    }

    QJsonObject ProjectSettings::toJson() const {
        auto projectSettingsObj = QJsonObject();

        projectSettingsObj.insert("insight", this->insightSettings.toJson());

        return projectSettingsObj;
    }

    InsightProjectSettings::InsightProjectSettings(const QJsonObject& jsonObject) {
        if (jsonObject.contains("mainWindowSize")) {
            const auto mainWindowSizeObj = jsonObject.find("mainWindowSize")->toObject();

            if (mainWindowSizeObj.contains("width") && mainWindowSizeObj.contains("height")) {
                this->mainWindowSize = QSize(
                    mainWindowSizeObj.find("width")->toInt(),
                    mainWindowSizeObj.find("height")->toInt()
                );
            }
        }

        if (jsonObject.contains("leftPanelState")) {
            this->leftPanelState = this->panelStateFromJson(
                jsonObject.find("leftPanelState")->toObject()
            );
        }

        if (jsonObject.contains("bottomPanelState")) {
            this->bottomPanelState = this->panelStateFromJson(
                jsonObject.find("bottomPanelState")->toObject()
            );
        }

        if (jsonObject.contains("registersPaneState")) {
            this->registersPaneState = this->paneStateFromJson(
                jsonObject.find("registersPaneState")->toObject()
            );
        }

        if (jsonObject.contains("ramInspectionPaneState")) {
            this->ramInspectionPaneState = this->paneStateFromJson(
                jsonObject.find("ramInspectionPaneState")->toObject()
            );
        }

        if (jsonObject.contains("eepromInspectionPaneState")) {
            this->eepromInspectionPaneState = this->paneStateFromJson(
                jsonObject.find("eepromInspectionPaneState")->toObject()
            );
        }

        if (jsonObject.contains("memoryInspectionPaneSettings")) {
            const auto settingsMappingObj = jsonObject.find("memoryInspectionPaneSettings")->toObject();

            for (auto settingsIt = settingsMappingObj.begin(); settingsIt != settingsMappingObj.end(); settingsIt++) {
                const auto settingsObj = settingsIt.value().toObject();
                const auto memoryTypeName = settingsIt.key();

                if (!InsightProjectSettings::memoryTypesByName.contains(memoryTypeName)) {
                    continue;
                }

                this->memoryInspectionPaneSettingsByMemoryType.insert(std::pair(
                    InsightProjectSettings::memoryTypesByName.at(memoryTypeName),
                    this->memoryInspectionPaneSettingsFromJson(settingsObj)
                ));
            }
        }
    }

    QJsonObject InsightProjectSettings::toJson() const {
        auto insightObj = QJsonObject();

        if (this->mainWindowSize.has_value()) {
            insightObj.insert("mainWindowSize", QJsonObject({
                {"width", this->mainWindowSize->width()},
                {"height", this->mainWindowSize->height()},
            }));
        }

        auto memoryInspectionPaneSettingsObj = QJsonObject();

        for (const auto& [memoryType, inspectionPaneSettings] : this->memoryInspectionPaneSettingsByMemoryType) {
            if (!InsightProjectSettings::memoryTypesByName.contains(memoryType)) {
                // This is just a precaution - all known memory types should be in the mapping.
                continue;
            }

            memoryInspectionPaneSettingsObj.insert(
                InsightProjectSettings::memoryTypesByName.at(memoryType),
                this->memoryInspectionPaneSettingsToJson(inspectionPaneSettings)
            );
        }

        insightObj.insert("memoryInspectionPaneSettings", memoryInspectionPaneSettingsObj);

        if (this->leftPanelState.has_value()) {
            insightObj.insert(
                "leftPanelState",
                this->panelStateToJson(this->leftPanelState.value())
            );
        }

        if (this->bottomPanelState.has_value()) {
            insightObj.insert(
                "bottomPanelState",
                this->panelStateToJson(this->bottomPanelState.value())
            );
        }

        if (this->registersPaneState.has_value()) {
            insightObj.insert(
                "registersPaneState",
                this->paneStateToJson(this->registersPaneState.value())
            );
        }

        if (this->ramInspectionPaneState.has_value()) {
            insightObj.insert(
                "ramInspectionPaneState",
                this->paneStateToJson(this->ramInspectionPaneState.value())
            );
        }

        if (this->eepromInspectionPaneState.has_value()) {
            insightObj.insert(
                "eepromInspectionPaneState",
                this->paneStateToJson(this->eepromInspectionPaneState.value())
            );
        }

        return insightObj;
    }

    Widgets::TargetMemoryInspectionPaneSettings InsightProjectSettings::memoryInspectionPaneSettingsFromJson(
        const QJsonObject& jsonObject
    ) const {
        auto inspectionPaneSettings = Widgets::TargetMemoryInspectionPaneSettings();

        if (jsonObject.contains("refreshOnTargetStop")) {
            inspectionPaneSettings.refreshOnTargetStop = jsonObject.value("refreshOnTargetStop").toBool();
        }

        if (jsonObject.contains("refreshOnActivation")) {
            inspectionPaneSettings.refreshOnActivation = jsonObject.value("refreshOnActivation").toBool();
        }

        if (jsonObject.contains("hexViewerSettings")) {
            auto& hexViewerSettings = inspectionPaneSettings.hexViewerWidgetSettings;
            const auto hexViewerSettingsObj = jsonObject.find("hexViewerSettings")->toObject();

            if (hexViewerSettingsObj.contains("highlightStackMemory")) {
                hexViewerSettings.highlightStackMemory =
                    hexViewerSettingsObj.value("highlightStackMemory").toBool();
            }

            if (hexViewerSettingsObj.contains("highlightFocusedMemory")) {
                hexViewerSettings.highlightFocusedMemory =
                    hexViewerSettingsObj.value("highlightFocusedMemory").toBool();
            }

            if (hexViewerSettingsObj.contains("highlightHoveredRowAndCol")) {
                hexViewerSettings.highlightHoveredRowAndCol =
                    hexViewerSettingsObj.value("highlightHoveredRowAndCol").toBool();
            }

            if (hexViewerSettingsObj.contains("displayAsciiValues")) {
                hexViewerSettings.displayAsciiValues =
                    hexViewerSettingsObj.value("displayAsciiValues").toBool();
            }

            if (hexViewerSettingsObj.contains("displayAnnotations")) {
                hexViewerSettings.displayAnnotations =
                    hexViewerSettingsObj.value("displayAnnotations").toBool();
            }

            if (hexViewerSettingsObj.contains("addressLabelType")) {
                hexViewerSettings.addressLabelType = InsightProjectSettings::addressTypesByName.valueAt(
                    hexViewerSettingsObj.value("addressLabelType").toString()
                ).value_or(hexViewerSettings.addressLabelType);
            }
        }

        if (jsonObject.contains("focusedRegions")) {
            const auto focusedRegions = jsonObject.find("focusedRegions")->toArray();

            for (const auto& regionValue : focusedRegions) {
                const auto regionObj = regionValue.toObject();

                if (!regionObj.contains("name")
                    || !regionObj.contains("addressRange")
                    || !regionObj.contains("addressInputType")
                    || !regionObj.contains("createdTimestamp")
                    || !regionObj.contains("dataType")
                ) {
                    continue;
                }

                const auto addressRangeObj = regionObj.find("addressRange")->toObject();
                if (!addressRangeObj.contains("startAddress")
                    || !addressRangeObj.contains("endAddress")
                ) {
                    continue;
                }

                auto region = FocusedMemoryRegion(
                    regionObj.find("name")->toString(),
                    {
                        static_cast<std::uint32_t>(addressRangeObj.find("startAddress")->toInteger()),
                        static_cast<std::uint32_t>(addressRangeObj.find("endAddress")->toInteger()),
                    }
                );

                region.createdDate.setSecsSinceEpoch(regionObj.find("createdTimestamp")->toInteger());

                const auto addressInputType = InsightProjectSettings::addressTypesByName.valueAt(
                    regionObj.find("addressInputType")->toString()
                );

                if (addressInputType.has_value()) {
                    region.addressRangeInputType = addressInputType.value();
                }

                const auto dataType = InsightProjectSettings::regionDataTypesByName.valueAt(
                    regionObj.find("dataType")->toString()
                );

                if (dataType.has_value()) {
                    region.dataType = dataType.value();
                }

                const auto endianness = InsightProjectSettings::regionEndiannessByName.valueAt(
                    regionObj.find("endianness")->toString()
                );

                if (endianness.has_value()) {
                    region.endianness = endianness.value();
                }

                inspectionPaneSettings.focusedMemoryRegions.emplace_back(region);
            }
        }

        if (jsonObject.contains("excludedRegions")) {
            const auto excludedRegions = jsonObject.find("excludedRegions")->toArray();

            for (const auto& regionValue : excludedRegions) {
                const auto regionObj = regionValue.toObject();

                if (!regionObj.contains("name")
                    || !regionObj.contains("addressRange")
                    || !regionObj.contains("addressInputType")
                    || !regionObj.contains("createdTimestamp")
                ) {
                    continue;
                }

                const auto addressRangeObj = regionObj.find("addressRange")->toObject();
                if (!addressRangeObj.contains("startAddress")
                    || !addressRangeObj.contains("endAddress")
                ) {
                    continue;
                }

                auto region = ExcludedMemoryRegion(
                    regionObj.find("name")->toString(),
                    {
                        static_cast<std::uint32_t>(addressRangeObj.find("startAddress")->toInteger()),
                        static_cast<std::uint32_t>(addressRangeObj.find("endAddress")->toInteger()),
                    }
                );

                region.createdDate.setSecsSinceEpoch(regionObj.find("createdTimestamp")->toInteger());

                const auto addressInputType = InsightProjectSettings::addressTypesByName.valueAt(
                    regionObj.find("addressInputType")->toString()
                );

                if (addressInputType.has_value()) {
                    region.addressRangeInputType = addressInputType.value();
                }

                inspectionPaneSettings.excludedMemoryRegions.emplace_back(region);
            }
        }

        return inspectionPaneSettings;
    }

    Widgets::PanelState InsightProjectSettings::panelStateFromJson(const QJsonObject& jsonObject) const {
        return Widgets::PanelState(
            (jsonObject.contains("size") ? static_cast<int>(jsonObject.value("size").toInteger()) : 0),
            (jsonObject.contains("open") ? jsonObject.value("open").toBool() : false)
        );
    }

    Widgets::PaneState InsightProjectSettings::paneStateFromJson(const QJsonObject& jsonObject) const {
        auto detachedWindowState = std::optional<Widgets::DetachedWindowState>(std::nullopt);

        if (jsonObject.contains("detachedWindowState")) {
            detachedWindowState = Widgets::DetachedWindowState();

            const auto detachedWindowStateObject = jsonObject.value("detachedWindowState").toObject();

            if (detachedWindowStateObject.contains("size")) {
                const auto sizeObject = detachedWindowStateObject.value("size").toObject();
                detachedWindowState->size = QSize(
                    sizeObject.value("width").toInt(0),
                    sizeObject.value("height").toInt(0)
                );
            }

            if (detachedWindowStateObject.contains("position")) {
                const auto positionObject = detachedWindowStateObject.value("position").toObject();
                detachedWindowState->position = QPoint(
                    positionObject.value("x").toInt(0),
                    positionObject.value("y").toInt(0)
                );
            }
        }

        return Widgets::PaneState(
            (jsonObject.contains("activated") ? jsonObject.value("activated").toBool() : false),
            (jsonObject.contains("attached") ? jsonObject.value("attached").toBool() : true),
            detachedWindowState
        );
    }

    QJsonObject InsightProjectSettings::memoryInspectionPaneSettingsToJson(
        const Widgets::TargetMemoryInspectionPaneSettings& inspectionPaneSettings
    ) const {
        const auto& regionDataTypesByName = InsightProjectSettings::regionDataTypesByName;
        const auto& regionEndiannessByName = InsightProjectSettings::regionEndiannessByName;
        const auto& addressTypesByName = InsightProjectSettings::addressTypesByName;

        auto settingsObj = QJsonObject({
            {"refreshOnTargetStop", inspectionPaneSettings.refreshOnTargetStop},
            {"refreshOnActivation", inspectionPaneSettings.refreshOnActivation},
        });

        const auto& hexViewerSettings = inspectionPaneSettings.hexViewerWidgetSettings;
        settingsObj.insert("hexViewerSettings", QJsonObject({
            {"highlightStackMemory", hexViewerSettings.highlightStackMemory},
            {"highlightFocusedMemory", hexViewerSettings.highlightFocusedMemory},
            {"highlightHoveredRowAndCol", hexViewerSettings.highlightHoveredRowAndCol},
            {"displayAsciiValues", hexViewerSettings.displayAsciiValues},
            {"displayAnnotations", hexViewerSettings.displayAnnotations},
            {"addressLabelType", addressTypesByName.valueAt(hexViewerSettings.addressLabelType).value()},
        }));

        auto focusedRegions = QJsonArray();
        for (const auto& focusedRegion : inspectionPaneSettings.focusedMemoryRegions) {
            if (!regionDataTypesByName.contains(focusedRegion.dataType)
                || !regionEndiannessByName.contains(focusedRegion.endianness)
                || !addressTypesByName.contains(focusedRegion.addressRangeInputType)
            ) {
                continue;
            }

            const auto addressRangeObj = QJsonObject({
                {"startAddress", static_cast<qint64>(focusedRegion.addressRange.startAddress)},
                {"endAddress", static_cast<qint64>(focusedRegion.addressRange.endAddress)},
            });

            auto regionObj = QJsonObject({
                {"name", focusedRegion.name},
                {"addressRange", addressRangeObj},
                {"createdTimestamp", focusedRegion.createdDate.toSecsSinceEpoch()},
                {"addressInputType", addressTypesByName.at(focusedRegion.addressRangeInputType)},
                {"dataType", regionDataTypesByName.at(focusedRegion.dataType)},
                {"endianness", regionEndiannessByName.at(focusedRegion.endianness)},
            });

            focusedRegions.push_back(regionObj);
        }

        auto excludedRegions = QJsonArray();
        for (const auto& excludedRegion : inspectionPaneSettings.excludedMemoryRegions) {
            if (!addressTypesByName.contains(excludedRegion.addressRangeInputType)) {
                continue;
            }

            const auto addressRangeObj = QJsonObject({
                {"startAddress", static_cast<qint64>(excludedRegion.addressRange.startAddress)},
                {"endAddress", static_cast<qint64>(excludedRegion.addressRange.endAddress)},
            });

            auto regionObj = QJsonObject({
                {"name", excludedRegion.name},
                {"addressRange", addressRangeObj},
                {"createdTimestamp", excludedRegion.createdDate.toSecsSinceEpoch()},
                {"addressInputType", addressTypesByName.at(excludedRegion.addressRangeInputType)},
            });

            excludedRegions.push_back(regionObj);
        }

        settingsObj.insert("focusedRegions", focusedRegions);
        settingsObj.insert("excludedRegions", excludedRegions);

        return settingsObj;
    }

    QJsonObject InsightProjectSettings::panelStateToJson(const Widgets::PanelState& panelState) const {
        return QJsonObject({
            {"size", panelState.size},
            {"open", panelState.open},
        });
    }

    QJsonObject InsightProjectSettings::paneStateToJson(const Widgets::PaneState& paneState) const {
        auto json = QJsonObject({
            {"activated", paneState.activated},
            {"attached", paneState.attached},
        });

        if (paneState.detachedWindowState.has_value()) {
            json.insert("detachedWindowState", QJsonObject({
                {
                    "size",
                    QJsonObject({
                        {"width", paneState.detachedWindowState->size.width()},
                        {"height", paneState.detachedWindowState->size.height()},
                    })
                },
                {
                    "position",
                    QJsonObject({
                        {"x", paneState.detachedWindowState->position.x()},
                        {"y", paneState.detachedWindowState->position.y()},
                    })
                }
            }));
        }

        return json;
    }
}
