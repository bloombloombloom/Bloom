target_sources(
    Bloom
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/TargetMemoryAddressRange.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/TargetDescriptor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/TargetAddressSpaceDescriptor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/TargetMemorySegmentDescriptor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/TargetPeripheralDescriptor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/TargetPeripheralSignalDescriptor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/TargetRegisterGroupDescriptor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/TargetRegisterDescriptor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/TargetBitFieldDescriptor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/TargetPadDescriptor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/TargetPinoutDescriptor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/TargetPinDescriptor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/TargetVariantDescriptor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/TargetMemoryCache.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/TargetPhysicalInterface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/DynamicRegisterValue.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/DeltaProgramming/Session.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/TargetDescription/TargetDescriptionFile.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Avr8/Avr8.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Avr8/Avr8TargetConfig.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Avr8/TargetDescriptionFile.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Avr8/OpcodeDecoder/Decoder.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Microchip/Avr8/IspParameters.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/RiscV/RiscV.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/RiscV/RiscVTargetConfig.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/RiscV/TargetDescriptionFile.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/RiscV/IsaDescriptor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/RiscV/Wch/WchRiscV.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/RiscV/Wch/WchRiscVTargetConfig.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/RiscV/Wch/TargetDescriptionFile.cpp
)
