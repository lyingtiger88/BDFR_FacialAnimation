cmake_minimum_required(VERSION 3.20)
project(BDFR_FacialAnimation VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BDFR_BUILD_STUDIO "Build the Qt desktop BDFR Studio application" OFF)

add_library(bdfr_core
    src/CurveRegistry.cpp
    src/BinaryCodec.cpp
    src/CurveMixer.cpp
    src/Timeline.cpp
    src/Session.cpp
    src/Timecode.cpp
    src/Sequence.cpp
    src/CorrectiveEngine.cpp
    src/ExpressionStack.cpp
    src/JsonCodec.cpp
    src/CurveDynamics.cpp
    src/KeyReducer.cpp
    src/Project.cpp
    src/ModelRegistry.cpp
    src/CurveCatalog.cpp
    src/CurveTools.cpp
    src/Diagnostics.cpp
    src/Persistence.cpp
    src/Retargeter.cpp
    src/AutoMapper.cpp
    src/Arkit52.cpp
    src/TextSpeech.cpp
    src/WavAudio.cpp
    src/AudioFeatures.cpp
    src/Prosody.cpp
    src/PitchTracker.cpp
    src/AudioBehavior.cpp
    src/DialogueMarkup.cpp
    src/DialogueCompiler.cpp
    src/TranscriptTiming.cpp
    src/LanguageProfile.cpp
    src/LiveRuntime.cpp
    src/ExternalMocap.cpp
    src/MocapCsv.cpp
    src/Schema.cpp
    src/Recovery.cpp
    src/EmotionEngine.cpp
    src/PerformanceFusion.cpp
    src/Calibration.cpp
    src/NeutralDrift.cpp
    src/CurveTuning.cpp
    src/BehaviorEngine.cpp
    src/Personality.cpp
    src/FaceObservation.cpp
    src/CaptureQuality.cpp
    src/OfflineVideoProcessor.cpp
    src/SequenceFilter.cpp
    src/SequenceDiagnostics.cpp
    src/SequenceCleanup.cpp
    src/SequenceCompare.cpp
    src/FramePacketCodec.cpp
    src/SessionStream.cpp
    src/UdpTransport.cpp
    src/LiveSessionReceiver.cpp
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


add_executable(bdfr_offline_pipeline_tests tests/test_offline_pipeline.cpp)
target_link_libraries(bdfr_offline_pipeline_tests PRIVATE bdfr_core)
add_test(NAME bdfr_offline_pipeline_tests COMMAND bdfr_offline_pipeline_tests)


add_executable(bdfr_quality_tests tests/test_quality.cpp)
target_link_libraries(bdfr_quality_tests PRIVATE bdfr_core)
add_test(NAME bdfr_quality_tests COMMAND bdfr_quality_tests)


add_executable(bdfr_sequence_compare_tests tests/test_sequence_compare.cpp)
target_link_libraries(bdfr_sequence_compare_tests PRIVATE bdfr_core)
add_test(NAME bdfr_sequence_compare_tests COMMAND bdfr_sequence_compare_tests)


add_executable(bdfr_initial_smoke_tests tests/test_initial_smoke.cpp)
target_link_libraries(bdfr_initial_smoke_tests PRIVATE bdfr_core)
add_test(NAME bdfr_initial_smoke_tests COMMAND bdfr_initial_smoke_tests)


add_executable(bdfr_live_receiver tools/bdfr_live_receiver.cpp)
target_link_libraries(bdfr_live_receiver PRIVATE bdfr_core)


if(BDFR_BUILD_STUDIO)
    add_subdirectory(apps/BDFRStudio)
endif()
