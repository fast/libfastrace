cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0048 NEW)
project(libfastrace VERSION 0.7.2)
set (CMAKE_CXX_STANDARD 11)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CARGO_CMD cargo build --verbose --target-dir=${CMAKE_CURRENT_BINARY_DIR})
    set(TARGET_DIR "debug")
else ()
    set(CARGO_CMD cargo build --release --target-dir=${CMAKE_CURRENT_BINARY_DIR})
    set(TARGET_DIR "release")
endif ()

set(RUST_PART_LIB "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_DIR}/libfastrace_rust.a")
set(RUST_PART_CXX "${CMAKE_CURRENT_BINARY_DIR}/cxxbridge/libfastrace/src/lib.rs.cc")
set(RUST_PART_H "${CMAKE_CURRENT_BINARY_DIR}/cxxbridge/libfastrace/src/lib.rs.h")
add_custom_command(
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Cargo.toml ${CMAKE_CURRENT_SOURCE_DIR}/src/lib.rs ${CMAKE_CURRENT_SOURCE_DIR}/build.rs
    OUTPUT ${RUST_PART_LIB} ${RUST_PART_CXX} ${RUST_PART_H}
    COMMAND CARGO_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR} RUSTFLAGS="${RUST_FLAGS}" ${CARGO_CMD}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}/cxxbridge/libfastrace/src
    ${CMAKE_SOURCE_DIR}/include
)
add_library(libfastrace STATIC src/libfastrace.cpp ${RUST_PART_CXX})
set_target_properties(libfastrace PROPERTIES PUBLIC_HEADER "${CMAKE_SOURCE_DIR}/include/libfastrace.h;${RUST_PART_H}")

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    libfastraceConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(TARGETS libfastrace
        EXPORT libfastraceTargets
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include/libfastrace
        )

install(FILES ${RUST_PART_LIB} DESTINATION lib)

install(EXPORT libfastraceTargets
        FILE libfastraceTargets.cmake
        DESTINATION lib/cmake/libfastrace
        )

configure_file(libfastraceConfig.cmake.in libfastraceConfig.cmake @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libfastraceConfig.cmake"
                "${CMAKE_CURRENT_BINARY_DIR}/libfastraceConfigVersion.cmake"
        DESTINATION lib/cmake/libfastrace
        )
