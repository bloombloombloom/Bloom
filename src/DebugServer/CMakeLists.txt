target_sources(
    Bloom
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/DebugServerComponent.cpp

        # GDB RSP Server
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/GdbDebugServerConfig.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/Connection.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/DebugSession.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/ResponsePackets/ResponsePacket.cpp
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
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/CommandPackets/Detach.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/CommandPackets/ListRegistersMonitor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/CommandPackets/ReadRegistersMonitor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/CommandPackets/WriteRegisterMonitor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/CommandPackets/VContContinueExecution.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/CommandPackets/VContStepExecution.cpp

        # AVR GDB RSP Server
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/AvrGdb/AvrGdbRsp.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/AvrGdb/AvrGdbTargetDescriptor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/AvrGdb/CommandPackets/ReadRegisters.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/AvrGdb/CommandPackets/ReadRegister.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/AvrGdb/CommandPackets/WriteRegister.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/AvrGdb/CommandPackets/ReadMemory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/AvrGdb/CommandPackets/WriteMemory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/AvrGdb/CommandPackets/ReadMemoryMap.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/AvrGdb/CommandPackets/FlashErase.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/AvrGdb/CommandPackets/FlashWrite.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/AvrGdb/CommandPackets/FlashDone.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/AvrGdb/CommandPackets/VContSupportedActionsQuery.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/AvrGdb/CommandPackets/VContRangeStep.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/AvrGdb/CommandPackets/EepromFill.cpp

        # RISC-V GDB RSP Server
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/RiscVGdb/RiscVGdbRsp.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/RiscVGdb/RiscVGdbTargetDescriptor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/RiscVGdb/CommandPackets/ReadRegisters.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/RiscVGdb/CommandPackets/ReadRegister.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/RiscVGdb/CommandPackets/WriteRegister.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/RiscVGdb/CommandPackets/ReadMemory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/RiscVGdb/CommandPackets/WriteMemory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/RiscVGdb/CommandPackets/ReadMemoryMap.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/RiscVGdb/CommandPackets/FlashErase.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/RiscVGdb/CommandPackets/FlashWrite.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/RiscVGdb/CommandPackets/FlashDone.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/RiscVGdb/CommandPackets/VContSupportedActionsQuery.cpp
)

if (NOT EXCLUDE_INSIGHT)
    target_sources(
        Bloom
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/Gdb/CommandPackets/ActivateInsight.cpp
    )
endif ()
