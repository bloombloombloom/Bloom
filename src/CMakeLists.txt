target_sources(
    Bloom
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Application.cpp

        # Helpers & other
        ${CMAKE_CURRENT_SOURCE_DIR}/Logger/Logger.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Helpers/Paths.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Helpers/String.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Helpers/EpollInstance.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Helpers/EventFdNotifier.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Helpers/ConditionVariableNotifier.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Helpers/Process.cpp
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
add_subdirectory(Insight)
