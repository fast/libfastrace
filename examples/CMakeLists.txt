project(Example)
cmake_minimum_required(VERSION 3.5)
set (CMAKE_C_STANDARD 11)
set (CMAKE_CXX_STANDARD 11)

# using local repository
find_package(libfastrace 0.7.2)

function(add_example target_name source_file)
  add_executable(${target_name} ${source_file})
  target_link_libraries(${target_name} libfastrace fastrace_rust)
endfunction()

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

foreach(source_file ${C_EXAMPLE_SOURCES})
  get_filename_component(target_name ${source_file} NAME_WE)
  add_example(${target_name} ${source_file})
endforeach()

foreach(source_file ${CXX_EXAMPLE_SOURCES})
  get_filename_component(target_name ${source_file} NAME_WE)
  add_example(${target_name} ${source_file})
endforeach()
