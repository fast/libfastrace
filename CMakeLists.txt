cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0048 NEW)
project(minitrace_c VERSION 0.5.1)
set (CMAKE_CXX_STANDARD 11)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CARGO_CMD cargo build --verbose --target-dir=${CMAKE_CURRENT_BINARY_DIR})
    set(TARGET_DIR "debug")
else ()
    set(CARGO_CMD cargo build --release --target-dir=${CMAKE_CURRENT_BINARY_DIR})
    set(TARGET_DIR "release")
endif ()

set(RUST_PART_LIB "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_DIR}/libminitrace_rust.a")
set(RUST_PART_CXX "${CMAKE_CURRENT_BINARY_DIR}/cxxbridge/minitrace-c/src/lib.rs.cc")
set(RUST_PART_H "${CMAKE_CURRENT_BINARY_DIR}/cxxbridge/minitrace-c/src/lib.rs.h")
add_custom_command(
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Cargo.toml ${CMAKE_CURRENT_SOURCE_DIR}/src/lib.rs ${CMAKE_CURRENT_SOURCE_DIR}/build.rs
    OUTPUT ${RUST_PART_LIB} ${RUST_PART_CXX} ${RUST_PART_H}
    COMMAND CARGO_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR} RUSTFLAGS="${RUST_FLAGS}" ${CARGO_CMD}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}/cxxbridge/minitrace-c/src
    ${CMAKE_SOURCE_DIR}/include
)
add_library(minitrace_c STATIC src/minitrace_c.cpp ${RUST_PART_CXX})
set_target_properties(minitrace_c PROPERTIES PUBLIC_HEADER "${CMAKE_SOURCE_DIR}/include/minitrace_c.h;${RUST_PART_H}")

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    minitrace_cConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(TARGETS minitrace_c
        EXPORT minitrace_cTargets
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include/minitrace_c
        )

install(FILES ${RUST_PART_LIB} DESTINATION lib)

install(EXPORT minitrace_cTargets
        FILE minitrace_cTargets.cmake
        DESTINATION lib/cmake/minitrace_c
        )

configure_file(minitrace_cConfig.cmake.in minitrace_cConfig.cmake @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/minitrace_cConfig.cmake"
                "${CMAKE_CURRENT_BINARY_DIR}/minitrace_cConfigVersion.cmake"
        DESTINATION lib/cmake/minitrace_c
        )
