#pragma once

#include <QWidget>
#include <QShowEvent>
#include <QResizeEvent>
#include <QHBoxLayout>
#include <optional>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TextInput.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PlainTextEdit.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PushButton.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ListView/ListView.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TaskProgressIndicator/TaskProgressIndicator.hpp"

#include "src/Targets/TargetMemory.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemorySnapshot.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/HexViewerWidget.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/ContextMenuAction.hpp"
#include "MemoryRegionItem.hpp"

namespace Widgets
{
    class SnapshotViewer: public QWidget
    {
        Q_OBJECT

    public:
        SnapshotViewer(
            MemorySnapshot& snapshot,
            const Targets::TargetMemoryDescriptor& memoryDescriptor,
            QWidget* parent = nullptr
        );

    protected:
        void showEvent(QShowEvent* event) override;
        void resizeEvent(QResizeEvent* event) override;

    private:
        MemorySnapshot& snapshot;
        const Targets::TargetMemoryDescriptor& memoryDescriptor;

        QWidget* container = nullptr;

        QWidget* detailsContainer = nullptr;
        TextInput* nameInput = nullptr;
        PlainTextEdit* descriptionInput = nullptr;

        ListView* memoryRegionListView = nullptr;
        ListScene* memoryRegionListScene = nullptr;
        std::vector<MemoryRegionItem*> memoryRegionItems;

        std::optional<Targets::TargetMemoryBuffer> hexViewerData;
        HexViewerWidget* hexViewerWidget = nullptr;
        HexViewerWidgetSettings hexViewerWidgetSettings = HexViewerWidgetSettings();

        ContextMenuAction* restoreBytesAction = nullptr;

        QWidget* bottomBar = nullptr;
        QHBoxLayout* bottomBarLayout = nullptr;

        TaskProgressIndicator* taskProgressIndicator = nullptr;

        void onHexViewerReady();
        void restoreSelectedBytes(
            const std::unordered_map<Targets::TargetMemoryAddress, ByteItem*>& selectedByteItemsByAddress,
            bool confirmationPromptEnabled
        );
    };
}
