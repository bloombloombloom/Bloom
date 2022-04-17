cmake_minimum_required(VERSION 3.22)

project(Bloom LANGUAGES CXX VERSION 0.8.3)
set(CMAKE_PROJECT_HOMEPAGE_URL "https://bloom.oscillate.io")

set(CMAKE_VERBOSE_MAKEFILE off)

# Create directory for generated sources
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/Generated)

set(CMAKE_CXX_STANDARD 20)
set(ENABLE_SANITIZERS off)

add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

link_directories(/usr/local/lib)

set(CMAKE_AUTOMOC ON)
set(AUTOGEN_BUILD_DIR ../build/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build/bin")

find_package(Qt6Core REQUIRED)
find_package(Qt6Gui REQUIRED)
find_package(Qt6Widgets REQUIRED)
find_package(Qt6Xml REQUIRED)
find_package(Qt6Svg REQUIRED)
find_package(Qt6UiTools REQUIRED)
find_package(Qt6SvgWidgets REQUIRED)
find_package(Qt6Network REQUIRED)

# Bloom is distributed with some third-party dependencies (in the form of shared objects). These dependencies are
# distributed with Bloom for various reasons (licensing, availability in some package managers, etc).
#
# The shared objects are located in bin/lib (relative to Bloom's installation directory). For this reason, we instruct
# the dynamic linker to look for any of Bloom's shared object dependencies in that location first. We do this by
# including '$ORIGIN/lib' in the RPATH of Bloom's binary executable.
set(CMAKE_SKIP_BUILD_RPATH false)
set(CMAKE_BUILD_RPATH_USE_ORIGIN true)
set(CMAKE_INSTALL_RPATH "\$ORIGIN/lib:/usr/local/lib")
set(CMAKE_BUILD_RPATH ${CMAKE_INSTALL_RPATH})

add_compile_definitions(BLOOM_VERSION="${CMAKE_PROJECT_VERSION}")

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    add_compile_definitions(BLOOM_DEBUG_BUILD)

    # BLOOM_COMPILED_RESOURCES_PATH_OVERRIDE can be used to override the file path used for compiled resources.
    # We override this path in debug builds to avoid using compiled resources. This makes debugging and small tweaks
    # a lot easier, as it removes the need to recompile for each tweak.
    # CAUTION: Although convenient, this does add a limitation; the debug build can only be run on the same machine
    # that compiled it. Or a machine that has the Bloom source located in the same place.
    # See Paths::compiledResourcesPath() for more.
    add_compile_definitions(BLOOM_COMPILED_RESOURCES_PATH_OVERRIDE="${CMAKE_CURRENT_SOURCE_DIR}")

    # BLOOM_HOME_DOMAIN_NAME_OVERRIDE can be used to override the domain name used in URLs to the Bloom website.
    # I (Nav) use this in debug builds, so I can test local changes that involve the Bloom website as well as Bloom
    # itself. Other users can comment out this override if they don't have a copy of the Bloom website running on their
    # local machine. See Paths::homeDomainName() for more.
    add_compile_definitions(BLOOM_HOME_DOMAIN_NAME_OVERRIDE="http://bloom.local")

    # CMAKE_BUILD_RPATH needs to point to the local Qt installation, to use Gammaray during development.
    # This is because the distributed Qt binaries may not be compatible with the local installation of Gammaray
    set(CMAKE_BUILD_RPATH /opt/Qt/6.1.2/gcc_64/lib/)
endif()

add_executable(Bloom build/resources/TargetDescriptionFiles/AVR/Mapping.json)
add_subdirectory(src)

set_target_properties(Bloom PROPERTIES OUTPUT_NAME bloom)
target_include_directories(Bloom PUBLIC ./)

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    # When Qt isn't playing nice, it's very useful to have access to the Qt source code, to step through.
    # The QT source directory is specified as an include path just so that CLion can navigate to the Qt implementation
    # files, during debugging. No QT headers are actually included via this method. Feel free to comment this out if
    # you don't possess the Qt source code on your machine. You may need to invalidate CMake cache.
    target_include_directories(Bloom PUBLIC /opt/Qt/6.1.2/Src)
endif()

target_link_libraries(Bloom -static-libgcc -static-libstdc++)
target_link_libraries(Bloom -lstdc++fs)
target_link_libraries(Bloom -lpthread)
target_link_libraries(Bloom -lusb-1.0)
target_link_libraries(Bloom -lhidapi-libusb)
target_link_libraries(Bloom Qt6::Core)
target_link_libraries(Bloom Qt6::Gui)
target_link_libraries(Bloom Qt6::UiTools)
target_link_libraries(Bloom Qt6::Widgets)
target_link_libraries(Bloom Qt6::Xml)
target_link_libraries(Bloom Qt6::Svg)
target_link_libraries(Bloom Qt6::SvgWidgets)
target_link_libraries(Bloom Qt6::Network)

target_compile_options(
    Bloom
    PUBLIC -std=c++2a
    PUBLIC -pedantic
    PUBLIC -Wconversion
    PUBLIC -Wpessimizing-move
    PUBLIC -Wredundant-move
    PUBLIC -fno-sized-deallocation
    PUBLIC $<$<CONFIG:DEBUG>:-g>
    PUBLIC $<$<CONFIG:DEBUG>:-O0>
    #    PUBLIC $<$<CONFIG:DEBUG>:-O3>
    PUBLIC $<$<CONFIG:RELEASE>:-Ofast>
    PUBLIC $<$<CONFIG:DEBUG>:-fno-inline>
    PUBLIC $<$<CONFIG:DEBUG>:-fkeep-static-functions>
)

target_link_options(
    Bloom
    PUBLIC [=[-Wl,--disable-new-dtags]=] #,--verbose
)

if (${ENABLE_SANITIZERS})
    message(WARNING "Sanitizers have been enabled")

    # For TSAN, see ThreadSanitizerSuppression.txt

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
    ${CMAKE_CURRENT_SOURCE_DIR}/build/resources/TargetDescriptionFiles/AVR/Mapping.json
    DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/build/scripts/Avr8TargetDescriptionFiles.php
    COMMAND echo 'Processing AVR target description files.'
    COMMAND
    php ${CMAKE_CURRENT_SOURCE_DIR}/build/scripts/Avr8TargetDescriptionFiles.php
)

# Compile resources
add_custom_command(
    OUTPUT
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Generated/resources.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Generated/resources_fake.cpp
    DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources.qrc
    COMMAND echo 'Compiling QT resources. |${CMAKE_BUILD_TYPE}|'
    COMMAND
    rcc -o ${CMAKE_CURRENT_SOURCE_DIR}/src/Generated/resources.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources.qrc
)

# Copy resources/fonts into build/resources/Fonts
add_custom_command(
    TARGET Bloom
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/fonts
    ${CMAKE_CURRENT_SOURCE_DIR}/build/resources/Fonts
)

# Copy resources/udevrules/99-bloom.rules to build/resources/UDevRules/99-bloom.rules
add_custom_command(
    TARGET Bloom
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/udevrules/99-bloom.rules
    ${CMAKE_CURRENT_SOURCE_DIR}/build/resources/UDevRules/99-bloom.rules
)

# Installation configuration
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/release/")

install(TARGETS Bloom DESTINATION bin PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)

install(
    DIRECTORY build/bin/plugins
    DESTINATION "bin"
    DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
)

install(
    DIRECTORY build/bin/platforms
    DESTINATION "bin"
    DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
)

install(
    DIRECTORY build/resources
    DESTINATION "."
    DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
)

install(
    DIRECTORY build/bin/lib
    DESTINATION "bin"
    DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
)

configure_file(
    "${PROJECT_SOURCE_DIR}/Packaging.cmake"
    "${PROJECT_BINARY_DIR}/Packaging.cmake"
)

set(CPACK_GENERATOR "DEB;RPM")
set(CPACK_PROJECT_CONFIG_FILE "${PROJECT_BINARY_DIR}/CPack.cmake")

include(CPack)
