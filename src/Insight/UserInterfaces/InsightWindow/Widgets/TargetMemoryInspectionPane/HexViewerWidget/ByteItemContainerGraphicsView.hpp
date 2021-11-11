#pragma once

#include <QGraphicsView>
#include <QWidget>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>
#include <map>
#include <vector>
#include <QSize>
#include <QString>
#include <QEvent>
#include <QGraphicsSceneMouseEvent>
#include <optional>

#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetState.hpp"

#include "src/Insight/InsightWorker/InsightWorker.hpp"

#include "ByteItemGraphicsScene.hpp"
#include "HexViewerWidgetSettings.hpp"

namespace Bloom::Widgets
{
    class ByteItemContainerGraphicsView: public QGraphicsView
    {
        Q_OBJECT

    public:
        ByteItemContainerGraphicsView(
            const Targets::TargetMemoryDescriptor& targetMemoryDescriptor,
            InsightWorker& insightWorker,
            const HexViewerWidgetSettings& settings,
            QLabel* hoveredAddressLabel,
            QWidget* parent
        );

        [[nodiscard]] ByteItemGraphicsScene* getScene() const {
            return this->scene;
        }

    protected:
        bool event(QEvent* event) override;
        void resizeEvent(QResizeEvent* event) override;

    private:
        ByteItemGraphicsScene* scene = nullptr;
    };
}
