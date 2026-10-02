cmake_minimum_required(VERSION 3.20)
project(BDFR_FacialAnimation VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(bdfr_core
    src/CurveRegistry.cpp
    src/BinaryCodec.cpp
    src/CurveMixer.cpp
    src/Timeline.cpp
)

target_include_directories(bdfr_core PUBLIC include)

if(MSVC)
    target_compile_options(bdfr_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(bdfr_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()
add_executable(bdfr_core_tests tests/test_core.cpp)
target_link_libraries(bdfr_core_tests PRIVATE bdfr_core)
add_test(NAME bdfr_core_tests COMMAND bdfr_core_tests)
