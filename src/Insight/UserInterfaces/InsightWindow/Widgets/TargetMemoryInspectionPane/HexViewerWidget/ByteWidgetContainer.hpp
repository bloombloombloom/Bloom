#pragma once

#include <QWidget>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>
#include <map>
#include <vector>
#include <QSize>
#include <QString>
#include <QEvent>
#include <optional>

#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetState.hpp"

#include "src/Insight/InsightWorker/InsightWorker.hpp"

#include "ByteWidget.hpp"

namespace Bloom::Widgets
{
    class ByteWidgetContainer: public QWidget
    {
        Q_OBJECT

    public:
        std::optional<ByteWidget*> hoveredByteWidget;

        std::map<std::uint32_t, ByteWidget*> byteWidgetsByAddress;
        std::map<std::size_t, std::vector<ByteWidget*>> byteWidgetsByRowIndex;
        std::map<std::size_t, std::vector<ByteWidget*>> byteWidgetsByColumnIndex;

        ByteWidgetContainer(
            const Targets::TargetMemoryDescriptor& targetMemoryDescriptor,
            InsightWorker& insightWorker,
            QLabel* hoveredAddressLabel,
            QWidget* parent
        );

        void updateValues(const Targets::TargetMemoryBuffer& buffer);

    signals:
        void byteWidgetsAdjusted();

    protected:
        void resizeEvent(QResizeEvent* event) override;

    private:
        const Targets::TargetMemoryDescriptor& targetMemoryDescriptor;
        Targets::TargetState targetState = Targets::TargetState::UNKNOWN;
        InsightWorker& insightWorker;

        QWidget* parent = nullptr;
        QLabel* hoveredAddressLabel = nullptr;

        void adjustByteWidgets();

    private slots:
        void onTargetStateChanged(Targets::TargetState newState);
        void onByteWidgetEnter(Bloom::Widgets::ByteWidget* widget);
        void onByteWidgetLeave(Bloom::Widgets::ByteWidget* widget);
    };
}
