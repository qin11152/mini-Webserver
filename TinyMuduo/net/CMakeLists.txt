set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

aux_source_directory(. srcs)

add_library(qinNet SHARED ${srcs})