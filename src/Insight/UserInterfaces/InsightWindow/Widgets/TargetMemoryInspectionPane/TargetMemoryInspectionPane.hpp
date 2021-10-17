#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QScrollArea>
#include <set>
#include <QSize>
#include <QString>
#include <QEvent>
#include <optional>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PanelWidget.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/SvgToolButton.hpp"
#include "src/Insight/InsightWorker/InsightWorker.hpp"

#include "HexViewerWidget/HexViewerWidget.hpp"

#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetState.hpp"

namespace Bloom::Widgets
{
    class TargetMemoryInspectionPane: public QWidget
    {
        Q_OBJECT

    public:
        bool activated = false;

        TargetMemoryInspectionPane(
            const Targets::TargetMemoryDescriptor& targetMemoryDescriptor,
            InsightWorker& insightWorker,
            PanelWidget *parent
        );

        void refreshMemoryValues(std::optional<std::function<void(void)>> callback = std::nullopt);

        void activate();
        void deactivate();

    protected:
        void resizeEvent(QResizeEvent* event) override;

        virtual void postActivate();
        virtual void postDeactivate();

    private:
        const Targets::TargetMemoryDescriptor& targetMemoryDescriptor;
        InsightWorker& insightWorker;

        PanelWidget* parent = nullptr;
        QWidget* container = nullptr;

        QWidget* titleBar = nullptr;
        HexViewerWidget* hexViewerWidget = nullptr;

        Targets::TargetState targetState = Targets::TargetState::UNKNOWN;

    private slots:
        void onTargetStateChanged(Targets::TargetState newState);
        void onMemoryRead(const Targets::TargetMemoryBuffer& buffer);
    };
}
