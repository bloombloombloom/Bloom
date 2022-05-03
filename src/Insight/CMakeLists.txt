target_sources(
    Bloom
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Insight.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/InsightWorker/InsightWorker.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/UiLoader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/BloomProxyStyle.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/InsightWindow.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/AboutWindow.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/PaneWidget.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/PanelWidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/Label.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/RotatableLabel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/LabeledSeparator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TextInput.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/SvgWidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/SvgToolButton.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/ClickableWidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/ExpandingHeightScrollAreaWidget.hpp

        # Insight worker tasks
        ${CMAKE_CURRENT_SOURCE_DIR}/InsightWorker/Tasks/InsightWorkerTask.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/InsightWorker/Tasks/ReadTargetRegisters.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/InsightWorker/Tasks/WriteTargetRegister.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/InsightWorker/Tasks/RefreshTargetPinStates.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/InsightWorker/Tasks/SetTargetPinState.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/InsightWorker/Tasks/ReadTargetMemory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/InsightWorker/Tasks/ReadStackPointer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/InsightWorker/Tasks/QueryLatestVersionNumber.cpp

        # Error dialogue window
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/ErrorDialogue/ErrorDialogue.cpp

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
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetRegistersPane/ItemWidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetRegistersPane/RegisterGroupWidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetRegistersPane/RegisterWidget.cpp

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
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/ByteItemContainerGraphicsView.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/ByteItemGraphicsScene.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/ByteItem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/ByteAddressContainer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/ByteAddressItem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/AnnotationItem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/ValueAnnotationItem.cpp

        # Memory region manager window
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemoryRegionManager/MemoryRegionManagerWindow.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemoryRegionManager/RegionItem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemoryRegionManager/FocusedRegionItem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemoryRegionManager/ExcludedRegionItem.cpp
)
