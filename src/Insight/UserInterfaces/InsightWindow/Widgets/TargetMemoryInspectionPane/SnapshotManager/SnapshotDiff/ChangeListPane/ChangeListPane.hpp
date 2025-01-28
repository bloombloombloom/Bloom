#pragma once

#include <QWidget>
#include <QResizeEvent>
#include <QShowEvent>
#include <QAction>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PaneWidget.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ListView/ListView.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/Label.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/SnapshotManager/SnapshotDiff/DifferentialHexViewerWidget/DifferentialHexViewerWidget.hpp"

#include "src/Targets/TargetMemoryAddressRange.hpp"

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
        void setRestoreEnabled(bool restoreEnabled);

    signals:
        void restoreBytesRequested(const std::set<Targets::TargetMemoryAddress>& addresses);

    protected:
        void resizeEvent(QResizeEvent* event) override;
        void showEvent(QShowEvent* event) override;

    private:
        DifferentialHexViewerWidget* hexViewerWidgetA;
        DifferentialHexViewerWidget* hexViewerWidgetB;

        QWidget* container = nullptr;
        Label* placeHolderLabel = nullptr;

        ListView* changeListView = nullptr;
        ListScene* changeListScene = nullptr;

        const ChangeListItem* selectedChangeListItem = nullptr;

        QAction* selectBytesAction = new QAction{"Select", this};
        QAction* restoreBytesAction = new QAction{"Restore", this};

        bool restoreEnabled = false;

        void onItemSelectionChanged(const std::list<ListItem*>& selectedItems);
        void onItemContextMenu(ListItem* item, QPoint sourcePosition);
        void refreshChangeListViewSize();
    };
}
