#pragma once

#include <optional>
#include <set>

#include "InsightWorkerTask.hpp"
#include "src/Targets/TargetMemory.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/HexViewerWidgetSettings.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/FocusedMemoryRegion.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/ExcludedMemoryRegion.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/ByteItemGraphicsScene.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/ByteItem.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/Label.hpp"

namespace Bloom
{
    class ConstructHexViewerByteItems: public InsightWorkerTask
    {
        Q_OBJECT

    public:
        ConstructHexViewerByteItems(
            const Targets::TargetMemoryDescriptor& memoryDescriptor,
            std::optional<Targets::TargetStackPointer>& currentStackPointer,
            Widgets::ByteItem** hoveredByteItem,
            std::set<Widgets::ByteItem*>& highlightedByteItems,
            Widgets::HexViewerWidgetSettings& settings
        );

        TaskGroups getTaskGroups() const override {
            return TaskGroups();
        };

    signals:
        void sceneCreated(Widgets::ByteItemGraphicsScene* scene);
        void byteItems(std::map<Targets::TargetMemoryAddress, Widgets::ByteItem*> byteItemsByAddress);

    protected:
        void run(TargetController::TargetControllerConsole&) override;

    private:
        std::map<Targets::TargetMemoryAddress, Widgets::ByteItem*> byteItemsByAddress;

        const Targets::TargetMemoryDescriptor& memoryDescriptor;
        std::optional<Targets::TargetStackPointer>& currentStackPointer;
        Widgets::ByteItem** hoveredByteItem;
        std::set<Widgets::ByteItem*>& highlightedByteItems;
        Widgets::HexViewerWidgetSettings& settings;
    };
}
