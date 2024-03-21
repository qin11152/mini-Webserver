set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(echo echo/echo.cpp echo/main.cpp)

target_link_libraries(echo qinNet qinBase pthread)
