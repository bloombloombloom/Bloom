target_sources(
    Bloom
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/USB/UsbDevice.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/USB/UsbInterface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/USB/HID/HidInterface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Protocols/CMSIS-DAP/CmsisDapInterface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Protocols/CMSIS-DAP/Command.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Protocols/CMSIS-DAP/Response.cpp

        # Microchip EDBG implementation
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/EDBG/AVR/AvrCommand.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/EDBG/AVR/CommandFrames/AVR8Generic/ReadMemory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/EDBG/AVR/AvrResponse.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/EDBG/AVR/ResponseFrames/AvrResponseFrame.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/EDBG/AVR/ResponseFrames/AVR8Generic/Avr8GenericResponseFrame.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/EDBG/AVR/ResponseFrames/AVR8Generic/GetDeviceId.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/EDBG/AVR/ResponseFrames/Discovery/DiscoveryResponseFrame.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/EDBG/AVR/ResponseFrames/HouseKeeping/HouseKeepingResponseFrame.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/EDBG/AVR/ResponseFrames/AVRISP/AvrIspResponseFrame.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/EDBG/AVR/ResponseFrames/EDBGControl/EdbgControlResponseFrame.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/EDBG/AVR/AvrEvent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/EDBG/AVR/Events/AVR8Generic/BreakEvent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/EDBG/AVR/Parameters/AVR8Generic/DebugWireJtagParameters.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/EDBG/AVR/Parameters/AVR8Generic/PdiParameters.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/EDBG/AVR/Parameters/AVR8Generic/UpdiParameters.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/EDBG/EdbgInterface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/EDBG/EdbgTargetPowerManagementInterface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/EDBG/AVR/EdbgAvr8Session.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/EDBG/AVR/EdbgAvr8Interface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/EDBG/AVR/EdbgAvrIspInterface.cpp

        # Microchip EDBG debug tools
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/EdbgDevice.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/EdbgToolConfig.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/AtmelICE/AtmelIce.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/PowerDebugger/PowerDebugger.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/MplabSnap/MplabSnap.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/MplabPickit4/MplabPickit4.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/XplainedPro/XplainedPro.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/XplainedMini/XplainedMini.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/XplainedNano/XplainedNano.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/CuriosityNano/CuriosityNano.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/JtagIce3/JtagIce3.cpp

        # WCH debug tools and interface implementations
        ${CMAKE_CURRENT_SOURCE_DIR}/WCH/Protocols/WchLink/WchLinkInterface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/WCH/WchLinkDebugInterface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/WCH/WchLinkToolConfig.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/WCH/WchLinkBase.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/WCH/WchLinkE/WchLinkE.cpp

        # RISC-V Debug Translator implementation
        ${CMAKE_CURRENT_SOURCE_DIR}/Protocols/RiscVDebugSpec/DebugTranslatorConfig.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Protocols/RiscVDebugSpec/DebugTranslator.cpp
)
