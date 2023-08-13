#pragma once

#include <QWidget>
#include <optional>
#include <QResizeEvent>
#include <QShowEvent>
#include <QVBoxLayout>
#include <QMap>
#include <QAction>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PaneWidget.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/SvgToolButton.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ListView/ListView.hpp"

#include "src/Targets/TargetState.hpp"
#include "src/Targets/TargetMemory.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemorySnapshot.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/FocusedMemoryRegion.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/ExcludedMemoryRegion.hpp"

#include "src/Insight/InsightWorker/Tasks/InsightWorkerTask.hpp"

#include "./CreateSnapshotWindow/CreateSnapshotWindow.hpp"
#include "MemorySnapshotItem.hpp"
#include "SnapshotViewer/SnapshotViewer.hpp"
#include "SnapshotDiff/SnapshotDiff.hpp"

namespace Widgets
{
    class SnapshotManager: public PaneWidget
    {
        Q_OBJECT

    public:
        CreateSnapshotWindow* createSnapshotWindow = nullptr;

        explicit SnapshotManager(
            const Targets::TargetMemoryDescriptor& memoryDescriptor,
            const std::optional<Targets::TargetMemoryBuffer>& data,
            const bool& staleData,
            const std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
            const std::vector<ExcludedMemoryRegion>& excludedMemoryRegions,
            const std::optional<Targets::TargetStackPointer>& stackPointer,
            PaneState& state,
            PanelWidget* parent = nullptr
        );

        void onCurrentDataChanged();

    signals:
        void insightWorkerTaskCreated(const QSharedPointer<InsightWorkerTask>& task);
        void snapshotRestored(const QString& snapshotId);

    protected:
        void resizeEvent(QResizeEvent* event) override;
        void showEvent(QShowEvent* event) override;

    private:
        const Targets::TargetMemoryDescriptor& memoryDescriptor;
        const std::optional<Targets::TargetMemoryBuffer>& data;
        const bool& staleData;

        const std::vector<FocusedMemoryRegion>& focusedMemoryRegions;
        const std::vector<ExcludedMemoryRegion>& excludedMemoryRegions;
        const std::optional<Targets::TargetStackPointer>& stackPointer;

        Targets::TargetState targetState = Targets::TargetState::UNKNOWN;

        QMap<QString, MemorySnapshot> snapshotsById;
        QMap<QString, MemorySnapshotItem*> snapshotItemsById;
        QMap<QString, SnapshotViewer*> snapshotViewersById;
        QMap<QString, SnapshotDiff*> snapshotDiffs;
        QMap<QString, SnapshotDiff*> snapshotCurrentDiffsBySnapshotAId;

        QWidget* container = nullptr;
        QWidget* toolBar = nullptr;

        SvgToolButton* createSnapshotButton = nullptr;
        SvgToolButton* deleteSnapshotButton = nullptr;

        ListView* snapshotListView = nullptr;
        ListScene* snapshotListScene = nullptr;

        std::list<MemorySnapshotItem*> selectedSnapshotItems;

        QAction* openSnapshotViewerAction = new QAction("Open", this);
        QAction* openSnapshotDiffAction = new QAction("Compare Selection", this);
        QAction* openSnapshotCurrentDiffAction = new QAction("Compare with Current", this);
        QAction* deleteSnapshotAction = new QAction("Delete", this);
        QAction* restoreSnapshotAction = new QAction("Restore", this);

        void createSnapshot(
            const QString& name,
            const QString& description,
            bool captureFocusedRegions,
            bool captureDirectlyFromTarget
        );

        void addSnapshot(MemorySnapshot&& snapshotTmp);
        void onSnapshotItemSelectionChanged(const std::list<ListItem*>& selectedItems);
        void openSnapshotViewer(const QString& snapshotId);
        void openSnapshotDiff(const QString& snapshotIdA, const QString& snapshotIdB);
        void openSnapshotCurrentDiff(const QString& snapshotIdA);
        void deleteSnapshot(const QString& snapshotId, bool confirmationPromptEnabled);
        void restoreSnapshot(const QString& snapshotId, bool confirmationPromptEnabled);
        void onSnapshotItemDoubleClick(MemorySnapshotItem* item);
        void onSnapshotItemContextMenu(ListItem* item, QPoint sourcePosition);
        void onTargetStateChanged(Targets::TargetState newState);
    };
}
