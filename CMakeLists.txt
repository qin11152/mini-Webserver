cmake_minimum_required(VERSION 3.13)

project(server)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(${CMAKE_SOURCE_DIR}/TinyMuduo/net)
include_directories(${CMAKE_SOURCE_DIR}/TinyMuduo/base)
include_directories(${CMAKE_SOURCE_DIR}/TinyMuduo)

link_directories(${CMAKE_BINARY_DIR}/lib)

add_subdirectory(${CMAKE_SOURCE_DIR}/TinyMuduo/net)
add_subdirectory(${CMAKE_SOURCE_DIR}/TinyMuduo/base)
add_subdirectory(${CMAKE_SOURCE_DIR}/demo)

aux_source_directory(./TinyMuduo srcs)

add_executable(${PROJECT_NAME} ${srcs})

target_link_libraries(${PROJECT_NAME} qinNet qinBase pthread)

