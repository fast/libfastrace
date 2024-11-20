cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0048 NEW)
project(Example VERSION 1.0.0)

# Set C/C++ standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required package
find_package(libfastrace 0.7.2 REQUIRED)
if(NOT libfastrace_FOUND)
    message(FATAL_ERROR "libfastrace not found!")
endif()

# Helper function to add examples
function(add_example target_name source_file)
    add_executable(${target_name} ${source_file})
    target_link_libraries(${target_name}
        PRIVATE
            libfastrace::libfastrace  # Use namespace
            fastrace_rust
    )
    # Add compile options if needed
    target_compile_options(${target_name}
        PRIVATE
            $<$<C_COMPILER_ID:GNU>:-Wall -Wextra>
            $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra>
    )
endfunction()

# Define source files
set(C_EXAMPLE_SOURCES
    get_started.c
    synchronous.c
    asynchronous.c
)

set(CXX_EXAMPLE_SOURCES
    get_started2.cc
    synchronous2.cc
    asynchronous2.cc
)

# Build C examples
foreach(source_file ${C_EXAMPLE_SOURCES})
    get_filename_component(target_name ${source_file} NAME_WE)
    add_example(${target_name} ${source_file})
endforeach()

# Build C++ examples
foreach(source_file ${CXX_EXAMPLE_SOURCES})
    get_filename_component(target_name ${source_file} NAME_WE)
    add_example(${target_name} ${source_file})
endforeach()

# Optional: Add install targets for examples
option(INSTALL_EXAMPLES "Install example executables" OFF)
if(INSTALL_EXAMPLES)
    foreach(source_file ${C_EXAMPLE_SOURCES} ${CXX_EXAMPLE_SOURCES})
        get_filename_component(target_name ${source_file} NAME_WE)
        install(TARGETS ${target_name}
            RUNTIME DESTINATION bin/examples
        )
    endforeach()
endif()
