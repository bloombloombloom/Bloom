#pragma once

#include <QWidget>
#include <QResizeEvent>
#include <QShowEvent>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PaneWidget.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ListView/ListView.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/SnapshotManager/SnapshotDiff/DifferentialHexViewerWidget/DifferentialHexViewerWidget.hpp"

#include "src/Targets/TargetMemory.hpp"

#include "ChangeListItem.hpp"

namespace Widgets
{
    class ChangeListPane: public PaneWidget
    {
        Q_OBJECT

    public:
        explicit ChangeListPane(
            DifferentialHexViewerWidget* hexViewerWidgetA,
            DifferentialHexViewerWidget* hexViewerWidgetB,
            PaneState& state,
            PanelWidget* parent = nullptr
        );

        void setDiffRanges(const std::vector<Targets::TargetMemoryAddressRange>& diffRanges);

    protected:
        void resizeEvent(QResizeEvent* event) override;
        void showEvent(QShowEvent* event) override;

    private:
        DifferentialHexViewerWidget* hexViewerWidgetA;
        DifferentialHexViewerWidget* hexViewerWidgetB;

        QWidget* container = nullptr;

        ListView* changeListView = nullptr;
        ListScene* changeListScene = nullptr;

        void onItemSelectionChanged(const std::list<ListItem*>& selectedItems);
        void refreshChangeListViewSize();
    };
}
