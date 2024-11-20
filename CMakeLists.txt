# Minimum CMake version and project definition
cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0048 NEW)
project(libfastrace VERSION 0.7.2)

# Build options
option(BUILD_TESTING "Build the testing tree." OFF)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build" FORCE)
endif()

# Configure Cargo build command based on build type
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CARGO_CMD cargo build --verbose --target-dir=${CMAKE_CURRENT_BINARY_DIR})
    set(TARGET_DIR "debug")
else()
    set(CARGO_CMD cargo build --release --target-dir=${CMAKE_CURRENT_BINARY_DIR})
    set(TARGET_DIR "release")
endif()

# Define paths for Rust-generated files
set(RUST_PART_LIB "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_DIR}/libfastrace_rust.a")
set(RUST_PART_CXX "${CMAKE_CURRENT_BINARY_DIR}/cxxbridge/libfastrace/src/lib.rs.cc")
set(RUST_PART_H "${CMAKE_CURRENT_BINARY_DIR}/cxxbridge/libfastrace/src/lib.rs.h")

# Custom command to build Rust part
add_custom_command(
    OUTPUT ${RUST_PART_LIB} ${RUST_PART_CXX} ${RUST_PART_H}
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/Cargo.toml
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lib.rs
        ${CMAKE_CURRENT_SOURCE_DIR}/build.rs
    COMMAND CARGO_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR}
            RUSTFLAGS="${RUST_FLAGS}"
            ${CARGO_CMD}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building Rust library..."
)

# Add library target
add_library(libfastrace STATIC
    src/libfastrace.cpp
    ${RUST_PART_CXX}
)

# Configure include directories
target_include_directories(libfastrace
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/cxxbridge/libfastrace/src>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Set library properties
set_target_properties(libfastrace
    PROPERTIES
        OUTPUT_NAME "fastrace"
        PUBLIC_HEADER "${CMAKE_SOURCE_DIR}/include/libfastrace.h;${RUST_PART_H}"
)

# Generate version file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    libfastraceConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

# Install targets
install(TARGETS libfastrace
    EXPORT libfastraceTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include/libfastrace
)

# Install Rust library
install(FILES ${RUST_PART_LIB}
    DESTINATION lib
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)

# Install CMake config files
install(EXPORT libfastraceTargets
    FILE libfastraceTargets.cmake
    NAMESPACE libfastrace::
    DESTINATION lib/cmake/libfastrace
)

configure_file(libfastraceConfig.cmake.in libfastraceConfig.cmake @ONLY)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/libfastraceConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/libfastraceConfigVersion.cmake"
    DESTINATION lib/cmake/libfastrace
)

# Add uninstall target
if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY
    )
    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
    )
endif()

# Add tests if enabled
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
