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
    src/Session.cpp
    src/Sequence.cpp
    src/CorrectiveEngine.cpp
    src/ExpressionStack.cpp
    src/JsonCodec.cpp
    src/CurveDynamics.cpp
    src/KeyReducer.cpp
    src/Project.cpp
    src/CurveCatalog.cpp
    src/CurveTools.cpp
    src/Diagnostics.cpp
    src/Persistence.cpp
    src/Retargeter.cpp
    src/AutoMapper.cpp
    src/TextSpeech.cpp
    src/DialogueMarkup.cpp
    src/DialogueCompiler.cpp
    src/LiveRuntime.cpp
    src/ExternalMocap.cpp
    src/Schema.cpp
    src/Recovery.cpp
    src/EmotionEngine.cpp
    src/PerformanceFusion.cpp
    src/Calibration.cpp
    src/BehaviorEngine.cpp
    src/Personality.cpp
    src/FaceObservation.cpp
    src/CaptureQuality.cpp
    src/FramePacketCodec.cpp
    src/UdpTransport.cpp
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


add_executable(bdfr_cli tools/bdfr_cli.cpp)
target_link_libraries(bdfr_cli PRIVATE bdfr_core)

add_test(NAME bdfr_cli_text COMMAND bdfr_cli text "Hello world")

if(WIN32)
    target_link_libraries(bdfr_core PRIVATE ws2_32)
endif()
