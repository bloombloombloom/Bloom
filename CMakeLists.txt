cmake_minimum_required(VERSION 3.12)
project(Bloom LANGUAGES CXX VERSION 0.4.2)

set(CMAKE_VERBOSE_MAKEFILE off)

# Create directory for generated sources
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/Generated)

set(CMAKE_CXX_STANDARD 20)
set(ENABLE_SANITIZERS off)

add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

# The HIDAPI library lives here
link_directories(/usr/local/lib)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(AUTOGEN_BUILD_DIR ../build/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build/bin")

find_package(Qt6Core)
find_package(Qt6Gui)
find_package(Qt6Widgets)
find_package(Qt6Xml)
find_package(Qt6Svg)
find_package(Qt6UiTools)
find_package(Qt6SvgWidgets)

set(CMAKE_SKIP_BUILD_RPATH false)
set(CMAKE_BUILD_RPATH_USE_ORIGIN true)
set(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib")
set(CMAKE_BUILD_RPATH ${CMAKE_INSTALL_RPATH})

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_compile_definitions(BLOOM_DEBUG_BUILD)

    # BLOOM_COMPILED_RESOURCES_PATH_OVERRIDE can be used to override the file path used for compiled resources.
    # We override this path in debug builds to avoid using compiled resources. This makes debugging and small tweaks
    # a lot easier, as it removes the need to recompile for each tweak.
    # CAUTION: Although convenient, this does add a limitation; the debug build can only be run on the same machine
    # that compiled it. Or a machine that has the Bloom source located in the same place.
    # See Paths::compiledResourcesPath() for more.
    add_compile_definitions(BLOOM_COMPILED_RESOURCES_PATH_OVERRIDE="${CMAKE_CURRENT_SOURCE_DIR}")

    # CMAKE_SKIP_BUILD_RPATH needs to be set to true to use Gammaray during development.
    # This is because the distributed Qt binaries may not be compatible with the local installation of Gammaray
    set(CMAKE_BUILD_RPATH /opt/Qt/6.1.2/gcc_64/lib/)
endif()

add_executable(Bloom
    src/main.cpp
    src/Application.cpp

    # Helpers & other
    src/Logger/Logger.cpp
    src/Helpers/Paths.cpp
    src/Generated/resources.cpp

    # Project & application configuration
    src/ApplicationConfig.cpp

    # Events
    src/EventManager/EventListener.cpp
    src/EventManager/EventManager.cpp

    # Signal handler
    src/SignalHandler/SignalHandler.cpp

    # Target controller
    src/TargetController/TargetController.cpp
    src/TargetController/TargetControllerConsole.cpp

    # Debug tool drivers
    src/DebugToolDrivers/USB/UsbDevice.cpp
    src/DebugToolDrivers/USB/Interface.cpp
    src/DebugToolDrivers/USB/HID/HidInterface.cpp
    src/DebugToolDrivers/Microchip/AtmelICE/AtmelIce.cpp
    src/DebugToolDrivers/Microchip/PowerDebugger/PowerDebugger.cpp
    src/DebugToolDrivers/Microchip/MplabSnap/MplabSnap.cpp
    src/DebugToolDrivers/Protocols/CMSIS-DAP/Command.cpp
    src/DebugToolDrivers/Protocols/CMSIS-DAP/Response.cpp
    src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/AvrCommand.cpp
    src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AvrCommandFrame.cpp
    src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AVR8Generic/ReadMemory.cpp
    src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/AvrResponse.cpp
    src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/ResponseFrames/AvrResponseFrame.cpp
    src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/AvrEvent.cpp
    src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/Events/AVR8Generic/BreakEvent.cpp
    src/DebugToolDrivers/Protocols/CMSIS-DAP/CmsisDapInterface.cpp
    src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/EdbgInterface.cpp
    src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/EdbgAvr8Interface.cpp

    # Targets
    src/Targets/TargetDescription/TargetDescriptionFile.cpp
    src/Targets/TargetRegister.cpp
    src/Targets/Microchip/AVR/AVR8/Avr8.cpp
    src/Targets/Microchip/AVR/AVR8/TargetDescription/TargetDescriptionFile.cpp
    build/resources/TargetDescriptionFiles/AVR/Mapping.json

    # Debug servers
    src/DebugServers/DebugServer.cpp
    src/DebugServers/GdbRsp/GdbRspDebugServer.cpp
    src/DebugServers/GdbRsp/AvrGdbRsp/AvrGdbRsp.cpp
    src/DebugServers/GdbRsp/Connection.cpp
    src/DebugServers/GdbRsp/CommandPackets/CommandPacket.cpp
    src/DebugServers/GdbRsp/CommandPackets/CommandPacketFactory.cpp
    src/DebugServers/GdbRsp/CommandPackets/SupportedFeaturesQuery.cpp
    src/DebugServers/GdbRsp/CommandPackets/ReadRegisters.cpp
    src/DebugServers/GdbRsp/CommandPackets/WriteRegister.cpp
    src/DebugServers/GdbRsp/CommandPackets/ContinueExecution.cpp
    src/DebugServers/GdbRsp/CommandPackets/StepExecution.cpp
    src/DebugServers/GdbRsp/CommandPackets/InterruptExecution.cpp
    src/DebugServers/GdbRsp/CommandPackets/ReadMemory.cpp
    src/DebugServers/GdbRsp/CommandPackets/WriteMemory.cpp
    src/DebugServers/GdbRsp/CommandPackets/SetBreakpoint.cpp
    src/DebugServers/GdbRsp/CommandPackets/RemoveBreakpoint.cpp
    src/DebugServers/GdbRsp/ResponsePackets/SupportedFeaturesResponse.cpp

    # Insight
    src/Insight/Insight.cpp
    src/Insight/InsightWorker/InsightWorker.cpp
    src/Insight/UserInterfaces/InsightWindow/UiLoader.cpp
    src/Insight/UserInterfaces/InsightWindow/InsightWindow.cpp
    src/Insight/UserInterfaces/InsightWindow/AboutWindow.cpp
    src/Insight/UserInterfaces/InsightWindow/Widgets/PanelWidget.cpp
    src/Insight/UserInterfaces/InsightWindow/Widgets/RotatableLabel.cpp
    src/Insight/UserInterfaces/InsightWindow/Widgets/SvgWidget.cpp
    src/Insight/UserInterfaces/InsightWindow/Widgets/SvgToolButton.hpp
    src/Insight/UserInterfaces/InsightWindow/Widgets/ClickableWidget.cpp
    src/Insight/UserInterfaces/InsightWindow/Widgets/ExpandingHeightScrollAreaWidget.hpp

    # Insight worker tasks
    src/Insight/InsightWorker/Tasks/InsightWorkerTask.cpp
    src/Insight/InsightWorker/Tasks/ReadTargetRegisters.cpp
    src/Insight/InsightWorker/Tasks/WriteTargetRegister.cpp
    src/Insight/InsightWorker/Tasks/RefreshTargetPinStates.cpp
    src/Insight/InsightWorker/Tasks/SetTargetPinState.cpp
    src/Insight/InsightWorker/Tasks/ReadTargetMemory.cpp

    # Error dialogue window
    src/Insight/UserInterfaces/InsightWindow/Widgets/ErrorDialogue/ErrorDialogue.cpp

    # Target package widgets
    src/Insight/UserInterfaces/InsightWindow/Widgets/TargetWidgets/TargetPackageWidgetContainer.cpp
    src/Insight/UserInterfaces/InsightWindow/Widgets/TargetWidgets/TargetPackageWidget.cpp
    src/Insight/UserInterfaces/InsightWindow/Widgets/TargetWidgets/TargetPinWidget.cpp
    src/Insight/UserInterfaces/InsightWindow/Widgets/TargetWidgets/TargetPinBodyWidget.cpp
    src/Insight/UserInterfaces/InsightWindow/Widgets/TargetWidgets/DIP/DualInlinePackageWidget.cpp
    src/Insight/UserInterfaces/InsightWindow/Widgets/TargetWidgets/DIP/PinWidget.cpp
    src/Insight/UserInterfaces/InsightWindow/Widgets/TargetWidgets/DIP/PinBodyWidget.cpp
    src/Insight/UserInterfaces/InsightWindow/Widgets/TargetWidgets/DIP/BodyWidget.cpp
    src/Insight/UserInterfaces/InsightWindow/Widgets/TargetWidgets/QFP/QuadFlatPackageWidget.cpp
    src/Insight/UserInterfaces/InsightWindow/Widgets/TargetWidgets/QFP/PinWidget.cpp
    src/Insight/UserInterfaces/InsightWindow/Widgets/TargetWidgets/QFP/PinBodyWidget.cpp
    src/Insight/UserInterfaces/InsightWindow/Widgets/TargetWidgets/QFP/BodyWidget.cpp

    # Target register side pane
    src/Insight/UserInterfaces/InsightWindow/Widgets/TargetRegistersPane/TargetRegistersPaneWidget.cpp
    src/Insight/UserInterfaces/InsightWindow/Widgets/TargetRegistersPane/ItemWidget.cpp
    src/Insight/UserInterfaces/InsightWindow/Widgets/TargetRegistersPane/RegisterGroupWidget.cpp
    src/Insight/UserInterfaces/InsightWindow/Widgets/TargetRegistersPane/RegisterWidget.cpp

    # Target register inspector window
    src/Insight/UserInterfaces/InsightWindow/Widgets/TargetRegisterInspector/TargetRegisterInspectorWindow.cpp
    src/Insight/UserInterfaces/InsightWindow/Widgets/TargetRegisterInspector/RegisterHistoryWidget/RegisterHistoryWidget.cpp
    src/Insight/UserInterfaces/InsightWindow/Widgets/TargetRegisterInspector/RegisterHistoryWidget/Item.cpp
    src/Insight/UserInterfaces/InsightWindow/Widgets/TargetRegisterInspector/RegisterHistoryWidget/CurrentItem.cpp
    src/Insight/UserInterfaces/InsightWindow/Widgets/TargetRegisterInspector/RegisterHistoryWidget/RegisterHistoryItem.cpp
    src/Insight/UserInterfaces/InsightWindow/Widgets/TargetRegisterInspector/BitsetWidget/BitsetWidget.cpp
    src/Insight/UserInterfaces/InsightWindow/Widgets/TargetRegisterInspector/BitsetWidget/BitWidget.cpp
    src/Insight/UserInterfaces/InsightWindow/Widgets/TargetRegisterInspector/BitsetWidget/BitBodyWidget.cpp

    # Target memory inspection pane
    src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/TargetMemoryInspectionPane.cpp
    src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/HexViewerWidget.cpp
    src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/ByteWidgetContainer.cpp
    src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/HexViewerWidget/ByteWidget.cpp
)

set_target_properties(Bloom PROPERTIES OUTPUT_NAME bloom)
target_include_directories(Bloom PUBLIC ./)

# Copy AVR8 TDFs to build directory and construct JSON mapping of AVR8 target signatures to TDF paths.
add_custom_command(
    OUTPUT
    ${CMAKE_CURRENT_SOURCE_DIR}/build/resources/TargetDescriptionFiles/AVR/Mapping.json
    DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/build/scripts/Avr8TargetDescriptionFiles.php
    COMMAND echo 'Processing AVR target description files.'
    COMMAND
    php ${CMAKE_CURRENT_SOURCE_DIR}/build/scripts/Avr8TargetDescriptionFiles.php
)

# Compile resources
add_custom_command(
    OUTPUT
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Generated/resources.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Generated/resources_fake.cpp
    DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources.qrc
    COMMAND echo 'Compiling QT resources. |${CMAKE_BUILD_TYPE}|'
    COMMAND
    rcc -o ${CMAKE_CURRENT_SOURCE_DIR}/src/Generated/resources.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources.qrc
)

# Copy resources/fonts into build/resources/Fonts
add_custom_command(
    TARGET Bloom
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/fonts
    ${CMAKE_CURRENT_SOURCE_DIR}/build/resources/Fonts
)

# Copy resources/udevrules/99-bloom.rules to build/resources/UDevRules/99-bloom.rules
add_custom_command(
    TARGET Bloom
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/udevrules/99-bloom.rules
    ${CMAKE_CURRENT_SOURCE_DIR}/build/resources/UDevRules/99-bloom.rules
)

target_link_libraries(Bloom -static-libgcc -static-libstdc++)
target_link_libraries(Bloom -lstdc++fs)
target_link_libraries(Bloom -lpthread)
target_link_libraries(Bloom -lusb-1.0)
target_link_libraries(Bloom -lhidapi-libusb)
target_link_libraries(Bloom Qt6::Core)
target_link_libraries(Bloom Qt6::Gui)
target_link_libraries(Bloom Qt6::UiTools)
target_link_libraries(Bloom Qt6::Widgets)
target_link_libraries(Bloom Qt6::Xml)
target_link_libraries(Bloom Qt6::Svg)
target_link_libraries(Bloom Qt6::SvgWidgets)

target_compile_options(
    Bloom
    PUBLIC -std=c++2a
    PUBLIC -pedantic
    PUBLIC -Wconversion
    PUBLIC -fno-sized-deallocation
    PUBLIC $<$<CONFIG:DEBUG>:-g>
    PUBLIC $<$<CONFIG:DEBUG>:-O0>
#    PUBLIC $<$<CONFIG:DEBUG>:-Os>
    PUBLIC $<$<CONFIG:RELEASE>:-Ofast>
    PUBLIC $<$<CONFIG:DEBUG>:-fno-inline>
    PUBLIC $<$<CONFIG:DEBUG>:-fkeep-static-functions>
)

target_link_options(
    Bloom
    PUBLIC [=[-Wl,--disable-new-dtags]=] #,--verbose
)

if (${ENABLE_SANITIZERS})
    message(WARNING "Sanitizers have been enabled")

    # For TSAN, see ThreadSanitizerSuppression.txt

    # Some sanitizers are not compatible with each other.
    target_compile_options(
        Bloom
        PUBLIC "-fsanitize=address"
        #PUBLIC "-fsanitize=undefined"
#        PUBLIC "-fsanitize=thread"
#        PUBLIC "$<$<BOOL:${ENABLE_SANITIZERS}>:-fsanitize=address>"
#        PUBLIC "$<$<BOOL:${ENABLE_SANITIZERS}>:-fsanitize=undefined>"
#        PUBLIC "$<$<BOOL:${ENABLE_SANITIZERS}>:-fsanitize=integer-divide-by-zero>"
#        PUBLIC "$<$<BOOL:${ENABLE_SANITIZERS}>:-fsanitize=unreachable>"
#        PUBLIC "$<$<BOOL:${ENABLE_SANITIZERS}>:-fsanitize=vla-bound>"
#        PUBLIC "$<$<BOOL:${ENABLE_SANITIZERS}>:-fsanitize=null>"
#        PUBLIC "$<$<BOOL:${ENABLE_SANITIZERS}>:-fsanitize=return>"
#        PUBLIC "$<$<BOOL:${ENABLE_SANITIZERS}>:-fsanitize=signed-integer-overflow>"
#        PUBLIC "$<$<BOOL:${ENABLE_SANITIZERS}>:-fsanitize=bounds>"
#        PUBLIC "$<$<BOOL:${ENABLE_SANITIZERS}>:-fsanitize=alignment>"
#        PUBLIC "$<$<BOOL:${ENABLE_SANITIZERS}>:-fsanitize=object-size>"
#        PUBLIC "$<$<BOOL:${ENABLE_SANITIZERS}>:-fsanitize=float-divide-by-zero>"
#        PUBLIC "$<$<BOOL:${ENABLE_SANITIZERS}>:-fsanitize=float-cast-overflow>"
#        PUBLIC "$<$<BOOL:${ENABLE_SANITIZERS}>:-fsanitize=nonnull-attribute>"
#        PUBLIC "$<$<BOOL:${ENABLE_SANITIZERS}>:-fsanitize=returns-nonnull-attribute>"
#        PUBLIC "$<$<BOOL:${ENABLE_SANITIZERS}>:-fsanitize=bool>"
#        PUBLIC "$<$<BOOL:${ENABLE_SANITIZERS}>:-fsanitize=enum>"
#        PUBLIC "$<$<BOOL:${ENABLE_SANITIZERS}>:-fsanitize=vptr>"
    )

    target_link_libraries(
        Bloom
        "-fsanitize=address"
#        "-fsanitize=undefined"
#        "-fsanitize=thread"
    )
endif()

# Installation configuration
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/release/")
install(TARGETS Bloom DESTINATION bin PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)

install(DIRECTORY build/bin/plugins DESTINATION "bin" DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)
install(DIRECTORY build/bin/platforms DESTINATION "bin" DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)
install(DIRECTORY build/resources DESTINATION "." DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)
install(DIRECTORY build/lib DESTINATION "." DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)

# Debian package configuration
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_NAME "Bloom")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "")
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/resources/packaging/description.txt CPACK_DEBIAN_PACKAGE_DESCRIPTION)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A debug interface for embedded systems development on Linux")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Bloom Support <support@bloom.oscillate.io>")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://bloom.oscillate.io")
set(CPACK_PACKAGE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_PACKAGING_INSTALL_PREFIX "/opt/bloom")
set(
    CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/packaging/postinst;
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/packaging/postrm
)

include(CPack)
