target_sources(
    Bloom
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/DebugServerComponent.cpp

        # GDB RSP Server
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/GdbRspDebugServer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/GdbDebugServerConfig.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/Connection.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/DebugSession.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/TargetDescriptor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/ResponsePackets/SupportedFeaturesResponse.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/CommandPackets/CommandPacket.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/CommandPackets/SupportedFeaturesQuery.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/CommandPackets/ContinueExecution.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/CommandPackets/StepExecution.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/CommandPackets/InterruptExecution.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/CommandPackets/SetBreakpoint.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/CommandPackets/RemoveBreakpoint.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/CommandPackets/Monitor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/CommandPackets/ResetTarget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/CommandPackets/HelpMonitorInfo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/CommandPackets/BloomVersion.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/CommandPackets/BloomVersionMachine.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/CommandPackets/GenerateSvd.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/CommandPackets/Detach.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/CommandPackets/EepromFill.cpp

        # AVR GDB RSP Server
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/AvrGdb/AvrGdbRsp.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/AvrGdb/TargetDescriptor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/AvrGdb/CommandPackets/ReadRegisters.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/AvrGdb/CommandPackets/ReadRegister.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/AvrGdb/CommandPackets/WriteRegister.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/AvrGdb/CommandPackets/ReadMemory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/AvrGdb/CommandPackets/WriteMemory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/AvrGdb/CommandPackets/ReadMemoryMap.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/AvrGdb/CommandPackets/FlashErase.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/AvrGdb/CommandPackets/FlashWrite.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/AvrGdb/CommandPackets/FlashDone.cpp
)

if (NOT EXCLUDE_INSIGHT)
    target_sources(
        Bloom
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/CommandPackets/ActivateInsight.cpp
    )

    set(
        ACTIVATE_INSIGHT_HELP_TEXT
        "\n  insight               Activates the Insight GUI.\n"
    )
endif ()

configure_file(
    "./Gdb/Resources/GdbHelpMonitorInfo.txt.in"
    "${COMPILED_RESOURCES_BUILD_DIR}/Gdb/GdbHelpMonitorInfo.txt"
    @ONLY
)

set_source_files_properties(
    "${COMPILED_RESOURCES_BUILD_DIR}/Gdb/GdbHelpMonitorInfo.txt"
    PROPERTIES QT_RESOURCE_ALIAS "/Gdb/Resources/GdbHelpMonitorInfo.txt"
)

# DebugServer resources
qt_add_resources(
    Bloom
    "DebugServerResources"
    PREFIX
        "/compiled/src/DebugServer"
    FILES
        # GDB RSP Server
        "${COMPILED_RESOURCES_BUILD_DIR}/Gdb/GdbHelpMonitorInfo.txt"
)
