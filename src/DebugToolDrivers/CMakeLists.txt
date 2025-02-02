target_sources(
    Bloom
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Usb/UsbDevice.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Usb/UsbInterface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Usb/Hid/HidInterface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Protocols/CmsisDap/CmsisDapInterface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Protocols/CmsisDap/Command.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Protocols/CmsisDap/Response.cpp

        # Microchip EDBG implementation
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/Edbg/Avr/AvrCommand.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/Edbg/Avr/CommandFrames/Avr8Generic/ReadMemory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/Edbg/Avr/AvrResponse.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/Edbg/Avr/ResponseFrames/AvrResponseFrame.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/Edbg/Avr/ResponseFrames/Avr8Generic/Avr8GenericResponseFrame.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/Edbg/Avr/ResponseFrames/Avr8Generic/GetDeviceId.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/Edbg/Avr/ResponseFrames/Discovery/DiscoveryResponseFrame.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/Edbg/Avr/ResponseFrames/HouseKeeping/HouseKeepingResponseFrame.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/Edbg/Avr/ResponseFrames/AvrIsp/AvrIspResponseFrame.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/Edbg/Avr/ResponseFrames/EdbgControl/EdbgControlResponseFrame.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/Edbg/Avr/AvrEvent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/Edbg/Avr/Events/Avr8Generic/BreakEvent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/Edbg/Avr/Parameters/Avr8Generic/DebugWireJtagParameters.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/Edbg/Avr/Parameters/Avr8Generic/PdiParameters.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/Edbg/Avr/Parameters/Avr8Generic/UpdiParameters.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/Edbg/EdbgInterface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/Edbg/EdbgTargetPowerManagementInterface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/Edbg/Avr/EdbgAvr8Session.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/Edbg/Avr/EdbgAvr8Interface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Protocols/Edbg/Avr/EdbgAvrIspInterface.cpp

        # Microchip EDBG debug tools
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/EdbgDevice.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/EdbgToolConfig.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/AtmelIce/AtmelIce.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/PowerDebugger/PowerDebugger.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/MplabSnap/MplabSnap.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/MplabPickit4/MplabPickit4.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/XplainedPro/XplainedPro.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/XplainedMini/XplainedMini.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/XplainedNano/XplainedNano.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/CuriosityNano/CuriosityNano.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/JtagIce3/JtagIce3.cpp

        # WCH debug tools and interface implementations
        ${CMAKE_CURRENT_SOURCE_DIR}/Wch/Protocols/WchLink/WchLinkInterface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Wch/WchLinkDebugInterface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Wch/WchLinkToolConfig.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Wch/WchLinkBase.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Wch/WchLinkE/WchLinkE.cpp

        # RISC-V Debug Translator implementation
        ${CMAKE_CURRENT_SOURCE_DIR}/Protocols/RiscVDebug/DebugTranslatorConfig.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Protocols/RiscVDebug/DebugTranslator.cpp
)
