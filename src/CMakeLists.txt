target_sources(
    Bloom
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Application.cpp

        # Services
        ${CMAKE_CURRENT_SOURCE_DIR}/Services/TargetService.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Services/TargetControllerService.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Services/PathService.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Services/ProcessService.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Services/StringService.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Services/IntegerService.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Services/AlignmentService.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Services/Avr8InstructionService.cpp

        # Helpers & other
        ${CMAKE_CURRENT_SOURCE_DIR}/Logger/Logger.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Helpers/EpollInstance.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Helpers/EventFdNotifier.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Helpers/ConditionVariableNotifier.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/VersionNumber.cpp

        # Project & application configuration
        ${CMAKE_CURRENT_SOURCE_DIR}/ProjectConfig.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ProjectSettings.cpp

        # Events
        ${CMAKE_CURRENT_SOURCE_DIR}/EventManager/EventListener.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/EventManager/EventManager.cpp

        # Signal handler
        ${CMAKE_CURRENT_SOURCE_DIR}/SignalHandler/SignalHandler.cpp
)

add_subdirectory(DebugToolDrivers)
add_subdirectory(Targets)
add_subdirectory(TargetController)
add_subdirectory(DebugServer)

if (NOT ${EXCLUDE_INSIGHT})
    add_subdirectory(Insight)
endif()
