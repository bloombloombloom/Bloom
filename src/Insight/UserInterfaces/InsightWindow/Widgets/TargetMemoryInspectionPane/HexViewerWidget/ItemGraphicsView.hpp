#pragma once

#include <QGraphicsView>
#include <vector>
#include <QEvent>

#include "HexViewerWidgetSettings.hpp"
#include "ItemGraphicsScene.hpp"

#include "src/Targets/TargetMemory.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/Label.hpp"

namespace Bloom::Widgets
{
    class ItemGraphicsView: public QGraphicsView
    {
        Q_OBJECT

    public:
        ItemGraphicsView(
            const Targets::TargetMemoryDescriptor& targetMemoryDescriptor,
            const std::optional<Targets::TargetMemoryBuffer>& data,
            const std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
            const std::vector<ExcludedMemoryRegion>& excludedMemoryRegions,
            HexViewerWidgetSettings& settings,
            QWidget* parent
        );

        virtual void initScene();

        [[nodiscard]] ItemGraphicsScene* getScene() const {
            return this->scene;
        }

        void scrollToByteItemAtAddress(Targets::TargetMemoryAddress address);

    signals:
        void sceneReady();

    protected:
        const Targets::TargetMemoryDescriptor& targetMemoryDescriptor;
        const std::optional<Targets::TargetMemoryBuffer>& data;
        const std::vector<FocusedMemoryRegion>& focusedMemoryRegions;
        const std::vector<ExcludedMemoryRegion>& excludedMemoryRegions;
        HexViewerWidgetSettings& settings;

        ItemGraphicsScene* scene = nullptr;

        bool event(QEvent* event) override;
        void resizeEvent(QResizeEvent* event) override;
    };
}
