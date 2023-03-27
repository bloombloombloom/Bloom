#pragma once

#include <QWidget>
#include <optional>
#include <QResizeEvent>
#include <QShowEvent>
#include <QVBoxLayout>
#include <QHash>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PaneWidget.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/SvgToolButton.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ListView/ListView.hpp"

#include "src/Targets/TargetMemory.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemorySnapshot.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/FocusedMemoryRegion.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/ExcludedMemoryRegion.hpp"

#include "src/Insight/InsightWorker/Tasks/CaptureMemorySnapshot.hpp"

#include "./CreateSnapshotWindow/CreateSnapshotWindow.hpp"
#include "MemorySnapshotItem.hpp"

namespace Bloom::Widgets
{
    class SnapshotManager: public PaneWidget
    {
        Q_OBJECT

    public:
        CreateSnapshotWindow* createSnapshotWindow = nullptr;
        MemorySnapshotItem* selectedItem = nullptr;

        explicit SnapshotManager(
            const Targets::TargetMemoryDescriptor& memoryDescriptor,
            const std::optional<Targets::TargetMemoryBuffer>& data,
            const bool& staleData,
            const std::vector<FocusedMemoryRegion>& focusedMemoryRegions,
            const std::vector<ExcludedMemoryRegion>& excludedMemoryRegions,
            PaneState& state,
            PanelWidget* parent = nullptr
        );

    signals:
        void captureTaskCreated(const QSharedPointer<CaptureMemorySnapshot>& task);

    protected:
        void resizeEvent(QResizeEvent* event) override;
        void showEvent(QShowEvent* event) override;

    private:
        const Targets::TargetMemoryDescriptor& memoryDescriptor;
        const std::optional<Targets::TargetMemoryBuffer>& data;
        const bool& staleData;

        const std::vector<FocusedMemoryRegion>& focusedMemoryRegions;
        const std::vector<ExcludedMemoryRegion>& excludedMemoryRegions;

        QHash<QString, MemorySnapshot> snapshotsById;
        QHash<QString, MemorySnapshotItem*> snapshotItemsById;

        QWidget* container = nullptr;
        QWidget* toolBar = nullptr;

        SvgToolButton* createSnapshotButton = nullptr;
        SvgToolButton* deleteSnapshotButton = nullptr;

        ListView* snapshotListView = nullptr;
        ListScene* snapshotListScene = nullptr;

        void createSnapshot(
            const QString& name,
            const QString& description,
            bool captureFocusedRegions,
            bool captureDirectlyFromTarget
        );
        void addSnapshot(MemorySnapshot&& snapshotTmp);
        void onSnapshotItemSelected(MemorySnapshotItem* item);
    };
}
