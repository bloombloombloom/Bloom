set(BLOOM_PACKAGE_NAME "Bloom")
set(BLOOM_PACKAGE_FILE_NAME "Bloom-${CMAKE_PROJECT_VERSION}-Linux")
set(BLOOM_PACKAGE_DESCRIPTION "Debugger for AVR-based embedded systems")
set(BLOOM_PACKAGE_CONTACT "Nav Mohammed <support@bloom.oscillate.io>")
set(BLOOM_PACKAGE_RELEASE_DIR "${CMAKE_BINARY_DIR}/packaging/tmp/release")

string(TOLOWER ${BLOOM_PACKAGE_NAME} BLOOM_PACKAGE_NAME_LOWER)

file(MAKE_DIRECTORY "${BLOOM_PACKAGE_RELEASE_DIR}")

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/build/packaging/deb/package-deb.sh.in"
    "${CMAKE_BINARY_DIR}/packaging/package-deb.sh"
    FILE_PERMISSIONS
        OWNER_EXECUTE OWNER_READ OWNER_WRITE
        GROUP_READ
        WORLD_READ
    @ONLY
)

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/packaging/tmp/deb")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/packaging/tmp/rpm")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/packaging/tmp/pkgbuild")

# Generate the relevant configuration files for our DEB, RPM and PKGBUILD packages
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/build/packaging/deb/control.in"
    "${CMAKE_BINARY_DIR}/packaging/tmp/deb/DEBIAN/control"
    @ONLY
)
