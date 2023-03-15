#pragma once

#include <QWidget>
#include <optional>
#include <QResizeEvent>
#include <QShowEvent>
#include <QScrollArea>
#include <QVBoxLayout>
#include <map>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PaneWidget.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/SvgToolButton.hpp"

#include "src/Targets/TargetMemory.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemorySnapshot.hpp"

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

        std::map<QString, MemorySnapshot> snapshotsById;

        QWidget* container = nullptr;
        QWidget* toolBar = nullptr;

        SvgToolButton* createSnapshotButton = nullptr;
        SvgToolButton* deleteSnapshotButton = nullptr;

        QScrollArea* itemScrollArea = nullptr;
        QWidget* itemScrollAreaViewport = nullptr;
        QVBoxLayout* itemLayout = nullptr;

        void createSnapshot(
            const QString& name,
            const QString& description,
            bool captureFocusedRegions,
            bool captureDirectlyFromTarget
        );
        void addSnapshot(MemorySnapshot&& snapshotTmp);
        void sortSnapshotItems();
        void onSnapshotItemSelected(MemorySnapshotItem* item);
    };
}
