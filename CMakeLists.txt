cmake_minimum_required(VERSION 3.10)
project(Bloom LANGUAGES CXX VERSION 0.1)

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

find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Widgets)
find_package(Qt5Xml)
find_package(Qt5Svg)
find_package(Qt5UiTools)

set(CMAKE_SKIP_BUILD_RPATH true)
set(CMAKE_BUILD_RPATH_USE_ORIGIN true)
set(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib")
set(CMAKE_BUILD_RPATH ${CMAKE_INSTALL_RPATH})

add_executable(Bloom
    src/main.cpp
    src/Generated/resources.cpp
    src/ApplicationConfig.cpp
    src/Logger/Logger.cpp
    src/SignalHandler/SignalHandler.cpp
    src/DebugToolDrivers/USB/Interface.cpp
    src/DebugToolDrivers/USB/HID/HidInterface.cpp

    src/DebugToolDrivers/Microchip/AtmelICE/AtmelIce.cpp
    src/DebugToolDrivers/Microchip/PowerDebugger/PowerDebugger.cpp
    src/DebugToolDrivers/Microchip/MplabSnap/MplabSnap.cpp

    src/DebugToolDrivers/Protocols/CMSIS-DAP/Command.cpp
    src/DebugToolDrivers/Protocols/CMSIS-DAP/Response.cpp
    src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/AvrCommand.cpp
    src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AvrCommandFrame.cpp
    src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/AvrResponse.cpp
    src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/ResponseFrames/AvrResponseFrame.cpp
    src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/AvrEvent.cpp
    src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/Events/AVR8Generic/BreakEvent.cpp
    src/DebugToolDrivers/Protocols/CMSIS-DAP/CmsisDapInterface.cpp
    src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/EdbgInterface.cpp
    src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/EdbgAvr8Interface.cpp
    src/Targets/Target.cpp
    src/Targets/Microchip/AVR/AVR8/Avr8.cpp
    src/Targets/Microchip/AVR/AVR8/Mega/Mega.cpp
    src/Targets/Microchip/AVR/AVR8/PartDescription/PartDescriptionFile.cpp
    src/Application.cpp
    src/DebugToolDrivers/USB/UsbDevice.cpp
    src/TargetController/TargetController.cpp
    src/EventManager/EventListener.cpp
    src/EventManager/EventManager.cpp
    src/DebugServers/DebugServer.cpp
    src/DebugServers/GdbRsp/GdbRspDebugServer.cpp
    src/DebugServers/GdbRsp/CommandPackets/CommandPacket.cpp
    src/DebugServers/GdbRsp/CommandPackets/CommandPacketFactory.cpp
    src/DebugServers/GdbRsp/CommandPackets/SupportedFeaturesQuery.cpp
    src/DebugServers/GdbRsp/CommandPackets/ReadGeneralRegisters.cpp
    src/DebugServers/GdbRsp/CommandPackets/WriteGeneralRegisters.cpp
    src/DebugServers/GdbRsp/CommandPackets/ContinueExecution.cpp
    src/DebugServers/GdbRsp/CommandPackets/StepExecution.cpp
    src/DebugServers/GdbRsp/CommandPackets/InterruptExecution.cpp
    src/DebugServers/GdbRsp/CommandPackets/ReadMemory.cpp
    src/DebugServers/GdbRsp/CommandPackets/WriteMemory.cpp
    src/DebugServers/GdbRsp/CommandPackets/SetBreakpoint.cpp
    src/DebugServers/GdbRsp/CommandPackets/RemoveBreakpoint.cpp
    src/DebugServers/GdbRsp/ResponsePackets/SupportedFeaturesResponse.cpp
    src/DebugServers/GdbRsp/Connection.cpp
    src/Insight/Insight.cpp
    src/Insight/InsightWorker.cpp
    src/Insight/UserInterfaces/InsightWindow/InsightWindow.cpp
    src/Insight/UserInterfaces/InsightWindow/AboutWindow.cpp
    src/Insight/UserInterfaces/InsightWindow/TargetWidgets/TargetPackageWidget.hpp
    src/Insight/UserInterfaces/InsightWindow/TargetWidgets/TargetPinWidget.hpp

    src/Insight/UserInterfaces/InsightWindow/TargetWidgets/DIP/DualInlinePackageWidget.cpp
    src/Insight/UserInterfaces/InsightWindow/TargetWidgets/DIP/PinWidget.cpp
    src/Insight/UserInterfaces/InsightWindow/TargetWidgets/DIP/PinBodyWidget.cpp
    src/Insight/UserInterfaces/InsightWindow/TargetWidgets/DIP/BodyWidget.cpp

    src/Insight/UserInterfaces/InsightWindow/TargetWidgets/QFP/QuadFlatPackageWidget.cpp
    src/Insight/UserInterfaces/InsightWindow/TargetWidgets/QFP/PinWidget.cpp
    src/Insight/UserInterfaces/InsightWindow/TargetWidgets/QFP/PinBodyWidget.cpp
    src/Insight/UserInterfaces/InsightWindow/TargetWidgets/QFP/BodyWidget.cpp
    build/resources/TargetPartDescriptions/AVR/Mapping.json
)

set_target_properties(Bloom PROPERTIES OUTPUT_NAME bloom)
target_include_directories(Bloom PUBLIC ./)

# Construct JSON mapping of part description files.
add_custom_command(
    OUTPUT
    ${CMAKE_CURRENT_SOURCE_DIR}/build/resources/TargetPartDescriptions/AVR/Mapping.json
    DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/build/scripts/CopyAvrPartFilesAndCreateMapping.php
    COMMAND echo 'Processing AVR target description files.'
    COMMAND
    php ${CMAKE_CURRENT_SOURCE_DIR}/build/scripts/CopyAvrPartFilesAndCreateMapping.php
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

target_link_libraries(Bloom -static-libgcc -static-libstdc++)
target_link_libraries(Bloom -lstdc++fs)
target_link_libraries(Bloom -lpthread)
target_link_libraries(Bloom -lusb-1.0)
target_link_libraries(Bloom -lhidapi-libusb)
target_link_libraries(Bloom Qt5::Core)
target_link_libraries(Bloom Qt5::Gui)
target_link_libraries(Bloom Qt5::UiTools)
target_link_libraries(Bloom Qt5::Widgets)
target_link_libraries(Bloom Qt5::Xml)
target_link_libraries(Bloom Qt5::Svg)

target_compile_options(
    Bloom
    PUBLIC -std=c++2a
    PUBLIC -pedantic
    PUBLIC -Wconversion
    PUBLIC -fno-sized-deallocation
    PUBLIC $<$<CONFIG:DEBUG>:-g>
    PUBLIC $<$<CONFIG:DEBUG>:-O0>
#    PUBLIC $<$<CONFIG:DEBUG>:-Os>
    PUBLIC $<$<CONFIG:RELEASE>:-O2>
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

install(FILES build/bin/bloom.json DESTINATION bin PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ) # Remove this
install(DIRECTORY build/bin/plugins DESTINATION "bin" DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)
install(DIRECTORY build/bin/platforms DESTINATION "bin" DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)
install(DIRECTORY build/resources DESTINATION "." DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)
install(DIRECTORY build/lib DESTINATION "." DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)

# Debian package configuration
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_NAME "Bloom")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "")
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/resources/packaging/description.txt CPACK_DEBIAN_PACKAGE_DESCRIPTION)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A Linux-based debug interface for embedded systems development")
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
