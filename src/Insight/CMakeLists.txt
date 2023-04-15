target_sources(
    Bloom
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Insight.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/InsightSignals.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/InsightWorker/InsightWorker.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/UiLoader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/BloomProxyStyle.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/InsightWindow.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/AboutWindow.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/PaneWidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/PanelWidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/Label.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/RotatableLabel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/LabeledSeparator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TextInput.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/SvgWidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/SvgToolButton.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/ClickableWidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/ExpandingHeightScrollAreaWidget.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/PushButton.cpp

        # ListView widget
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/ListView/ListView.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/ListView/ListScene.cpp

        # Insight worker tasks
        ${CMAKE_CURRENT_SOURCE_DIR}/InsightWorker/Tasks/InsightWorkerTask.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/InsightWorker/Tasks/ReadTargetRegisters.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/InsightWorker/Tasks/WriteTargetRegister.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/InsightWorker/Tasks/RefreshTargetPinStates.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/InsightWorker/Tasks/SetTargetPinState.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/InsightWorker/Tasks/ReadTargetMemory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/InsightWorker/Tasks/WriteTargetMemory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/InsightWorker/Tasks/ReadStackPointer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/InsightWorker/Tasks/ReadProgramCounter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/InsightWorker/Tasks/GetTargetState.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/InsightWorker/Tasks/GetTargetDescriptor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/InsightWorker/Tasks/ConstructHexViewerTopLevelGroupItem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/InsightWorker/Tasks/CaptureMemorySnapshot.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/InsightWorker/Tasks/RetrieveMemorySnapshots.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/InsightWorker/Tasks/DeleteMemorySnapshot.cpp

        # Task indicators
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TaskIndicator/TaskIndicator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TaskProgressIndicator/TaskProgressIndicator.cpp

        # Task window
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TaskWindow/TaskWindow.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TaskWindow/Task.cpp

        # Error dialogue window
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/ErrorDialogue/ErrorDialogue.cpp

        # Dialogue window
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/Dialog/Dialog.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/ConfirmationDialog.cpp

        # Target package widgets
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetWidgets/TargetPackageWidgetContainer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetWidgets/TargetPackageWidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetWidgets/TargetPinWidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetWidgets/TargetPinBodyWidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetWidgets/DIP/DualInlinePackageWidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetWidgets/DIP/PinWidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetWidgets/DIP/PinBodyWidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetWidgets/DIP/BodyWidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetWidgets/QFP/QuadFlatPackageWidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetWidgets/QFP/PinWidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetWidgets/QFP/PinBodyWidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetWidgets/QFP/BodyWidget.cpp

        # Target register side pane
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetRegistersPane/TargetRegistersPaneWidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetRegistersPane/RegisterGroupItem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetRegistersPane/RegisterItem.cpp

        # Target register inspector window
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetRegisterInspector/TargetRegisterInspectorWindow.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetRegisterInspector/RegisterHistoryWidget/RegisterHistoryWidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetRegisterInspector/RegisterHistoryWidget/Item.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetRegisterInspector/RegisterHistoryWidget/CurrentItem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetRegisterInspector/RegisterHistoryWidget/RegisterHistoryItem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetRegisterInspector/BitsetWidget/BitsetWidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetRegisterInspector/BitsetWidget/BitWidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetRegisterInspector/BitsetWidget/BitBodyWidget.cpp

        # Target memory inspection pane
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/TargetMemoryInspectionPane.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/HexViewerWidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/ItemGraphicsView.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/ItemGraphicsScene.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/HexViewerItem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/ByteItem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/GroupItem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/TopLevelGroupItem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/FocusedRegionGroupItem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/StackMemoryGroupItem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/ByteAddressContainer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/ByteAddressItem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/ContextMenuAction.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemoryRegion.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/FocusedMemoryRegion.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/ExcludedMemoryRegion.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemorySnapshot.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/SnapshotManager/SnapshotManager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/SnapshotManager/MemorySnapshotItem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/SnapshotManager/CreateSnapshotWindow/CreateSnapshotWindow.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/SnapshotManager/SnapshotViewer/SnapshotViewer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/SnapshotManager/SnapshotViewer/MemoryRegionItem.cpp

        # Memory region manager window
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemoryRegionManager/MemoryRegionManagerWindow.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemoryRegionManager/RegionItem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemoryRegionManager/FocusedRegionItem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemoryRegionManager/ExcludedRegionItem.cpp
)

# Insight resources
qt_add_resources(
    Bloom
    "InsightResources"
    PREFIX
        "/compiled/src/Insight"
    FILES
        # Insight Window
        "./UserInterfaces/InsightWindow/Stylesheets/Global.qss"
        "./UserInterfaces/InsightWindow/UiFiles/InsightWindow.ui"
        "./UserInterfaces/InsightWindow/Stylesheets/InsightWindow.qss"
        "./UserInterfaces/InsightWindow/Images/bloom-icon.svg"
        "./UserInterfaces/InsightWindow/Images/RAM.svg"
        "./UserInterfaces/InsightWindow/Images/refresh.svg"
        "./UserInterfaces/InsightWindow/Images/refresh-disabled.svg"
        "./UserInterfaces/InsightWindow/Images/down-arrow.svg"
        "./UserInterfaces/InsightWindow/Images/detach-pane-icon.svg"
        "./UserInterfaces/InsightWindow/Images/attach-pane-icon.svg"
        "./UserInterfaces/InsightWindow/Images/checked-action-icon.svg"
        "./UserInterfaces/InsightWindow/Images/unchecked-action-icon.svg"

        # Task window
        "./UserInterfaces/InsightWindow/Widgets/TaskWindow/UiFiles/TaskWindow.ui"
        "./UserInterfaces/InsightWindow/Widgets/TaskWindow/Stylesheets/TaskWindow.qss"

        # Target package widgets
        "./UserInterfaces/InsightWindow/Widgets/TargetWidgets/DIP/Stylesheets/DualInlinePackage.qss"
        "./UserInterfaces/InsightWindow/Widgets/TargetWidgets/QFP/Stylesheets/QuadFlatPackage.qss"

        # Target registers pane
        "./UserInterfaces/InsightWindow/Widgets/TargetRegistersPane/UiFiles/TargetRegistersSidePane.ui"
        "./UserInterfaces/InsightWindow/Widgets/TargetRegistersPane/UiFiles/TargetRegistersSidePane.ui"
        "./UserInterfaces/InsightWindow/Widgets/TargetRegistersPane/Images/collapse-all.svg"
        "./UserInterfaces/InsightWindow/Widgets/TargetRegistersPane/Images/expand-all.svg"
        "./UserInterfaces/InsightWindow/Widgets/TargetRegistersPane/Images/arrow.svg"
        "./UserInterfaces/InsightWindow/Widgets/TargetRegistersPane/Images/register-group.svg"
        "./UserInterfaces/InsightWindow/Widgets/TargetRegistersPane/Images/register.svg"
        "./UserInterfaces/InsightWindow/Widgets/TargetRegistersPane/Images/target-registers.svg"
        "./UserInterfaces/InsightWindow/Widgets/TargetRegistersPane/Images/target-registers-disabled.svg"
        "./UserInterfaces/InsightWindow/Widgets/TargetRegistersPane/Images/search-registers.svg"

        # Target register inspector window
        "./UserInterfaces/InsightWindow/Widgets/TargetRegisterInspector/UiFiles/TargetRegisterInspectorWindow.ui"
        "./UserInterfaces/InsightWindow/Widgets/TargetRegisterInspector/Stylesheets/TargetRegisterInspectorWindow.qss"
        "./UserInterfaces/InsightWindow/Widgets/TargetRegisterInspector/RegisterHistoryWidget/UiFiles/RegisterHistoryWidget.ui"
        "./UserInterfaces/InsightWindow/Widgets/TargetRegisterInspector/Images/icon.svg"

        # Memory inspection pane
        "./UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/UiFiles/TargetMemoryInspectionPane.ui"
        "./UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/Stylesheets/TargetMemoryInspectionPane.qss"
        "./UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/Images/memory-inspection-icon.svg"
        "./UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/Images/memory-inspection-icon-disabled.svg"
        "./UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/Images/manage-focused-regions.svg"
        "./UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/Images/stale-data-icon.svg"
        "./UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/Images/manage-snapshots-icon.svg"
        "./UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/Images/highlight-stack-memory.svg"
        "./UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/Images/highlight-stack-memory-disabled.svg"
        "./UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/Images/highlight-focused-memory.svg"
        "./UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/Images/highlight-focused-memory-disabled.svg"
        "./UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/Images/highlight-hovered-rows-columns.svg"
        "./UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/Images/highlight-hovered-rows-columns-disabled.svg"
        "./UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/Images/display-annotations.svg"
        "./UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/Images/display-annotations-disabled.svg"
        "./UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/Images/ascii-view.svg"
        "./UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/Images/ascii-view-disabled.svg"
        "./UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/SnapshotManager/Images/new-snapshot-icon.svg"

        # Hex viewer widget
        "./UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/UiFiles/HexViewerWidget.ui"
        "./UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/Stylesheets/HexViewerWidget.qss"

        # Memory snapshots
        "./UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/SnapshotManager/UiFiles/SnapshotManager.ui"
        "./UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/SnapshotManager/CreateSnapshotWindow/UiFiles/CreateSnapshotWindow.ui"
        "./UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/SnapshotManager/SnapshotViewer/UiFiles/SnapshotViewer.ui"
        "./UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/SnapshotManager/SnapshotViewer/Stylesheets/SnapshotViewer.qss"

        # Memory region manager window
        "./UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemoryRegionManager/UiFiles/MemoryRegionManagerWindow.ui"
        "./UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemoryRegionManager/Stylesheets/MemoryRegionManagerWindow.qss"
        "./UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemoryRegionManager/UiFiles/FocusedMemoryRegionForm.ui"
        "./UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemoryRegionManager/UiFiles/ExcludedMemoryRegionForm.ui"
        "./UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemoryRegionManager/Images/icon.svg"
        "./UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemoryRegionManager/Images/add-region.svg"
        "./UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemoryRegionManager/Images/remove-region.svg"

        # About window
        "./UserInterfaces/InsightWindow/UiFiles/AboutWindow.ui"
        "./UserInterfaces/InsightWindow/Stylesheets/AboutWindow.qss"

        # Error dialogue
        "./UserInterfaces/InsightWindow/Widgets/ErrorDialogue/UiFiles/ErrorDialogue.ui"
        "./UserInterfaces/InsightWindow/Widgets/ErrorDialogue/Stylesheets/ErrorDialogue.qss"
        "./UserInterfaces/InsightWindow/Widgets/ErrorDialogue/Images/icon.svg"

        # Dialogue
        "./UserInterfaces/InsightWindow/Widgets/Dialog/UiFiles/Dialog.ui"
        "./UserInterfaces/InsightWindow/Widgets/Dialog/Stylesheets/Dialog.qss"
        "./UserInterfaces/InsightWindow/Widgets/Dialog/Images/icon.svg"
)
