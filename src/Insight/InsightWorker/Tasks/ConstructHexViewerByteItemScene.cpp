#include "ConstructHexViewerByteItemScene.hpp"

namespace Bloom
{
    ConstructHexViewerByteItemScene::ConstructHexViewerByteItemScene(
        const Targets::TargetMemoryDescriptor& memoryDescriptor,
        std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
        std::vector<ExcludedMemoryRegion>& excludedMemoryRegions,
        Widgets::HexViewerWidgetSettings& settings,
        Widgets::Label* hoveredAddressLabel
    )
        : memoryDescriptor(memoryDescriptor)
        , focusedMemoryRegions(focusedMemoryRegions)
        , excludedMemoryRegions(excludedMemoryRegions)
        , settings(settings)
        , hoveredAddressLabel(hoveredAddressLabel)
    {}

    void ConstructHexViewerByteItemScene::run(TargetController::TargetControllerConsole&) {
        auto* scene = new Widgets::ByteItemGraphicsScene(
            this->memoryDescriptor,
            this->focusedMemoryRegions,
            this->excludedMemoryRegions,
            this->settings,
            this->hoveredAddressLabel,
            nullptr
        );

        scene->moveToThread(nullptr);
        emit this->sceneCreated(scene);
    }
}
