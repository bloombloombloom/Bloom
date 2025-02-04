set(BLOOM_PACKAGE_NAME "Bloom")
set(BLOOM_PACKAGE_FILE_NAME "Bloom-${CMAKE_PROJECT_VERSION}-Linux-x86_64")
set(BLOOM_PACKAGE_DESCRIPTION "Debugger for AVR-based embedded systems")
set(BLOOM_PACKAGE_CONTACT "Nav Mohammed <support@bloom.oscillate.io>")

# All generated packages will install Bloom to BLOOM_INSTALLATION_PREFIX
set(BLOOM_INSTALLATION_PREFIX "/opt/bloom")
set(BLOOM_SHARED_LIBRARY_PATHS "${BLOOM_INSTALLATION_PREFIX}/lib")

string(TOLOWER ${BLOOM_PACKAGE_NAME} BLOOM_PACKAGE_NAME_LOWER)

# Generate Bloom's invocation script
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/build/distributed/bloom.sh.in"
    "${CMAKE_BINARY_DIR}/packaging/bloom.sh"
    @ONLY
)

# Generate the DEB control file and packaging script
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/packaging/deb")

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/build/packaging/deb/control.in"
    "${CMAKE_BINARY_DIR}/packaging/deb/control"
    @ONLY
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/build/packaging/deb/package-deb.sh.in"
    "${CMAKE_BINARY_DIR}/packaging/package-deb.sh"
    FILE_PERMISSIONS
    OWNER_EXECUTE OWNER_READ OWNER_WRITE
    GROUP_READ
    WORLD_READ
    @ONLY
)

# Generate the PKGBUILD file and packaging script
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/packaging/pkgbuild")

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/build/packaging/pkgbuild/PKGBUILD.in"
    "${CMAKE_BINARY_DIR}/packaging/pkgbuild/PKGBUILD"
    @ONLY
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/build/packaging/pkgbuild/package-pkgbuild.sh.in"
    "${CMAKE_BINARY_DIR}/packaging/package-pkgbuild.sh"
    FILE_PERMISSIONS
    OWNER_EXECUTE OWNER_READ OWNER_WRITE
    GROUP_READ
    WORLD_READ
    @ONLY
)
