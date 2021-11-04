#pragma once

#include <QGraphicsScene>
#include <QWidget>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>
#include <map>
#include <vector>
#include <QSize>
#include <QString>
#include <QGraphicsSceneMouseEvent>
#include <optional>

#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetState.hpp"

#include "src/Insight/InsightWorker/InsightWorker.hpp"

#include "ByteItem.hpp"
#include "ByteAddressContainer.hpp"

namespace Bloom::Widgets
{
    class ByteItemGraphicsScene: public QGraphicsScene
    {
        Q_OBJECT

    public:
        std::optional<ByteItem*> hoveredByteWidget;

        std::map<std::uint32_t, ByteItem*> byteItemsByAddress;
        std::map<std::size_t, std::vector<ByteItem*>> byteItemsByRowIndex;
        std::map<std::size_t, std::vector<ByteItem*>> byteItemsByColumnIndex;

        ByteItemGraphicsScene(
            const Targets::TargetMemoryDescriptor& targetMemoryDescriptor,
            InsightWorker& insightWorker,
            QLabel* hoveredAddressLabel,
            QWidget* parent
        );

        void updateValues(const Targets::TargetMemoryBuffer& buffer);

        void adjustByteWidgets();

        void setEnabled(bool enabled);

    signals:
        void byteWidgetsAdjusted();

    protected:
        bool event(QEvent* event) override;
        void mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

    private:
        const Targets::TargetMemoryDescriptor& targetMemoryDescriptor;
        Targets::TargetState targetState = Targets::TargetState::UNKNOWN;
        InsightWorker& insightWorker;

        QWidget* parent = nullptr;
        QLabel* hoveredAddressLabel = nullptr;

        ByteAddressContainer* byteAddressContainer = nullptr;

        bool enabled = true;

    private slots:
        void onTargetStateChanged(Targets::TargetState newState);
        void onByteWidgetEnter(Bloom::Widgets::ByteItem* widget);
        void onByteWidgetLeave();
    };
}
