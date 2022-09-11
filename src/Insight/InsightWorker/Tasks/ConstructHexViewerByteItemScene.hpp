#pragma once

#include "InsightWorkerTask.hpp"
#include "src/Targets/TargetMemory.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/HexViewerWidgetSettings.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/FocusedMemoryRegion.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/ExcludedMemoryRegion.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/ByteItemGraphicsScene.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/Label.hpp"

namespace Bloom
{
    class ConstructHexViewerByteItemScene: public InsightWorkerTask
    {
        Q_OBJECT

    public:
        ConstructHexViewerByteItemScene(
            const Targets::TargetMemoryDescriptor& memoryDescriptor,
            std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
            std::vector<ExcludedMemoryRegion>& excludedMemoryRegions,
            Widgets::HexViewerWidgetSettings& settings,
            Widgets::Label* hoveredAddressLabel
        );

        TaskGroups getTaskGroups() const override {
            return TaskGroups();
        };

    signals:
        void sceneCreated(Widgets::ByteItemGraphicsScene* scene);

    protected:
        void run(TargetController::TargetControllerConsole& targetControllerConsole) override;

    private:
        const Targets::TargetMemoryDescriptor& memoryDescriptor;
        std::vector<FocusedMemoryRegion>& focusedMemoryRegions;
        std::vector<ExcludedMemoryRegion>& excludedMemoryRegions;
        Widgets::HexViewerWidgetSettings& settings;
        Widgets::Label* hoveredAddressLabel;
    };
}
