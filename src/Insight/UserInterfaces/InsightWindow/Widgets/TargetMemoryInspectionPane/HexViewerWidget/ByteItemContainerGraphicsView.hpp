#pragma once

#include <QGraphicsView>
#include <QWidget>
#include <vector>
#include <QEvent>

#include "src/Targets/TargetMemory.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/Label.hpp"
#include "ByteItemGraphicsScene.hpp"
#include "HexViewerWidgetSettings.hpp"

namespace Bloom::Widgets
{
    class ByteItemContainerGraphicsView: public QGraphicsView
    {
        Q_OBJECT

    public:
        ByteItemContainerGraphicsView(QWidget* parent);

        void initScene(
            const Targets::TargetMemoryDescriptor& targetMemoryDescriptor,
            std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
            std::vector<ExcludedMemoryRegion>& excludedMemoryRegions,
            const HexViewerWidgetSettings& settings,
            Label* hoveredAddressLabel
        );

        [[nodiscard]] ByteItemGraphicsScene* getScene() const {
            return this->scene;
        }

        void scrollToByteItemAtAddress(Targets::TargetMemoryAddress address);

    signals:
        void ready();

    protected:
        bool event(QEvent* event) override;
        void resizeEvent(QResizeEvent* event) override;

    private:
        ByteItemGraphicsScene* scene = nullptr;
    };
}
