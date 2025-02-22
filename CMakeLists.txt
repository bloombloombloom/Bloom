cmake_minimum_required(VERSION 3.22)

project(Bloom LANGUAGES CXX VERSION 1.0.0)
set(CMAKE_PROJECT_HOMEPAGE_URL "https://bloom.oscillate.io")
set(CMAKE_CXX_STANDARD 20)

set(AUTOGEN_BUILD_DIR ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}_autogen/)

set(CMAKE_VERBOSE_MAKEFILE off)

set(ENABLE_SANITIZERS off)

set(CMAKE_AUTOMOC ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

option(EXCLUDE_INSIGHT "Exclude the Insight component from this build" OFF)

set(CMAKE_SKIP_RPATH true)
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
    set(CMAKE_SKIP_RPATH false)
    set(CMAKE_SKIP_BUILD_RPATH false)
    set(CMAKE_BUILD_RPATH /opt/Qt/6.2.4/gcc_64/lib/)
endif()

if (${CMAKE_BUILD_TYPE} MATCHES "Release")
    # A lot of Bloom's code runs on the assumption that the loaded TDF is valid and doesn't contain any errors.
    # The TDF validation script confirms this at build time, and we perform some sanity checks via assertions.
    # Disabling these sanity checks in release builds is problematic, because users can edit TDFs. They really
    # shouldn't do this, but I can see it happening.
    #
    # Enabling assertions in release builds will decrease the likelihood of all sorts of horrible things, if the user
    # messes with the TDFs in Bloom's installation directory.
    remove_definitions("-DNDEBUG")
endif()
add_executable(Bloom)
set_target_properties(
    Bloom
    PROPERTIES
        OUTPUT_NAME bloom
)

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
    PUBLIC -Wno-stringop-overflow
    PUBLIC -fno-sized-deallocation
    PUBLIC $<$<CONFIG:RELEASE>:-ffile-prefix-map=${CMAKE_SOURCE_DIR}/./src/=>
    PUBLIC $<$<CONFIG:RELEASE>:-ffile-prefix-map=${CMAKE_SOURCE_DIR}/src/=>
    PUBLIC $<$<CONFIG:DEBUG>:-g>
    # PUBLIC $<$<CONFIG:DEBUG>:-O0>
    PUBLIC $<$<CONFIG:DEBUG>:-Ofast>
    PUBLIC $<$<CONFIG:RELEASE>:-Ofast>
    PUBLIC $<$<CONFIG:DEBUG>:-fno-inline>
    PUBLIC $<$<CONFIG:DEBUG>:-fkeep-static-functions>
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

include(${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/TargetDescriptionFiles.cmake)

# This custom command will invoke the TDF validation script for all TDFs in Bloom's codebase. It will also create a
# text file, which we use as a dependency in the custom command to generate brief target descriptors. This dependency
# allows us to avoid needlessly running TDF validation and brief target descriptor generation on every build.
#
# We specify all TDF files as dependencies for this command, so that CMake will run the command whenever a TDF is
# modified.
set(TDF_VALIDATION_OUTPUT_FILE_PATH "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}_autogen/tdf_validation_date.txt")
add_custom_command(
    OUTPUT ${TDF_VALIDATION_OUTPUT_FILE_PATH}
    COMMENT "Validating target description files"
    DEPENDS ${TDF_FILES_LIST}
    COMMAND php ${CMAKE_CURRENT_SOURCE_DIR}/build/scripts/ValidateTargetDescriptionFiles.php
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/
    COMMAND date > ${TDF_VALIDATION_OUTPUT_FILE_PATH}
)

set(
    GENERATED_BRIEF_TARGET_DESCRIPTOR_MAPPING_PATH
    "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}_autogen/GeneratedMapping.txt"
)

target_sources(
    Bloom
    PRIVATE
        ${GENERATED_BRIEF_TARGET_DESCRIPTOR_MAPPING_PATH}
)

target_compile_definitions(
    Bloom
    PUBLIC GENERATED_BRIEF_TARGET_DESCRIPTOR_MAPPING_PATH="${GENERATED_BRIEF_TARGET_DESCRIPTOR_MAPPING_PATH}"
)

# This custom command will invoke the GenerateBriefTargetDescriptors.php build script to generate a
# BriefTargetDescriptor for all targets supported by Bloom. These descriptors are stored in an ASCII text file, located
# at ${GENERATED_BRIEF_TARGET_DESCRIPTOR_MAPPING_PATH}. See the TargetService class for more on this.
#
# The script will also copy all TDFs to the build directory.
#
# We specify all TDF files as dependencies for this command, so that CMake will run the command whenever a TDF is
# modified.
add_custom_command(
    OUTPUT
        ${GENERATED_BRIEF_TARGET_DESCRIPTOR_MAPPING_PATH}
        ${CMAKE_BINARY_DIR}/resources/TargetDescriptionFiles/
    DEPENDS
        ${TDF_VALIDATION_OUTPUT_FILE_PATH}
        ${CMAKE_CURRENT_SOURCE_DIR}/build/scripts/GenerateBriefTargetDescriptors.php
        ${TDF_FILES_LIST}
    COMMENT "Processing target description files"
    COMMAND php
    ARGS
        ${CMAKE_CURRENT_SOURCE_DIR}/build/scripts/GenerateBriefTargetDescriptors.php
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Targets/TargetDescriptionFiles/
        ${GENERATED_BRIEF_TARGET_DESCRIPTOR_MAPPING_PATH}
        ${CMAKE_BINARY_DIR}/resources/TargetDescriptionFiles/
)

include(./cmake/Installing.cmake)
include(./cmake/Packaging.cmake)
