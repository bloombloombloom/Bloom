cmake_minimum_required(VERSION 3.22)

project(Bloom LANGUAGES CXX VERSION 1.0.0)
set(CMAKE_PROJECT_HOMEPAGE_URL "https://bloom.oscillate.io")

set(CMAKE_VERBOSE_MAKEFILE off)

set(CMAKE_CXX_STANDARD 20)
set(ENABLE_SANITIZERS off)

set(CMAKE_AUTOMOC ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

option(EXCLUDE_INSIGHT "Exclude the Insight component from this build" OFF)

set(CMAKE_SKIP_BUILD_RPATH false)
set(CMAKE_BUILD_RPATH_USE_ORIGIN true)
set(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib:/usr/local/lib")
set(CMAKE_BUILD_RPATH ${CMAKE_INSTALL_RPATH})
set(COMPILED_RESOURCES_BUILD_DIR ${CMAKE_BINARY_DIR}/compiled_resources/)

add_compile_definitions(BLOOM_VERSION="${CMAKE_PROJECT_VERSION}")

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    add_compile_definitions(BLOOM_DEBUG_BUILD)

    # BLOOM_COMPILED_RESOURCES_PATH_OVERRIDE can be used to override the file path used for compiled resources.
    # We override this path in debug builds to avoid using compiled resources. This makes debugging and small tweaks
    # a lot easier, as it removes the need to recompile for each tweak.
    # CAUTION: Although convenient, this does add a limitation; the debug build can only be run on the same machine
    # that compiled it. Or a machine that has the Bloom source located in the same place.
    # See Services::PathService::compiledResourcesPath() for more.
    add_compile_definitions(BLOOM_COMPILED_RESOURCES_PATH_OVERRIDE="${CMAKE_CURRENT_SOURCE_DIR}")

    # BLOOM_HOME_DOMAIN_NAME_OVERRIDE can be used to override the domain name used in URLs to the Bloom website.
    # I (Nav) use this in debug builds, so I can test local changes that involve the Bloom website as well as Bloom
    # itself. Other users can comment out this override if they don't have a copy of the Bloom website running on their
    # local machine. See Services::PathService::homeDomainName() for more.
    add_compile_definitions(BLOOM_HOME_DOMAIN_NAME_OVERRIDE="http://bloom.local")

    # CMAKE_BUILD_RPATH needs to point to the local Qt installation, to use Gammaray during development.
    # This is because the distributed Qt binaries may not be compatible with the local installation of Gammaray
    # If you don't intend to use Gammaray, you can comment this out
    set(CMAKE_BUILD_RPATH /opt/Qt/6.2.4/gcc_64/lib/)
endif()

add_executable(Bloom)
set_target_properties(Bloom PROPERTIES OUTPUT_NAME bloom)

find_package(yaml-cpp 0.7.0 REQUIRED)
find_package(Qt6Core REQUIRED)
find_package(Qt6Xml REQUIRED)
find_package(Qt6Network REQUIRED)

if (NOT ${EXCLUDE_INSIGHT})
    find_package(Qt6Widgets REQUIRED)
    find_package(Qt6Gui REQUIRED)
    find_package(Qt6Widgets REQUIRED)
    find_package(Qt6Xml REQUIRED)
    find_package(Qt6Svg REQUIRED)
    find_package(Qt6UiTools REQUIRED)
    find_package(Qt6SvgWidgets REQUIRED)

    target_link_libraries(Bloom Qt6::Gui)
    target_link_libraries(Bloom Qt6::UiTools)
    target_link_libraries(Bloom Qt6::Widgets)
    target_link_libraries(Bloom Qt6::Xml)
    target_link_libraries(Bloom Qt6::Svg)
    target_link_libraries(Bloom Qt6::SvgWidgets)
endif()

target_link_libraries(Bloom -lstdc++fs)
target_link_libraries(Bloom -lpthread)
target_link_libraries(Bloom -lusb-1.0)
target_link_libraries(Bloom -lhidapi-libusb)
target_link_libraries(Bloom -lprocps)
target_link_libraries(Bloom ${YAML_CPP_LIBRARIES})
target_link_libraries(Bloom Qt6::Core)
target_link_libraries(Bloom Qt6::Xml)
target_link_libraries(Bloom Qt6::Network)

target_sources(
    Bloom
    PRIVATE
        ${CMAKE_BINARY_DIR}/resources/TargetDescriptionFiles/AVR/Mapping.json
)

qt_add_resources(
    Bloom
    "ApplicationResources"
    PREFIX
        "/compiled"
    FILES
        "./resources/bloom.template.yaml"
        "./resources/help.txt"
)

add_subdirectory(src)
target_include_directories(Bloom PUBLIC ./)
target_include_directories(Bloom PUBLIC ${YAML_CPP_INCLUDE_DIR})

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    # When Qt isn't playing nice, it's very useful to have access to the Qt source code, to step through.
    # The QT source directory is specified as an include path just so that CLion can navigate to the Qt implementation
    # files, during debugging. No QT headers are actually included via this method. Feel free to comment this out if
    # you don't possess the Qt source code on your machine. You may need to invalidate CMake cache.
    # target_include_directories(Bloom PUBLIC /opt/Qt/6.2.4/Src)
endif()

target_compile_definitions(
    Bloom
    PUBLIC $<$<BOOL:${EXCLUDE_INSIGHT}>:EXCLUDE_INSIGHT>
)

target_compile_options(
    Bloom
    PUBLIC -std=c++2a
    PUBLIC -pedantic
    PUBLIC -Wconversion
    PUBLIC -Wpessimizing-move
    PUBLIC -Wredundant-move
    PUBLIC -Wsuggest-override
    PUBLIC -Wreorder
    PUBLIC -fno-sized-deallocation
    PUBLIC $<$<CONFIG:DEBUG>:-g>
    # PUBLIC $<$<CONFIG:DEBUG>:-O0>
    PUBLIC $<$<CONFIG:DEBUG>:-Ofast>
    PUBLIC $<$<CONFIG:RELEASE>:-Ofast>
    PUBLIC $<$<CONFIG:DEBUG>:-fno-inline>
    PUBLIC $<$<CONFIG:DEBUG>:-fkeep-static-functions>
)

target_link_options(
    Bloom
    PUBLIC [=[-Wl,-z,origin]=] #,--verbose
)

if (${ENABLE_SANITIZERS})
    message(WARNING "Sanitizers have been enabled")

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

# Copy AVR8 TDFs to build directory and construct JSON mapping of AVR8 target signatures to TDF paths.
add_custom_command(
    OUTPUT
    ${CMAKE_BINARY_DIR}/resources/TargetDescriptionFiles/AVR/Mapping.json
    DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/build/scripts/Avr8TargetDescriptionFiles.php
    COMMAND echo 'Processing AVR target description files.'
    COMMAND
    php ${CMAKE_CURRENT_SOURCE_DIR}/build/scripts/Avr8TargetDescriptionFiles.php ${CMAKE_BINARY_DIR}
)

include(./cmake/Installing.cmake)

include(./cmake/Packaging.cmake)
