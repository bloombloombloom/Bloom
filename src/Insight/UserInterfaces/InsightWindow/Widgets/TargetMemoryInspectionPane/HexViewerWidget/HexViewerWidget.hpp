#pragma once

#include <QWidget>
#include <QLabel>
#include <QGraphicsView>
#include <QVBoxLayout>
#include <set>
#include <map>
#include <QSize>
#include <QString>
#include <QEvent>
#include <optional>

#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetState.hpp"

#include "src/Insight/InsightWorker/InsightWorker.hpp"

#include "ByteItemContainerGraphicsView.hpp"

namespace Bloom::Widgets
{
    class HexViewerWidget: public QWidget
    {
        Q_OBJECT

    public:
        QToolButton* refreshButton = nullptr;

        HexViewerWidget(
            const Targets::TargetMemoryDescriptor& targetMemoryDescriptor,
            InsightWorker& insightWorker,
            QWidget* parent
        );

        void updateValues(const Targets::TargetMemoryBuffer& buffer);

    protected:
        void resizeEvent(QResizeEvent* event) override;

    private:
        const Targets::TargetMemoryDescriptor& targetMemoryDescriptor;
        InsightWorker& insightWorker;

        QWidget* container = nullptr;
        QWidget* toolBar = nullptr;
        QWidget* bottomBar = nullptr;

        ByteItemContainerGraphicsView* byteItemGraphicsView = nullptr;
        ByteItemGraphicsScene* byteItemGraphicsScene = nullptr;
        QWidget* byteWidgetScrollArea = nullptr;
        QWidget* byteWidgetAddressContainer = nullptr;
        QVBoxLayout* byteWidgetAddressLayout = nullptr;
        QLabel* hoveredAddressLabel = nullptr;

        Targets::TargetState targetState = Targets::TargetState::UNKNOWN;

    private slots:
        void onTargetStateChanged(Targets::TargetState newState);
        void onByteWidgetsAdjusted();
    };
}
