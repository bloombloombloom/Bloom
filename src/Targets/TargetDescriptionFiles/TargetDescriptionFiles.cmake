# We maintain a list of all TDF files here
# We use this list to specify all TDF files as dependencies to some custom commands, so that CMake will run the
# commands whenever any TDF is modified. See the custom commands defined in the root CMakeLists.txt for more.
list(
    APPEND
    TDF_FILES_LIST
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY406.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY4313.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY2313A.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY1614.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY261.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY261A.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY861.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY441.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY212.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY13.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY806.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY45.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY202.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY1627.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY24A.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY461.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY804.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY1604.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY1617.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY1626.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY404.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY1607.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY841.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY13A.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY84A.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY814.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY24.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY2313.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY461A.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY167.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY44.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY1634.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY414.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY43U.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY1624.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY48.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY84.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY417.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY3216.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY214.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY828.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY807.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY3217.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY85.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY1616.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY416.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY44A.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY816.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY1606.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY861A.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY402.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY87.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY817.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY204.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY25.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY88.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/TINY/ATTINY412.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA192A3.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA64B1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA128C3.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA16A4.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA256A3.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA192C3.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA16C4.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA256A3U.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA32D4.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA256D3.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA16A4U.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA64A1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA128A1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA32C4.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA128B1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA256C3.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA128A4U.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA64D3.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA64A4U.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA64A1U.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA384D3.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA16D4.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA32C3.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA128D3.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA32E5.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA192D3.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA64B3.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA128A3.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA64C3.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA32A4U.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA128A3U.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA128D4.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA256A3BU.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA64A3U.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA32A4.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA128B3.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA16E5.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA64A3.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA192A3U.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA8E5.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA256A3B.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA32D3.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA64D4.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA384C3.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/XMEGA/ATXMEGA128A1U.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR16DD32.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR128DA28.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR32DD28.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR64DA48.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR16DD14.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR64DB64.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR64DB48.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR128DA32.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR32DA28.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR32DB28.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR64DD14.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR64DA28.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR128DA48.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR32DB32.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR64DB28.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR16DD20.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR64DB32.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR32DA32.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR64DA32.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR32DD32.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR128DB48.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR64DD28.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR32DD20.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR64DA64.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR64DD20.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR32DB48.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR128DB64.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR32DD14.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR128DB32.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR32DA48.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR128DA64.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR128DB28.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR64DD32.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/D-SERIES/AVR16DD28.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/E-SERIES/AVR64EA28.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/E-SERIES/AVR64EA48.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/E-SERIES/AVR64EA32.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/AT90USB1287.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA406.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA325P.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/AT90CAN128.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA88.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA164PA.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA88P.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA48A.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA16A.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA1284.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA256RFR2.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA645.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA32C1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/AT90PWM216.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA169A.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA16.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/AT90USB1286.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA64RFR2.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA4808.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA325.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA168PA.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA32U4.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/AT90USB646.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA6490.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA644A.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA1280.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/AT90CAN32.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA649A.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA644RFR2.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA64C1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA169P.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/AT90PWM1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA6450.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA128.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA3290P.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA3250A.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA88A.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA128RFA1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA1284RFR2.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/AT90PWM316.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA164A.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA16U2.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA645A.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA2564RFR2.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA48PB.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA329P.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA162.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA4809.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA165PA.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA32M1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA324P.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/AT90PWM161.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA16U4.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA325A.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA3250PA.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA32A.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA324A.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA88PA.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA88PB.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA329.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA16M1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA6450A.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA168.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA32.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA2560.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA324PA.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA1608.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA64A.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA48PA.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA168A.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA64.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA649P.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA328P.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/AT90USB162.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/AT90CAN64.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA324PB.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA164P.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA328.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA6490A.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/AT90PWM2B.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA64M1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA1609.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA6490P.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/AT90PWM3B.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA128RFR2.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA649.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA48P.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA2561.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA644PA.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA168P.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA1281.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA48.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA165P.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/AT90PWM81.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA169PA.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/AT90USB647.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA165A.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA644P.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA3209.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/AT90USB82.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA808.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA3290A.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA3250P.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA3250.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA6450P.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA645P.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA3290.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA8U2.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA644.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA168PB.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA1284P.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA640.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA128A.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA329PA.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA325PA.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA3208.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA329A.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA32U2.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA3290PA.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA328PB.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/AVR8/MEGA/ATMEGA809.xml
)

# Because the TDF_FILES_LIST is only used to specify dependencies for some custom commands, there is nothing enforcing
# the maintenance of the list. It will be difficult to notice when the list goes out of sync.
#
# For this reason, we enforce maintenance by comparing the length of TDF_FILES_LIST with a generated list of discovered
# TDF files, and trigger a fatal error if they're out of sync.
file(
    GLOB_RECURSE
    DISCOVERED_TDF_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/*.xml
    LIST_DIRECTORIES false
)
list(LENGTH DISCOVERED_TDF_FILES DISCOVERED_TDF_FILE_COUNT)
list(LENGTH TDF_FILES_LIST SPECIFIED_TDF_FILE_COUNT)

if (${DISCOVERED_TDF_FILE_COUNT} GREATER ${SPECIFIED_TDF_FILE_COUNT})
    message(
        FATAL_ERROR
        "TDF file list is not in sync. Discovered ${DISCOVERED_TDF_FILE_COUNT} TDF files in "
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/ but only ${SPECIFIED_TDF_FILE_COUNT} "
        "have been specified in ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/TargetDescriptionFiles.cmake "
        "Please review TDF_FILES_LIST in the TargetDescriptionFiles.cmake"
    )
endif()
