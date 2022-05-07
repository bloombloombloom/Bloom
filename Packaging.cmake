# This file contains the CPack configuration for packaging Bloom. Bloom is currently packaged in Debian, RPM and
# PKGBUILD packages. Because CPack doesn't support PKGBUILD packages, we have to build those separately. But we still
# use CMake & CPack configuration to generate our PKGBUILD file. See below for more.
#
# To package Bloom, simply run CPack & makepkg from the CMake build directory:
#
# $ cd build/cmake-build-release/
# $ cpack
# $ makepkg
#
# CPack's -G option can be used to specify a particular generator:
# $ cpack -G DEB
#
# If no generator has been specified, CPack will generate packages for all configured generators (see CPACK_GENERATOR
# below).
#
# The makepkg command builds the PKGBUILD package. It uses the generated PKGBUILD file, which is generated from
# resources/packaging/PKGBUILD.template.in. See the configure_file() invocations below.
#
# NOTE: The above commands assume Bloom has been built in 'release' mode, and the install target has been run.

configure_file(
    "${PROJECT_SOURCE_DIR}/PackageGeneratorConfig.cmake"
    "${PROJECT_BINARY_DIR}/PackageGeneratorConfig.cmake"
)

set(CPACK_GENERATOR "DEB;RPM")
set(CPACK_PROJECT_CONFIG_FILE "${PROJECT_BINARY_DIR}/PackageGeneratorConfig.cmake")

set(CPACK_PACKAGE_NAME "Bloom")
string(TOLOWER ${CPACK_PACKAGE_NAME} CPACK_PACKAGE_NAME_LOWER)

set(
    CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Debugger for AVR-based embedded systems"
)

set(CPACK_PACKAGE_CONTACT "Nav Mohammed <support@bloom.oscillate.io>")

set(CPACK_PACKAGE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(CPACK_PACKAGING_INSTALL_PREFIX "/opt/bloom")

# Generate the PKGBUILD and bloom.install file in the CMake build directory.
configure_file(
    "${PROJECT_SOURCE_DIR}/resources/packaging/PKGBUILD.template.in"
    "${PROJECT_BINARY_DIR}/PKGBUILD"
    @ONLY
)

configure_file(
    "${PROJECT_SOURCE_DIR}/resources/packaging/bloom.install.template.in"
    "${PROJECT_BINARY_DIR}/bloom.install"
    @ONLY
)

include(CPack)
