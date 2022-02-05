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

        if (jsonObject.contains("memoryInspectionPaneSettings")) {
            const auto settingsMappingObj = jsonObject.find("memoryInspectionPaneSettings")->toObject();

            for (auto settingsIt = settingsMappingObj.begin(); settingsIt != settingsMappingObj.end(); settingsIt++) {
                const auto settingsObj = settingsIt.value().toObject();
                const auto memoryTypeName = settingsIt.key();

                if (!InsightProjectSettings::memoryTypesByName.contains(memoryTypeName)) {
                    continue;
                }

                const auto memoryType = InsightProjectSettings::memoryTypesByName.at(memoryTypeName);
                auto inspectionPaneSettings = Widgets::TargetMemoryInspectionPaneSettings();

                if (settingsObj.contains("hexViewerSettings")) {
                    auto& hexViewerSettings = inspectionPaneSettings.hexViewerWidgetSettings;
                    const auto hexViewerSettingsObj = settingsObj.find("hexViewerSettings")->toObject();

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
                }

                if (settingsObj.contains("focusedRegions")) {
                    const auto focusedRegions = settingsObj.find("focusedRegions")->toArray();

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
                                static_cast<std::uint32_t>(addressRangeObj.find("startAddress")
                                    ->toInteger()),
                                static_cast<std::uint32_t>(addressRangeObj.find("endAddress")
                                    ->toInteger()),
                            }
                        );

                        region.createdDate.setSecsSinceEpoch(regionObj.find("createdTimestamp")->toInteger());

                        const auto addressInputType = InsightProjectSettings::addressRangeInputTypesByName.valueAt(
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

                if (settingsObj.contains("excludedRegions")) {
                    const auto excludedRegions = settingsObj.find("excludedRegions")->toArray();

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
                                static_cast<std::uint32_t>(addressRangeObj.find("startAddress")
                                    ->toInteger()),
                                static_cast<std::uint32_t>(addressRangeObj.find("endAddress")
                                    ->toInteger()),
                            }
                        );

                        region.createdDate.setSecsSinceEpoch(regionObj.find("createdTimestamp")->toInteger());

                        const auto addressInputType = InsightProjectSettings::addressRangeInputTypesByName.valueAt(
                            regionObj.find("addressInputType")->toString()
                        );

                        if (addressInputType.has_value()) {
                            region.addressRangeInputType = addressInputType.value();
                        }

                        inspectionPaneSettings.excludedMemoryRegions.emplace_back(region);
                    }
                }

                this->memoryInspectionPaneSettingsByMemoryType.insert(std::pair(memoryType, inspectionPaneSettings));
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

            auto settingsObj = QJsonObject();

            const auto& hexViewerSettings = inspectionPaneSettings.hexViewerWidgetSettings;
            settingsObj.insert("hexViewerSettings", QJsonObject({
                {"highlightStackMemory", hexViewerSettings.highlightStackMemory},
                {"highlightFocusedMemory", hexViewerSettings.highlightFocusedMemory},
                {"highlightHoveredRowAndCol", hexViewerSettings.highlightHoveredRowAndCol},
                {"displayAsciiValues", hexViewerSettings.displayAsciiValues},
                {"displayAnnotations", hexViewerSettings.displayAnnotations},
            }));

            const auto& regionDataTypesByName = InsightProjectSettings::regionDataTypesByName;
            const auto& regionEndiannessByName = InsightProjectSettings::regionEndiannessByName;
            const auto& addressRangeInputTypesByName = InsightProjectSettings::addressRangeInputTypesByName;

            auto focusedRegions = QJsonArray();
            for (const auto& focusedRegion : inspectionPaneSettings.focusedMemoryRegions) {
                if (!regionDataTypesByName.contains(focusedRegion.dataType)
                    || !regionEndiannessByName.contains(focusedRegion.endianness)
                    || !addressRangeInputTypesByName.contains(focusedRegion.addressRangeInputType)
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
                    {"addressInputType", addressRangeInputTypesByName.at(focusedRegion.addressRangeInputType)},
                    {"dataType", regionDataTypesByName.at(focusedRegion.dataType)},
                    {"endianness", regionEndiannessByName.at(focusedRegion.endianness)},
                });

                focusedRegions.push_back(regionObj);
            }

            auto excludedRegions = QJsonArray();
            for (const auto& excludedRegion : inspectionPaneSettings.excludedMemoryRegions) {
                if (!addressRangeInputTypesByName.contains(excludedRegion.addressRangeInputType)) {
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
                    {"addressInputType", addressRangeInputTypesByName.at(excludedRegion.addressRangeInputType)},
                });

                excludedRegions.push_back(regionObj);
            }

            settingsObj.insert("focusedRegions", focusedRegions);
            settingsObj.insert("excludedRegions", excludedRegions);
            memoryInspectionPaneSettingsObj.insert(
                InsightProjectSettings::memoryTypesByName.at(memoryType),
                settingsObj
            );
        }

        insightObj.insert("memoryInspectionPaneSettings", memoryInspectionPaneSettingsObj);
        return insightObj;
    }
}
